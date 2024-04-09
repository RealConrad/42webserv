#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <algorithm>
#include "Logger.hpp"

class HTTPRequest {
    private:
        std::string method;
        std::string uri;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;

    public:
        HTTPRequest();
        HTTPRequest(const std::string& request);
        ~HTTPRequest();
        
        void parseRequest(const std::string& request);
        void printValues();

        std::string getMethod() const;
        std::string getURI() const;
        std::string getVersion() const;
        std::string getHeader(const std::string& name) const;
        std::string getBody() const;
};

#endif