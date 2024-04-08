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
		std::vector<int> server_fds;

		
		void acceptNewConnections(int server_fd);
		void closeConnection(int fd);
		int createAndBindSocket(int port);
	public:
		SocketManager(const HTTPConfig& config);
		~SocketManager();

		void setupServerSockets();
		void run();
		void addServerFd(int fd); // Method to add a server FD
		bool isServerSocket(int fd); // Method to check if FD is a server socket
};

#endif