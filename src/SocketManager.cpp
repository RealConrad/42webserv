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

void stopServer(int){
	g_run = false;
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
		int pollRevents = poll(&this->fds[0], this->fds.size(), this->config.server_timeout_time);
		if (pollRevents < 0) {
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
			continue;
		} else if (pollRevents == 0) {
			continue;
		}
		for (size_t i = 0; i < this->fds.size(); i++) {
			if (this->fds[i].revents & POLLIN) {
				INFO("Recived a request on a socket *" << this->fds[i].fd << "*");
				if (isServerSocket(this->fds[i].fd)) {
					acceptNewConnections(this->fds[i].fd);
				} else {
					handleClientRequest(fds[i]);
				}
			}
			if (this->fds[i].revents & POLLOUT && !isServerSocket(this->fds[i].fd)) {
				INFO("Sending response back to client from socket *" << this->fds[i].fd << "*");
				sendResponse(fds[i]);
			}
			if (this->fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				if (this->fds[i].revents & (POLLERR))
					ERROR("POLLERR : operation on the file descriptor failed unexpectedly on socket *" << fds[i].fd << "*");
				if (this->fds[i].revents & (POLLHUP))
					ERROR("POLLHUP : client closed the connection on socket *" << fds[i].fd << "*");
				if (this->fds[i].revents & (POLLNVAL))
					ERROR("POLLNVAL : file descriptor is not open or invalid on socket *" << fds[i].fd << "*");
				closeConnection(this->fds[i].fd);
				i--;
			}
		}
	}
}

/* -------------------------------------------------------------------------- */
/*                               Set Up Sockets                               */
/* -------------------------------------------------------------------------- */

void SocketManager::setupServerSockets() {
	INFO("Setting up server sockets");
	for (size_t i = 0; i < this->config.serverConfigs.size(); i++) {
		int sockfd = createAndBindSocket(this->config.serverConfigs[i].listenPort);
		if (sockfd >= 0) {
			struct pollfd pfd = {sockfd, POLLIN, 0};
			this->fds.push_back(pfd);
			this->server_fds.push_back(sockfd);
		}
	}
}

int SocketManager::createAndBindSocket(int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		ERROR("Failed to create socket for port: " << port);
		return -1;
	}

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
	SUCCESS("Server socket *" << server_fd << "* Accepted new connection on socket *" << newsockfd << "*");
}

void SocketManager::handleClientRequest(pollfd &fd) {
	fd.events |= POLLOUT;
	if (!clientStates[fd.fd].requestComplete) {
		if (readClientData(fd.fd) && clientStates[fd.fd].requestComplete) {
			processRequest(fd.fd);
		}
	}
}

/* ----------------------------- Handle Requests ---------------------------- */

bool SocketManager::readClientData(int fd) {
	INFO("Reading client request:");
	char buffer[4096 * 10];
	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer), 0);
	if (bytesRead > 0) {
		this->clientStates[fd].readBuffer.append(buffer, bytesRead);
		if (this->clientStates[fd].readBuffer.find("\r\n\r\n") != std::string::npos) {
			this->clientStates[fd].requestComplete = true;
			return true;
		}
	} else if (bytesRead == 0) {
		WARNING("Client connection is closed");
		closeConnection(fd);
	} else {
		ERROR("Failed to read from recv()");
		closeConnection(fd);
	}
	return false;
}

/* ---------------------------- Handle Responses ---------------------------- */

void SocketManager::processRequest(int fd) {
	HTTPRequest request(this->clientStates[fd].readBuffer);
	const ServerConfig& serverConfig = getCurrentServer(request);

	try {
		HTTPResponse response;
		response.prepareResponse(request, serverConfig);
		clientStates[fd].responseComplete = false;
		clientStates[fd].writeBuffer = response.convertToString();
	} catch (const std::runtime_error& e) {
		ERROR(e.what());
	}
}

void SocketManager::sendResponse(pollfd &fd) {
	if (clientStates[fd.fd].writeBuffer.empty()) {
		WARNING("Nothing to send on socket *" << fd.fd << "*");
		fd.events = POLLIN;
		return;
	}
	ssize_t bytesWritten = send(fd.fd, clientStates[fd.fd].writeBuffer.c_str(), clientStates[fd.fd].writeBuffer.size(), 0);
	if (bytesWritten > 0) {
		clientStates[fd.fd].writeBuffer.erase(0, bytesWritten);
		if (clientStates[fd.fd].writeBuffer.empty()) {
			fd.events = POLLIN;
			SUCCESS("Response sent successfully on socket *" << fd.fd << "*");
		}
	} else if (bytesWritten == 0) {
		WARNING("No data was sent for socket *" << fd.fd << "*");
	} else {
		ERROR("Failed to send response for socket *" << fd.fd << "*");
		closeConnection(fd.fd);
	}
}
/* -------------------------------------------------------------------------- */
/*                              Helper Functions                              */
/* -------------------------------------------------------------------------- */

ServerConfig& SocketManager::getCurrentServer(const HTTPRequest& request) {
	std::string hostName = request.getHeader("Host");

	size_t colonPos = hostName.find(":");
	if (colonPos != std::string::npos) {
		hostName = hostName.substr(0, colonPos);
	}
	for (std::vector<ServerConfig>::iterator iter = this->config.serverConfigs.begin(); iter != this->config.serverConfigs.end(); iter++) {
		if (iter->serverName == hostName) {
			return *iter;
		}
	}
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

void SocketManager::addServerFd(int fd) {
	server_fds.push_back(fd);
}

bool SocketManager::isServerSocket(int fd) {
	return std::find(server_fds.begin(), server_fds.end(), fd) != server_fds.end();
}
