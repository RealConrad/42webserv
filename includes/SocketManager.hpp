#ifndef SOCKET_MANAGER_HPP
# define SOCKET_MANAGER_HPP

#include <vector>
#include <map>
#include <unistd.h> 
#include <fcntl.h>
#include <poll.h> // poll
#include <netinet/in.h> // sockaddr_in
#include <algorithm>
#include "Structs.hpp"
#include "HTTPRequest.hpp"
#include "Logger.hpp"

class SocketManager {
	private:
		HTTPConfig config;
		std::vector<struct pollfd> fds;
		std::vector<int> server_fds;
		std::map<int, ClientState> clientStates;

		void acceptNewConnections(int server_fd);
		void closeConnection(int fd);
		int createAndBindSocket(int port);
		void handleClient(int fd);
		void addServerFd(int fd);
		bool isServerSocket(int fd);

	public:
		SocketManager(const HTTPConfig& config);
		~SocketManager();

		void setupServerSockets();
		void run();
};

#endif