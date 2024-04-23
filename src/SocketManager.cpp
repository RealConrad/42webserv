#include "SocketManager.hpp"

bool g_run; 

SocketManager::SocketManager(const HTTPConfig& config): config(config) {}

SocketManager::~SocketManager() {
	INFO("Closing all sockets");
	for (size_t i = 0; i < this->fds.size(); i++) {
		closeConnection(this->fds[i].fd);
	}
}

/* -------------------------------------------------------------------------- */
/*                                 Main Server                                */
/* -------------------------------------------------------------------------- */

void stopServer(int) {
	g_run = false;
}

void errnoPoll(){
	switch (errno) {
		case EBADF:
			ERROR("poll() failed: EBADF - One or more of the file descriptors was not valid.");
			break;
		case EINTR:
			ERROR("poll() failed: EINTR - Interrupted by a signal handler before any of the file descriptors became ready.");
			break;
		case EINVAL:
			ERROR("poll() failed: EINVAL - The nfds value exceeds the RLIMIT_NOFILE value, or the timeout value was invalid.");
			break;
		case ENOMEM:
			ERROR("poll() failed: ENOMEM - Not enough space to allocate file descriptor tables.");
			break;
		default:
			ERROR("poll() failed: Unknown error.");
			break;
	}
}

void SocketManager::pollin(pollfd &fd) {
	INFO("Recived a request on a socket *" << fd.fd << "*");
	if (isServerSocket(fd.fd)) {
		acceptNewConnections(fd.fd);
	} else {
		time(&clientStates[fd.fd].lastActivity);
		clientStates[fd.fd].responding = true;
		if (readClientData(fd.fd)) {
			fd.events |= POLLOUT;
			processRequest(fd.fd);
		}
	}
}
void SocketManager::pollout(pollfd &fd) {
	if (clientStates[fd.fd].hasForked){
		// processRequest(fd.fd);
		// DEBUG("Checking for child finish...")
		processCGI(checkAndHandleChildProcess(clientStates[fd.fd]), fd.fd);
	} else {
		INFO("Sending response back to client from socket *" << fd.fd << "*");
		time(&clientStates[fd.fd].lastActivity);
		sendResponse(fd);
	}
}

void SocketManager::pollerr(pollfd &fd) {
	if (fd.revents & (POLLERR))
		WARNING("POLLERR : operation on the file descriptor failed unexpectedly on socket *" << fd.fd << "*");
	if (fd.revents & (POLLHUP))
		WARNING("POLLHUP : client closed the connection on socket *" << fd.fd << "*");
	if (fd.revents & (POLLNVAL))
		WARNING("POLLNVAL : file descriptor is not open or invalid on socket *" << fd.fd << "*");
	clientStates[fd.fd].closeConnection = true;
}


void SocketManager::run() {
	if (this->fds.size() == 0) {
		ERROR("No servers configured, cannot run poll()!");
		return;
	}
	g_run = true;
	signal(SIGINT, stopServer);
	INFO("Running poll()");
	while (g_run) {
		if (poll(&this->fds[0], this->fds.size(), this->config.server_timeout_time) < 0) {
			errnoPoll();
			continue;
		}
		for (size_t i = 0; i < this->fds.size(); i++) {
			if (this->fds[i].revents & POLLIN)
				pollin(fds[i]);
			if (this->fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				pollerr(fds[i]);
			if (!isServerSocket(this->fds[i].fd)) {
				if (this->fds[i].revents & POLLOUT)
					pollout(fds[i]);
				time_t now;
				time(&now);
				if (clientStates[fds[i].fd].assignedConfig && clientStates[fds[i].fd].responding && difftime(now, clientStates[fds[i].fd].lastActivity) > clientStates[fds[i].fd].serverConfig.sendTimeout){
					WARNING("Send timeout on socket *" << fds[i].fd << "*");
					clientStates[fds[i].fd].killTheChild = true;
				}
				if (clientStates[fds[i].fd].assignedConfig && difftime(now, clientStates[fds[i].fd].lastActivity) > clientStates[fds[i].fd].serverConfig.keepAliveTimeout){
					WARNING("Keep-alive timeout on socket *" << fds[i].fd << "*");
					clientStates[fds[i].fd].closeConnection = true;
				}
				if (clientStates[fds[i].fd].closeConnection == true){
					closeConnection(this->fds[i].fd);
					i--;
				}
			}
		}
	}
}

/* -------------------------------------------------------------------------- */
/*                               Set Up Sockets                               */
/* -------------------------------------------------------------------------- */

bool SocketManager::portExists(std::vector<int> &ports, int port){
	for (size_t i = 0; i < ports.size(); i++) {
		if (ports[i] == port)
			return true;
	}
	return false;
}

void SocketManager::setupServerSockets() {
	INFO("Setting up server sockets");
	std::vector<int> ports;
	for (size_t i = 0; i < this->config.serverConfigs.size(); i++) {
		if (portExists(ports, this->config.serverConfigs[i].listenPort))
			continue;
		int sockfd = createAndBindSocket(this->config.serverConfigs[i].listenPort);
		if (sockfd >= 0) {
			ports.push_back(this->config.serverConfigs[i].listenPort);
			struct pollfd pfd = {sockfd, POLLIN, 0};
			this->fds.push_back(pfd);
			this->server_fds.push_back(sockfd);
			this->serverConfigs[sockfd] = this->config.serverConfigs[i];

		}
	}
}

int SocketManager::createAndBindSocket(int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		ERROR("Failed to create socket for port: " << port);
		return -1;
	}
	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK | FD_CLOEXEC) < 0) {
		closeConnection(sockfd);
		ERROR("Failed to set to non blocking mode for socket: *" << sockfd << "*");
		return -1;
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		closeConnection(sockfd);
		ERROR("Failed to bind socket to port: " << port);
		return -1;
	}

	if (listen(sockfd, SOMAXCONN) < 0) {
		closeConnection(sockfd);
		ERROR("Failed to initialize listen for socket: *" << sockfd << "*");
		return -1;
	}
	SUCCESS("Socket *"<< sockfd << "* is listening on port " << port);
	return sockfd;
}

/* -------------------------------------------------------------------------- */
/*                                Handle Client                               */
/* -------------------------------------------------------------------------- */

void SocketManager::acceptNewConnections(int server_fd) {
	sockaddr_in client_addr;
	socklen_t clilen = sizeof(client_addr);
	int newsockfd = accept(server_fd, (struct sockaddr*)&client_addr, &clilen);
	if (newsockfd < 0) {
		ERROR("Error accepting connection");
		return;
	}
	this->clientStates[newsockfd] = ClientState();
	fcntl(newsockfd, F_SETFL, O_NONBLOCK | FD_CLOEXEC);
	struct pollfd new_pfd = {newsockfd, POLLIN, 0};
	this->fds.push_back(new_pfd);
	time(&this->clientStates[newsockfd].lastActivity);
	this->clientStates[newsockfd].serverPort = this->serverConfigs[server_fd].listenPort;
	SUCCESS("Server socket *" << server_fd << "* Accepted new connection on socket *" << newsockfd << "*");
}

/* ----------------------------- Handle Requests ---------------------------- */

bool SocketManager::readClientData(int fd) {
	size_t size = 4096 * 4;
	char buffer[size];
	memset(buffer, 0, sizeof(buffer));
	ssize_t bytesRead = recv(fd, buffer, size, 0);
	if (bytesRead > 0) {
		this->clientStates[fd].readBuffer.append(buffer, bytesRead);
		if (!this->clientStates[fd].headersComplete) {
			size_t headerEndPos = this->clientStates[fd].readBuffer.find("\r\n\r\n");
			if (headerEndPos != std::string::npos) {
				this->clientStates[fd].headersComplete = true;
				this->clientStates[fd].headerEndIndex = headerEndPos + 4;
				size_t startPos = this->clientStates[fd].readBuffer.find("Content-Length: ");
				if (startPos != std::string::npos) {
					startPos += 16;
					size_t endPos = this->clientStates[fd].readBuffer.find("\r\n", startPos);
					std::istringstream iss(this->clientStates[fd].readBuffer.substr(startPos, endPos - startPos));
					iss >> this->clientStates[fd].contentLength;
					this->clientStates[fd].totalRead = this->clientStates[fd].readBuffer.length() - this->clientStates[fd].headerEndIndex;
				} else {
					this->clientStates[fd].contentLength = 0;
				}
				startPos = this->clientStates[fd].readBuffer.find("Host: ");
				std::string hostName;
				if (startPos != std::string::npos) {
					startPos += 6;
					size_t endPos = this->clientStates[fd].readBuffer.find("\r\n", startPos);
					std::istringstream iss(this->clientStates[fd].readBuffer.substr(startPos, endPos - startPos));
					iss >> hostName;
				}
				clientStates[fd].serverConfig = getCurrentServer(hostName, clientStates[fd].serverPort);
				clientStates[fd].assignedConfig = true;
			}
		} else {
			this->clientStates[fd].totalRead += bytesRead;
		}
		if (this->clientStates[fd].headersComplete && this->clientStates[fd].totalRead == this->clientStates[fd].contentLength) {
			this->clientStates[fd].totalRead = 0;
			this->clientStates[fd].headersComplete = false;
			return true;
		}
	} else if (bytesRead == 0) {
		this->clientStates[fd].closeConnection = true;
	} else if (bytesRead < 0) {
		ERROR("Failed to read from recv()");
		this->clientStates[fd].closeConnection = true;
	}
	return false;
}

/* Handle CGI */


std::string SocketManager::handleCGI(ClientState& client, std::string& fullPath) {
	std::string output;
    if (!client.hasForked) {
        INFO("Starting CGI - Forking process");
        if (pipe(client.childFd) == -1) {
            ERROR("Failed to create pipe");
            client.closeConnection = true;
            return ("Internal server error");
        }
        client.childPid = fork();
        if (client.childPid == -1) {
            ERROR("Failed to fork");
            close(client.childFd[0]);
			close(client.childFd[1]);
            client.closeConnection = true;
            return ("Internal server error");
        } else if (client.childPid == 0) {
            executeChild(client, fullPath);
            return ("Internal server error");
        } else {
            close(client.childFd[1]);  // Close the write end of the pipe in the parent
            client.hasForked = true;
        	return checkAndHandleChildProcess(client);
        }
    } else {
		return checkAndHandleChildProcess(client);
    }
}

void SocketManager::processCGI(std::string stringCode, int fd) {
	if (!stringCode.empty()){
		try {
			HTTPResponse response;
			if (stringCode == "CGI timeout" || stringCode == "CGI script error" || stringCode == "Internal server error"){
				response.assignGenericResponse(500, stringCode);
			} else {
				response.assignResponse(200, stringCode, "text/html");
			}
			this->clientStates[fd].writeBuffer = response.convertToString();
			this->clientStates[fd].readBuffer.clear();
			this->clientStates[fd].hasForked = false;
		} catch (const std::runtime_error& e) {
			ERROR(e.what());
		}
	}
}

std::string SocketManager::checkAndHandleChildProcess(ClientState& client) {
    int status;
    std::string output;
    char buffer[1024];
    int bytesRead;
    time_t now;
	time(&now);
	// DEBUG("checking the child..."<< client.childPid << " at " << now);
	if (difftime(now, client.lastActivity) > client.serverConfig.sendTimeout) {
		WARNING("CGI process timed out");
		kill(client.childPid, SIGKILL); // Force kill the child process
		waitpid(client.childPid, &status, 0); // Clean up the zombie process
		// client.hasForked = false;
		close(client.childFd[0]);
		output = "CGI timeout";
		return(output);
	}
	// Non-blocking check if child process has exited
	pid_t result = waitpid(client.childPid, &status, WNOHANG);
	if (result == 0) {
		return output; // Child still running, check again
	} if (result == client.childPid) {
		if (WIFEXITED(status)) {
			// Read remaining data from pipe
			while ((bytesRead = read(client.childFd[0], buffer, sizeof(buffer))) > 0) {
				output += buffer;
			}
			// client.hasForked = false;
			// close(client.childFd[0]);
			return(output);
		} else {
			ERROR("CGI script exited with error");
			// client.hasForked = false; 
			close(client.childFd[0]);
			output = "CGI script error";
			return(output);
		}
	} else {
		ERROR("waitpid returned unexpected result");
		// client.hasForked = false; 
		close(client.childFd[0]);
		output = "Internal server error";
		return(output);
	}
}

void SocketManager::executeChild(ClientState& client, std::string& fullPath) {
    char fullPathWritable[1024]; // Ensure the fullPath is within a reasonable limit
    std::strcpy(fullPathWritable, fullPath.c_str());
	// we need to divide the full path into runPath and PATH_INFO

    close(client.childFd[0]);
    dup2(client.childFd[1], STDOUT_FILENO);
    close(client.childFd[1]);

    const char* argv[] = {"/usr/bin/python3", fullPathWritable, NULL};

	if (client.method == "GET"){
    	const char* envp[] = {NULL}; // PATH_INFO
	} else {
    	const char* envp[] = {NULL}; // client.bodyCgi 
	}

    execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(envp));

    ERROR("execve failed: " << fullPath);
    exit(1); // If execve returns, it's an error
}



/* ---------------------------- Handle Responses ---------------------------- */



void SocketManager::processRequest(int fd) {
	HTTPRequest request(this->clientStates[fd].readBuffer);
	BLOCK(this->clientStates[fd].readBuffer);
	std::string stringCode = "go";
	std::string keepAlive = request.getHeader("Connection");
	if (keepAlive == "keep-alive")
		this->clientStates[fd].keepAlive = true;
	else
		this->clientStates[fd].keepAlive = false;
	if (clientStates[fd].contentLength > clientStates[fd].serverConfig.clientMaxBodySize){
		stringCode = "413";
	} else if (endsWith(request.getURI(), ".py")) { /// Modify to find .py or .py?whatever 
		std::string fullPath = clientStates[fd].serverConfig.rootDirectory + request.getURI();
		clientStates[fd].method = request.getMethod();
		if (clientStates[fd].method == "POST"){
			clientStates[fd].bodyCgi = request.getBody();
		}
		if (clientStates[fd].method == "GET" || clientStates[fd].method == "POST")
			stringCode = handleCGI(clientStates[fd], fullPath);
		else
			stringCode = "403";
	}
	if (stringCode.empty())
		return;

	try {
		HTTPResponse response;
		if (stringCode == "413"){
			WARNING("Body to big! serving 413!");
			std::string payload = "Request has a body size of " + ::toString(clientStates[fd].contentLength) 
				+ " bytes which exceeds the server body limit of " + ::toString(clientStates[fd].serverConfig.clientMaxBodySize) + " bytes!"; 
			response.assignGenericResponse(413, payload);
		} else if (stringCode == "403"){
			response.assignGenericResponse(403);
		} else if (stringCode == "CGI timeout" || stringCode == "CGI script error" || stringCode == "Internal server error"){
			response.assignGenericResponse(500, stringCode);
		} else if (stringCode == "go"){
			response.prepareResponse(request, this->clientStates[fd]);
		} else {
			response.assignResponse(200, stringCode, "text/html");
		}
		this->clientStates[fd].writeBuffer = response.convertToString();
		this->clientStates[fd].readBuffer.clear();
		this->clientStates[fd].hasForked = false;
	} catch (const std::runtime_error& e) {
		ERROR(e.what());
	}
}

void SocketManager::sendResponse(pollfd &fd) {
	if (this->clientStates[fd.fd].writeBuffer.empty()) {
		WARNING("Nothing to send on socket *" << fd.fd << "*");
		fd.events = POLLIN;
		clientStates[fd.fd].assignedConfig = false;
		return;
	}
	ssize_t bytesWritten = send(fd.fd, this->clientStates[fd.fd].writeBuffer.c_str(), this->clientStates[fd.fd].writeBuffer.size(), 0);
	if (bytesWritten > 0) {
		this->clientStates[fd.fd].writeBuffer.erase(0, bytesWritten);
		if (this->clientStates[fd.fd].writeBuffer.empty()) {
			clientStates[fd.fd].responding = false;
			fd.events = POLLIN;
			SUCCESS("Response sent successfully on socket *" << fd.fd << "*");
			if (this->clientStates[fd.fd].keepAlive == false) {
				WARNING("Non-keep-alive connection termination on socket *" << fd.fd << "*");
				this->clientStates[fd.fd].closeConnection = true;
			}
		clientStates[fd.fd].assignedConfig = false;
		}
	} else if (bytesWritten == 0) {
		WARNING("No data was sent for socket *" << fd.fd << "*");
	} else {
		ERROR("Failed to send response for socket *" << fd.fd << "*");
		this->clientStates[fd.fd].closeConnection = true;
	}
}

/* -------------------------------------------------------------------------- */
/*                              Helper Functions                              */
/* -------------------------------------------------------------------------- */

// void SocketManager::checkAndHandleChildProcess(ClientState& client) {
//     int status;
//     std::string output;
//     char buffer[1024];
//     int bytesRead;
//     // time_t startTime = time(NULL);
//     time_t now;
// 	DEBUG("checking the child..." << client.childPid);
//     // while (true) {
//         time(&now);
//         if (difftime(now, client.lastActivity) > client.serverConfig.sendTimeout) {
// 			HTTPResponse response;
//             WARNING("CGI process timed out");
//             kill(client.childPid, SIGKILL); // Force kill the child process
//             waitpid(client.childPid, &status, 0); // Clean up the zombie process
//             response.assignGenericResponse(500, "CGI timeout");
//             client.hasForked = false;
//             close(this->fd[0]);
//             break;
//         }

//         // Non-blocking check if child process has exited
//         pid_t result = waitpid(this->pid, &status, WNOHANG);
//         if (result == 0) {
//             continue; // Child still running, check again
//         } if (result == this->pid) {
//             if (WIFEXITED(status)) {
//                 // Read remaining data from pipe
//                 while ((bytesRead = read(this->fd[0], buffer, sizeof(buffer) - 1)) > 0) {
//                     buffer[bytesRead] = '\0';
//                     output += buffer;
//                 }
//                 assignResponse(200, output, "text/html");
// 				break;
//                 // client.hasForked = false;
//             } else {
//                 ERROR("CGI script exited with error");
//                 assignGenericResponse(500, "CGI script error");
// 				break;
//                 // client.hasForked = false;
//             }
//             close(this->fd[0]);
//         } else {
//             ERROR("waitpid returned unexpected result");
//             assignGenericResponse(500, "Internal server error");
//             close(this->fd[0]);
// 			break;
//             // client.hasForked = false;
//         }
// 	// }
// }


ServerConfig& SocketManager::getCurrentServer(std::string &hostName, int port) {
	size_t colonPos = hostName.find(":");
	if (colonPos != std::string::npos) {
		hostName = hostName.substr(0, colonPos);
	}
	bool defaultConfig = false;
	std::vector<ServerConfig>::iterator ogConfig;
	for (std::vector<ServerConfig>::iterator iter = this->config.serverConfigs.begin(); iter != this->config.serverConfigs.end(); iter++) {
		if (!defaultConfig && iter->listenPort == port){
			defaultConfig = true;
			ogConfig = iter;
		}
		if (iter->listenPort == port && iter->serverName == hostName) {
			return *iter;
		}
	}
	if (defaultConfig)
		return *ogConfig;
	throw std::runtime_error("Server config not found for host: " + hostName);
}

void SocketManager::closeConnection(int fd) {
	INFO("Closing socket: " << fd);
	close(fd);

	for (std::vector<struct pollfd>::iterator it = this->fds.begin(); it != this->fds.end();) {
		if (it->fd == fd) {
			it = this->fds.erase(it);
		} else {
			++it;
		}
	}

	for (std::vector<int>::iterator it = this->server_fds.begin(); it != this->server_fds.end();) {
		if (*it == fd) {
			it = this->server_fds.erase(it);
		} else {
			++it;
		}
	}

	std::map<int, ClientState>::iterator it = this->clientStates.find(fd);
	if (it != this->clientStates.end()) {
		this->clientStates.erase(it);
	}
}

bool SocketManager::isServerSocket(int fd) {
	return std::find(this->server_fds.begin(), this->server_fds.end(), fd) != this->server_fds.end();
}
