#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() {}

HTTPRequest::HTTPRequest(const std::string& request) {
	parseRequest(request);
}

HTTPRequest::~HTTPRequest() {}

void HTTPRequest::parseRequest(const std::string& request) {
	std::istringstream requestStream(request);
	std::string line;
	std::getline(requestStream, line);
	std::istringstream lineStream(line);
	lineStream >> this->method;
	lineStream >> this->uri;
	lineStream >> this->version;
	while (std::getline(requestStream, line)) {
		if (line == "\r")
			break;
		size_t colonPos = line.find(":");
		if (colonPos != std::string::npos) {
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 2, line.length() - colonPos - 3);
			this->headers[key] = value;
		}
	}
	std::string contentLength = getHeader("Content-Length");
	if (!contentLength.empty()) {
		std::istringstream iss(contentLength);
		int length;
		iss >> length;
		this->body.resize(length);
		requestStream.read(&body[0], length);
	}
	SUCCESS("Recived HTTP Request: " << method << " on " << uri);
	BLOCK(request);
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
