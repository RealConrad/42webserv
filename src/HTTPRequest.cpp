#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(const std::string& request) {
    parseRequest(request);
}

HTTPRequest::~HTTPRequest() {}

void HTTPRequest::parseRequest(const std::string& request) {
    INFO("Parsing HTTP Request");
    std::istringstream requestStream(request);
    std::string line;

    // First line contains: 'METHOD' 'URI' 'HTTP_VERSION'
    std::getline(requestStream, line);
    std::istringstream lineStream(line);
    lineStream >> this->method;
    lineStream >> this->uri;
    lineStream >> this->version;

    // Parse headers
    while (std::getline(requestStream, line)) {
        if (line == "\r")
            break;
        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos) {
            // break up line into 'key': 'value'
            std::string key = line.substr(0, colonPos);
            // Skip the colon, whitespace and the potential '\r'
            std::string value = line.substr(colonPos + 2, line.length() - colonPos - 3);
            this->headers[key] = value;
        }
    }
    // Parse body (if any)
    std::string contentLength = getHeader("Content-Length");
    if (!contentLength.empty()) {
        std::istringstream iss(contentLength);
        int length;
        iss >> length;
        // Resize the body to be able to hold length bytes
        this->body.resize(length);
        // Read length bytes from requestStream into body
        requestStream.read(&body[0], length);
    }
    SUCCESS("Finished Parsing HTTP Request");
}

/* -------------------------------------------------------------------------- */
/*                              Getter Functions                              */
/* -------------------------------------------------------------------------- */

std::string HTTPRequest::getMethod() const {
    return this->method;
}

std::string HTTPRequest::getURI() const {
    return this->uri;
}

std::string HTTPRequest::getVersion() const {
    return this->version;
}

std::string HTTPRequest::getBody() const {
    return this->body;
}

std::string HTTPRequest::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator iter = this->headers.find(name);
    if (iter != this->headers.end())
        return iter->second;
    return "";
}

void HTTPRequest::printValues() {
    std::cout  << "Method: " << getMethod() << std::endl;
    std::cout  << "URI: " << getURI() << std::endl;
    std::cout  << "Version: " << getVersion() << std::endl;
    std::cout  << "Body: " << getBody() << std::endl;
    std::cout  << "Headers: " << std::endl;
    for (std::map<std::string, std::string>::iterator iter = this->headers.begin(); iter != this->headers.end(); iter++) {
        std::cout << iter->first << ": " << iter->second << std::endl;
    }
}
