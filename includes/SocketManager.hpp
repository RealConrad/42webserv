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

		/**
		 * @brief Accepts a new connection (client) on the server socket (server_fd).
		 * 
		 * This function initializes a new client state and adds the client to the pollfd list.
		 * @param server_fd The server socket file descriptor to accept the connection on.
		 */
		void acceptNewConnections(int server_fd);


		/**
		 * @brief Closes the connection of the given `fd`.
		 * 
		 * This function also removes the file descriptor from the pollfd lists.
		 * @param fd The file descriptor of the connection to close.
		 */
		void closeConnection(int fd);

		/**
		 * @brief Creates a new socket and binds it to the given port.
		 * 
		 * Sets the socket to non-blocking mode and listens on the socket. 
		 * @param port The port to bind the socket to.
		 * @return The file descriptor of the created socket, or -1 if creating the socket failed.
		 */
		int createAndBindSocket(int port);

		/**
		 * @brief Checks if the given file descriptor is a server socket.
		 * 
		 * @param fd The file descriptor to check.
		 * @return True if the file descriptor is a server socket, false otherwise.
		 */
		bool isServerSocket(int fd);

		/**
		 * @brief Gets the server configuration for the given host and port.
		 *
		 * @param hostName The host name of the server. 
		 * @param port The port of the server.
		 * @return The found server configuration.
		 * @throw std::runtime_error If the server configuration could not be found.
		 */
		ServerConfig& getCurrentServer(std::string &hostName, int port);

		/**
		 * @brief Checks if the given port is already in the list of ports.
		 * @param ports Vector of ports to check.
		 * @param port The port to check for.
		 * @return True if the port is in the list, false otherwise.
		 */
		bool portExists(std::vector<int> &ports, int port);

		/**
		 * @brief Sends a response to a client.
		 *
		 * Sends a response using the client's file descriptor. Handles empty write buffers, successful writes, 
		 * keep-alive connections, no data written, and write errors.
		 *
		 * @param fd A reference to the pollfd struct for the client.
		 */
		void sendResponse(pollfd &fd);

		/**
		 * @brief Reads data from the client socket.
		 *
		 * This function reads data from the client socket associated with the given file descriptor.
		 *
		 * @param fd The file descriptor of the client socket.
		 * @return true if the data was successfully read, false otherwise.
		 */
		bool readClientData(int fd);

		/**
		 * @brief Processes a request received on the given file descriptor.
		 *
		 * This function is responsible for processing a request received on the specified file descriptor.
		 * It performs the necessary operations to handle the request depending on `method` or if its `CGI`
		 * and generates an appropriate response.
		 *
		 * @param fd The file descriptor representing the connection on which the request was received.
		 */
		void processRequest(int fd);

		/**
		 * Handles the Common Gateway Interface (CGI) request for the client.
		 *
		 * This function takes a reference to a `ClientState` object and a reference to a `std::string` object representing the full path of the requested resource.
		 * It processes the CGI request and returns a `std::string` containing the response from the CGI program.
		 *
		 * @param client The `ClientState` object representing the client making the request.
		 * @param fullPath The full path of the requested resource.
		 * @return A `std::string` containing the response from the CGI program.
		 */
		std::string handleCGI(ClientState& client, std::string& fullPath);

		/**
		 * Checks and handles the child process for the given client.
		 *
		 * This function checks if there is a child process associated with the client and handles it accordingly.
		 * It returns a string indicating the result of the handling process.
		 *
		 * @param client The client state object for which to check and handle the child process.
		 * @return A string indicating the result of the handling process.
		 */
		std::string checkAndHandleChildProcess(ClientState& client);

		/**
		 * @brief Executes a child process for the given client and full path.
		 *
		 * This function is responsible for executing a child process for the given client and full path.
		 * It takes a reference to a ClientState object and a reference to a string representing the full path.
		 *
		 * @param client The client state object.
		 * @param fullPath The full path of the file to be executed.
		 */
		void executeChild(ClientState& client, std::string& fullPath);

		/**
		 * @brief Processes the CGI script and sends the output to the given file descriptor.
		 *
		 * This function takes a string containing the CGI script code and an integer file descriptor as parameters.
		 * It processes the CGI script and sends the output to the specified file descriptor.
		 *
		 * @param stringCode The CGI script code to be processed.
		 * @param fd The file descriptor to which the output will be sent.
		 */
		void processCGI(std::string stringCode, int fd);

	public:
		SocketManager(const HTTPConfig& config);
		~SocketManager();

		/**
		 * @brief Sets up the server sockets.
		 * 
		 * This function is responsible for setting up the server sockets for communication.
		 * It performs the necessary initialization and configuration to establish the server sockets.
		 */
		void setupServerSockets();

		/**
		 * @brief Handles the POLLIN event for a given file descriptor.
		 *
		 * This function is responsible for handling the POLLIN event for a specific file descriptor.
		 * It performs the necessary operations to process incoming data from the file descriptor.
		 *
		 * @param fd The file descriptor to handle the POLLIN event for.
		 */
		void pollin(pollfd &fd);

		/**
		 * @brief Handles the POLLOUT event for a given file descriptor.
		 *
		 * This function is responsible for handling the POLLOUT event for a specific file descriptor.
		 * It is called when the file descriptor is ready for writing.
		 *
		 * @param fd The pollfd structure representing the file descriptor.
		 */
		void pollout(pollfd &fd);

		/**
		 * @brief Handles the error event for a given file descriptor.
		 *
		 * This function is called when an error event is detected for a file descriptor
		 * during the polling process. It takes a reference to a `pollfd` structure
		 * representing the file descriptor that encountered the error.
		 *
		 * @param fd The `pollfd` structure representing the file descriptor.
		 */
		void pollerr(pollfd &fd);

		/**
		 * @brief Runs the socket manager. This is the main loop.
		 * 
		 * This function is responsible for running the socket manager and handling incoming connections.
		 */
		void run();
};

#endif