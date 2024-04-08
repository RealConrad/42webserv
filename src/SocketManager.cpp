#include "SocketManager.hpp"
#include "Logger.hpp"

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
		ERROR("Failed to set to non blocking mode for socket: " << socket);
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
		ERROR("No servers found");
		return;
	}
	// while (true) {
	// 	// Pass all fds to poll stored in `fds`
	// 	int ret = poll(this->fds.data(), this->fds.size(), this->config.server_timeout_time);

	// 	// Error checking
	// 	if (ret < 0) {
	// 		ERROR("Poll() error");
	// 		break;			
	// 	} else if (ret == 0) { // 0 indicates time out (no fds were ready), try poll again
	// 		WARNING("Socket timed out, trying again");
	// 		continue ;
	// 	}
	// 	for (size_t i = 0; i < this->fds.size(); i++) {
	// 		if (this->fds[i].revents & POLLIN) {
				
	// 		}
	// 	}
	// }
}
