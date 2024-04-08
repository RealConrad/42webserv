#include "SocketManager.hpp"

SocketManager::SocketManager(const HTTPConfig& config): config(config) {}

SocketManager::~SocketManager() {
	INFO("Closing all sockets");
	for (size_t i = 0; i < this->fds.size(); i++) {
		close(this->fds[i].fd);
	}
}

void SocketManager::setupServerSockets() {
	INFO("Setting up server sockets");
	for (size_t i = 0; i < this->config.serverConfigs.size(); i++) {
		int sockfd = createAndBindSocket(this->config.serverConfigs[i].listenPort);
		if (sockfd >= 0) {
			struct pollfd pfd = {sockfd, POLLIN, 0};
			this->fds.push_back(pfd);
		}
	}
}

int SocketManager::createAndBindSocket(int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		ERROR("Failed to create socket for port: " << port);
		return -1;
	}

	// Make socket non blocking
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		close(sockfd);
		ERROR("Failed to set to non blocking mode for socket: " << sockfd);
		return -1;
	}

	// bind
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		ERROR("Failed to bind socket to port: " << port);
		return -1;
	}

	// listen
	if (listen(sockfd, SOMAXCONN) < 0) {
		close(sockfd);
		ERROR("Failed to initialize listen for socket: " << sockfd);
		return -1;
	}
	SUCCESS("Socket "<< sockfd << " listens on port " << port << "!");
	return sockfd;
}

void SocketManager::acceptNewConnections(int server_fd) {
	(void)server_fd;
}

void SocketManager::closeConnection(int fd) {
	INFO("Closing socket: " << fd);
	close(fd);
}

void SocketManager::run() {
	if (this->fds.size() == 0) {
		ERROR("No servers configured");
		return;
	}

	while (true) {
		// Poll the sockets for events
		int ret = poll(&this->fds[0], this->fds.size(), -1); // -1 means no timeout

		if (ret < 0) {
			ERROR("poll() error");
			break;
		}

		// Iterate over fds to check which ones are ready
		for (size_t i = 0; i < this->fds.size(); i++) {
			if (this->fds[i].revents & POLLIN) { // Check if ready for reading
				if (isServerSocket(this->fds[i].fd)) {
					// Accept a new connection
					sockaddr_in client_addr;
					socklen_t clilen = sizeof(client_addr);
					int newsockfd = accept(this->fds[i].fd, (struct sockaddr*)&client_addr, &clilen);
					if (newsockfd < 0) {
						ERROR("Error accepting connection");
						continue;
					}

					// Make the new socket non-blocking
					int flags = fcntl(newsockfd, F_GETFL, 0);
					fcntl(newsockfd, F_SETFL, flags | O_NONBLOCK);

					// Add the new socket to the fds vector to monitor it with poll()
					struct pollfd new_pfd = {newsockfd, POLLIN, 0};
					this->fds.push_back(new_pfd);

					INFO("Accepted new connection");
				} else {
					// Here, handle read/write for client sockets
				}
			}
		}
	}
}

void SocketManager::addServerFd(int fd) {
    server_fds.push_back(fd); // Add the server FD to the vector
}

bool SocketManager::isServerSocket(int fd) {
    return std::find(server_fds.begin(), server_fds.end(), fd) != server_fds.end();
}

