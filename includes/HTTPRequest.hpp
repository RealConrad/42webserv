#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <fstream>
#include <vector>
#include "Logger.hpp"
#include "Utils.hpp"

class HTTPRequest {
	private:
		std::string method;
		std::string uri;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
		std::string fileName;
		std::string fileContentType;

		/**
		 * @brief Parses the headers from the given input stream.
		 *
		 * This function reads the headers from the provided input stream and processes them accordingly.
		 * The headers are expected to be in the format of "key: value" pairs, with each pair separated by a newline.
		 *
		 * @param stream The input stream containing the headers.
		 */
		void parseHeaders(std::istringstream& stream);

		/**
		 * @brief Parses the body of the HTTP request.
		 *
		 * This function takes an `std::istringstream` object as input and parses the body of the HTTP request.
		 * The parsed body is then stored or processed as required by the application (e.g., form data/normal body).
		 *
		 * @param stream The input stream containing the body of the HTTP request.
		 */
		void parseBody(std::istringstream& stream);

		/**
		 * @brief Extracts the value associated with a given key from a header string.
		 *
		 * This function searches for the specified key in the header string and returns
		 * the corresponding value. If the key is not found, an empty string is returned.
		 *
		 * @param header The header string to search in.
		 * @param key The key to search for.
		 * @return The value associated with the key, or an empty string if the key is not found.
		 */
		std::string extractHeaderValue(const std::string& header, const std::string& key);

		/**
		 * @brief Parses a multi-part file from the given input stream using the specified boundary.
		 *
		 * This function reads the input stream and extracts the contents of a multi-part file
		 * that is delimited by the specified boundary.
		 *
		 * @param stream The input stream from which to read the multi-part file.
		 * @param boundary The boundary string that delimits the multi-part file.
		 */
		void parseMultipartFile(std::istream& stream, const std::string& boundary);

		/**
		 * @brief Parses the multipart headers from the given buffer.
		 *
		 * This function takes a vector of characters as input and parses the multipart headers from it.
		 * The parsed headers are then used for further processing.
		 *
		 * @param buffer The vector of characters containing the multipart headers.
		 */
		void parseMultipartHeaders(const std::vector<char>& buffer);

		/**
		 * Extracts the boundary string from the given content type.
		 *
		 * @param contentType The content type string.
		 * @return The boundary string extracted from the content type.
		 */
		std::string extractBoundary(const std::string& contentType) const;
	public:
		HTTPRequest();
		HTTPRequest(const std::string& request);
		~HTTPRequest();

		/**
		 * @brief Parses the given request.
		 *
		 * This function parses the provided request and performs necessary operations based on the request data.
		 *
		 * @param request The request to be parsed.
		 */
		void parseRequest(const std::string& request);

		/**
		 * @brief Gets the HTTP method of the request.
		 *
		 * This function returns the HTTP method (e.g., GET, POST, PUT, DELETE) of the request.
		 *
		 * @return The HTTP method of the request as a string.
		 */
		std::string getMethod() const;

		/**
		 * @brief Returns the URI (Uniform Resource Identifier) of the HTTP request.
		 *
		 * @return The URI of the HTTP request as a string.
		 */
		std::string getURI() const;

		/**
		 * @brief Get the version of the HTTP request.
		 *
		 * This function returns the version of the HTTP request as a string.
		 *
		 * @return The version of the HTTP request.
		 */
		std::string getVersion() const;

		/**
		 * Retrieves the value of the specified header.
		 *
		 * @param name The name of the header to retrieve.
		 * @return The value of the header, or an empty string if the header is not found.
		 */
		std::string getHeader(const std::string& name) const;

		/**
		 * @brief Gets the body of the HTTP request.
		 *
		 * This function returns the body of the HTTP request as a string.
		 *
		 * @return The body of the HTTP request.
		 */
		std::string getBody() const;

		/**
		 * @brief Gets the file name from the HTTP request.
		 * 
		 * This function returns the file name extracted from the HTTP request.
		 * 
		 * @return The file name as a string.
		 */
		std::string getFileName() const;

		/**
		 * @brief Gets the content type of the file associated with the HTTP request.
		 *
		 * @return The content type of the file as a string.
		 */
		std::string getFileContentType() const;
};

#endif