#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

#include <map>
#include <string>
#include <sstream>
#include "Logger.hpp"

class HTTPResponse {
    private:
        int statusCode;
        std::map<std::string, std::string> headers;
        std::string body;
    public:
        HTTPResponse();
        ~HTTPResponse();
        void setStatusCode(int code);
        void setHeader(const std::string& key, const std::string& value);
        void setBody(const std::string& body);
        std::string convertToString() const;
		void prepareResponse(HTTPResponse& response, int statusCode, const std::string& body, std::string contentType);

};

#endif