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
		
		/**
		 * @brief Prepares the HTTP response based on the given HTTP request and client state.
		 *
		 * This function takes an HTTP request and client state as input and prepares the corresponding HTTP response.
		 * It performs the necessary operations to generate the response, such as parsing the request, validating the request,
		 * and constructing the response headers and body.
		 *
		 * @param request The HTTP request object.
		 * @param client The client state object.
		 */
		void prepareResponse(HTTPRequest& request, ClientState& client);

		/**
		 * Handles a GET request.
		 *
		 * This function is responsible for processing a GET request and generating an appropriate response.
		 *
		 * @param request The HTTPRequest object representing the incoming request.
		 * @param client The ClientState object representing the client connection.
		 */
		void handleRequestGET(const HTTPRequest& request, ClientState& client);

		/**
		 * Handles a POST request.
		 *
		 * This function is responsible for processing a POST request and generating an appropriate response.
		 *
		 * @param request The HTTPRequest object representing the incoming request.
		 * @param serverConfig The ServerConfig object containing the server configuration settings.
		 */
		void handleRequestPOST(const HTTPRequest& request, const ServerConfig& serverConfig);

		/**
		 * Handles a DELETE request.
		 *
		 * This function is responsible for processing a DELETE request and generating an appropriate response.
		 *
		 * @param request The HTTPRequest object representing the incoming request.
		 * @param serverConfig The ServerConfig object containing the server configuration settings.
		 */
		void handleRequestDELETE(const HTTPRequest& request, const ServerConfig& serverConfig);

		/**
		 * @brief Assigns the HTTP response with the given status code, body, and content type.
		 *
		 * This function is responsible for assigning the HTTP response with the provided status code,
		 * body, and content type. The status code represents the HTTP status code to be set in the response.
		 * The body parameter contains the response body, which is the content to be sent back to the client.
		 * The contentType parameter specifies the type of content in the response body.
		 *
		 * @param statusCode The HTTP status code to be set in the response.
		 * @param body The response body, which is the content to be sent back to the client.
		 * @param contentType The type of content in the response body.
		 */
		void assignResponse(int statusCode, const std::string& body, std::string contentType);

		/**
		 * @brief Assigns a generic HTTP response.
		 * 
		 * This function is used to assign a generic HTTP response to the HTTPResponse object.
		 * It takes an HTTP status code and an optional message as parameters.
		 * 
		 * @param statusCode The HTTP status code of the response.
		 * @param message The optional message to be included in the response.
		 */
		void assignGenericResponse(int statusCode, const std::string& message = "");

		/**
		 * @brief Serves a file to the client.
		 * 
		 * This function is responsible for serving a file to the client.
		 * It takes a reference to a ClientState object and the URI of the file to be served.
		 * 
		 * @param client The client state object representing the client connection.
		 * @param uri The URI of the file to be served.
		 */
		void serveFile(ClientState& client, const std::string& uri);

		/**
		 * @brief Serves the index page for the given server configuration.
		 *
		 * This function serves the index page for the specified server configuration.
		 * It returns a boolean value indicating whether the index page was successfully served or not.
		 *
		 * @param serverConfig The server configuration to use for serving the index page.
		 * @return True if the index page was successfully served, false otherwise.
		 */
		bool serveIndex(const ServerConfig& serverConfig);

		/**
		 * @brief Serves the default file for the given URI and full path.
		 *
		 * This function serves the default file for the specified URI and full path.
		 * It returns a boolean value indicating whether the default file was successfully served or not.
		 *
		 * @param uri The URI for which the default file needs to be served.
		 * @param fullPath The full path of the default file.
		 * @return True if the default file was served successfully, false otherwise.
		 */
		bool serveDefaultFile(const std::string& uri, const std::string& fullPath);

		/**
		 * @brief Serves a directory listing for the given URI and full path.
		 *
		 * This function generates a directory listing HTML page for the specified URI and full path.
		 * The generated HTML page includes links to the files and subdirectories within the directory.
		 *
		 * @param uri The URI of the directory.
		 * @param fullPath The full path of the directory on the server.
		 */
		void serveDirectoryListing(const std::string& uri, const std::string& fullPath);

		/**
		 * @brief Serves a delete page for the specified URI and full path.
		 *
		 * This function is responsible for serving a delete page for the given URI and full path.
		 * It takes the URI and full path as parameters and performs the necessary operations to serve the delete page.
		 *
		 * @param uri The URI of the delete page.
		 * @param fullPath The full path of the delete page.
		 */
		void serveDeletePage(const std::string& uri, const std::string& fullPath);

		/**
		 * Converts the HTTP response object to a string representation.
		 *
		 * @return The string representation of the HTTP response.
		 */
		std::string convertToString() const;

		/**
		 * Determines the content type of the response based on the given request URI.
		 *
		 * @param requestURI The URI of the request.
		 * @return The content type of the response as a string.
		 */
		std::string determineContentType(std::string requestURI);

		/**
		 * @brief Serves a regular file.
		 * 
		 * This function is responsible for serving a regular file specified by the given URI and full path.
		 * 
		 * @param uri The URI of the file to be served.
		 * @param fullPath The full path of the file to be served.
		 */
		void serveRegularFile(const std::string& uri, const std::string& fullPath);

		/**
		 * Checks if a given HTTP method is allowed for a specific URI based on the server configuration.
		 *
		 * @param method The HTTP method to check.
		 * @param uri The URI to check.
		 * @param serverConfig The server configuration containing the allowed methods for each URI.
		 * @return True if the method is allowed for the URI, false otherwise.
		 */
		static bool isMethodAllowed(const std::string& method, const std::string& uri, const ServerConfig& serverConfig);

		/**
		 * Checks if the given URI is a redirection.
		 *
		 * @param uri The URI to check.
		 * @param serverConfig The server configuration.
		 * @return A string indicating the redirection URL if the URI is a redirection, an empty string otherwise.
		 */
		std::string isRedirection(const std::string& uri, const ServerConfig& serverConfig);

		/**
		 * @brief Extracts the folder name from a given URI.
		 *
		 * This function takes a URI as input and extracts the folder name from it.
		 * The folder name is the last component of the URI path, excluding the file name.
		 *
		 * @param uri The URI from which to extract the folder name.
		 * @return The extracted folder name.
		 */
		std::string extractFolderName(const std::string& uri);

		/**
		 * Checks if the given URI contains any cheeky slashes.
		 *
		 * @param uri The URI to check.
		 * @return True if the URI contains cheeky slashes, false otherwise.
		 */
		bool cheekySlashes(const std::string& uri);

		/**
		 * @brief Sets the status code of the HTTP response.
		 *
		 * This function sets the status code of the HTTP response to the specified code.
		 *
		 * @param code The status code to set.
		 */
		void setStatusCode(int code);

		/**
		 * @brief Sets the header with the specified key and value.
		 * 
		 * @param key The key of the header.
		 * @param value The value of the header.
		 */
		void setHeader(const std::string& key, const std::string& value);
		
		/**
		 * @brief Sets the body of the HTTP response.
		 *
		 * This function sets the body of the HTTP response to the specified string.
		 *
		 * @param body The string representing the body of the HTTP response.
		 */
		void setBody(const std::string& body);
};

#endif