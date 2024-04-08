#ifndef SOCKET_MANAGER_HPP
# define SOCKET_MANAGER_HPP

#include <vector>
#include <map>
#include <unistd.h> 
#include <fcntl.h>
#include <poll.h> // poll
#include <netinet/in.h> // sockaddr_in
#include "Structs.hpp"
#include "Logger.hpp"

class SocketManager {
	private:
		HTTPConfig config;
		std::vector<struct pollfd> fds;
		
		void acceptNewConnections(int server_fd);
		void closeConnection(int fd);
		int createAndBindSocket(int port);
	public:
		SocketManager(const HTTPConfig& config);
		~SocketManager();

		void setupServerSockets();
		void run();
};

#endif