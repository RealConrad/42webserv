#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>

#include "Logger.hpp"
#include "Structs.hpp"
#include "Utils.hpp"
#include "HTTPRequest.hpp"

# define CGI_TIMEOUT 5

class HTTPResponse {
	private:
		int statusCode;
		std::map<std::string, std::string> headers;
		static const std::map<int, std::string> statusCodes;
		std::string body;

		static std::map<int, std::string> initializeStatusCodes();
	public:
		HTTPResponse();
		~HTTPResponse();
		
		void prepareResponse(HTTPRequest& request, ClientState& client);
		void handleRequestGET(const HTTPRequest& request, ClientState& client);
		void handleRequestPOST(const HTTPRequest& request, const ServerConfig& serverConfig);
		void handleRequestDELETE(const HTTPRequest& request, const ServerConfig& serverConfig);

		void assignResponse(int statusCode, const std::string& body, std::string contentType);
		void assignGenericResponse(int statusCode, const std::string& message = "");

		void serveFile(ClientState& client, const std::string& uri);
		bool serveIndex(const ServerConfig& serverConfig);
		bool serveDefaultFile(const std::string& uri, const std::string& fullPath);
		void serveDirectoryListing(const std::string& uri, const std::string& fullPath);
		void serveDeletePage(const std::string& uri, const std::string& fullPath);
		void serveCGI(ClientState& client, std::string& fullPath);
		void executeChild(const std::string& fullPath);
		void checkAndHandleChildProcess(ClientState& client);
		void closePipes();

		/* -------------------------------------------------------------------------- */
		/*                              Helper Functions                              */
		/* -------------------------------------------------------------------------- */
		std::string convertToString() const;
		std::string determineContentType(std::string requestURI);
		void serveRegularFile(const std::string& uri, const std::string& fullPath);
		static bool isMethodAllowed(const std::string& method, const std::string& uri, const ServerConfig& serverConfig);
		std::string isRedirection(const std::string& uri, const ServerConfig& serverConfig);
		std::string extractFolderName(const std::string& uri);
		bool cheekySlashes(const std::string& uri);

		/* -------------------------------------------------------------------------- */
		/*                                   Setters                                  */
		/* -------------------------------------------------------------------------- */
		void setStatusCode(int code);
		void setHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& body);

};

#endif