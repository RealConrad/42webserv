#ifndef SOCKET_MANAGER_HPP
# define SOCKET_MANAGER_HPP

#include <vector>
#include <map>
#include <unistd.h> 
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <netinet/in.h>
#include <algorithm>
#include <fstream>
#include <streambuf>
#include <ctime>
#include <cstring>
#include <signal.h>
#include <sys/wait.h>

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
		std::map<int, ServerConfig> serverConfigs;

		void acceptNewConnections(int server_fd);
		void closeConnection(int fd);
		int createAndBindSocket(int port);
		bool isServerSocket(int fd);
		ServerConfig& getCurrentServer(std::string &hostName, int port);

		bool portExists(std::vector<int> &ports, int port);
		void sendResponse(pollfd &fd);
		bool readClientData(int fd);
		void processRequest(int fd);

		std::string handleCGI(ClientState& client, std::string& fullPath);
		std::string checkAndHandleChildProcess(ClientState& client);
		void executeChild(ClientState& client, std::string& fullPath);
		void processCGI(std::string stringCode, int fd);

	public:
		SocketManager(const HTTPConfig& config);
		~SocketManager();

		void setupServerSockets();

		void pollin(pollfd &fd);
		void pollout(pollfd &fd);
		void pollerr(pollfd &fd);
		void run();
};

#endif