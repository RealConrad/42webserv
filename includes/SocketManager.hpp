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
		void handleClient(int fd);
		void addServerFd(int fd);
		bool isServerSocket(int fd);

		template <typename T>
		void removeElement(std::vector<T>& vec, const T& value) {
			typename std::vector<T>::iterator it = std::find(vec.begin(), vec.end(), value);
			if (it != vec.end()) {
				vec.erase(it);
			}
		}
	public:
		SocketManager(const HTTPConfig& config);
		~SocketManager();

		void setupServerSockets();
		void run();
};

#endif