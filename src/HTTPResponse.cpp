#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse() {}

HTTPResponse::~HTTPResponse() {}

/* -------------------------------------------------------------------------- */
/*                              Setter Functions                              */
/* -------------------------------------------------------------------------- */

void HTTPResponse::setStatusCode(int code) {
    this->statusCode = code;
}

void HTTPResponse::setHeader(const std::string& key, const std::string& value) {
    this->headers[key] = value;
}

void HTTPResponse::setBody(const std::string& body) {
    this->body = body;
}


// Needs to be more in depth for handling more HTTP methods, content types, and status codes
std::string HTTPResponse::convertToString() const {
    INFO("Converting HTTPResponse into string");
    std::ostringstream responseStream;

    // Status message based on code
    responseStream << "HTTP/1.1 " << this->statusCode << " " << "\r\n";
    for (std::map<std::string, std::string>::const_iterator const_iter = this->headers.begin(); const_iter != this->headers.end(); const_iter++) {
        responseStream << const_iter->first << ": " << const_iter->second << "\r\n";
    }
    responseStream << "\r\n" << this->body;
    return responseStream.str();
}
