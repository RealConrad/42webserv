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

        void parseHeaders(std::istringstream& stream);
        void parseBody(std::istringstream& stream);
        std::string extractHeaderValue(const std::string& header, const std::string& key);
		void parseMultipartFile(std::istream& stream, const std::string& boundary);
		void parseMultipartHeaders(const std::vector<char>& buffer);
        std::string extractBoundary(const std::string& contentType) const;
    public:
        HTTPRequest();
        HTTPRequest(const std::string& request);
        ~HTTPRequest();

        void parseRequest(const std::string& request);

        std::string getMethod() const;
        std::string getURI() const;
        std::string getVersion() const;
        std::string getHeader(const std::string& name) const;
        std::string getBody() const;
        std::string getFileName() const;
        std::string getFileContentType() const;
};

#endif