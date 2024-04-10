#ifndef SOCKET_MANAGER_HPP
# define SOCKET_MANAGER_HPP

#include <vector>
#include <map>
#include <unistd.h> 
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h> // sockaddr_in
#include <algorithm>
#include <fstream>
#include <streambuf>

#include "Structs.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

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
		bool isMethodAllowed(const std::string& method, const std::string& uri, const ServerConfig& serverConfig);
		ServerConfig& getCurrentServer(const HTTPRequest& request);

		void sendResponse(int fd);
		bool readClientData(int fd);
		void processRequestAndRespond(int fd);

	public:
		SocketManager(const HTTPConfig& config);
		~SocketManager();

		void setupServerSockets();
		void run();
};

#endif