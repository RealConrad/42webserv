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
	parseHeaders(requestStream);
    parseBody(requestStream);
	SUCCESS("Recived HTTP Request: " << method << " on " << uri);
}

void HTTPRequest::parseHeaders(std::istringstream& stream) {
    std::string line;
    while (std::getline(stream, line) && line != "\r") {
        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = trim(line.substr(colonPos + 1));
            this->headers[key] = value;
        }
    }
}

void HTTPRequest::parseBody(std::istringstream& stream) {
    std::string contentType = this->headers["Content-Type"];
    if (contentType.find("multipart/form-data") != std::string::npos) {
        std::string boundary = extractBoundary(contentType);
        if (!boundary.empty()) {
            parseMultipartBody(stream, boundary);
        }
    } else {
        std::string contentLength = this->headers["Content-Length"];
        if (!contentLength.empty()) {
            std::istringstream iss(contentLength);
			int length;
			iss >> length;
            this->body.resize(length);
            stream.read(&body[0], length);
        }
    }
}

/* -------------------------------------------------------------------------- */
/*                                File parsing                                */
/* -------------------------------------------------------------------------- */

void HTTPRequest::parseMultipartBody(std::istringstream& stream, const std::string& boundary) {
    std::string part, tempLine;
    while (std::getline(stream, tempLine)) {
        if (tempLine.find(boundary) != std::string::npos) {
            if (!part.empty())
				parseMultipartPart(part);
            part.clear();
            if (tempLine.find(boundary + "--") != std::string::npos)
				break;
        } else {
            part += tempLine + "\n";
        }
    }
}

void HTTPRequest::parseMultipartPart(const std::string& part) {
    std::istringstream partStream(part);
    std::string line;
    bool inHeader = true;
    while (std::getline(partStream, line)) {
        if (line.empty())
			inHeader = false;
        else if (inHeader) {
            if (line.find("filename=") != std::string::npos) {
                this->fileName = extractHeaderValue(line, "filename");
            } else if (line.find("Content-Type:") != std::string::npos) {
                this->fileContentType = extractHeaderValue(line, "Content-Type");
            }
        } else {
            this->body += line + "\n";
        }
    }
}

std::string HTTPRequest::extractHeaderValue(const std::string& header, const std::string& key) {
    size_t startPos = header.find(key) + key.length();
    if (key[key.length() - 1] != '=')
		startPos += 2; // Skip ": "
    size_t endPos = header.find('"', startPos + 1);
    return header.substr(startPos, endPos - startPos);
}

std::string HTTPRequest::extractBoundary(const std::string& contentType) {
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        return "";
    }

    // Extract the boundary value starting just after "boundary="
    std::string boundary = contentType.substr(boundaryPos + 9);
    size_t semicolonPos = boundary.find(";");
    if (semicolonPos != std::string::npos) {
        boundary = boundary.substr(0, semicolonPos);
    }

	boundary = trim(boundary);
    // append '--' --> required by the multipart standard
    boundary = "--" + boundary;
    return boundary;
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

std::string HTTPRequest::getFileName() const {
	return this->fileName;
}

std::string HTTPRequest::getFileContentType() const {
	return this->fileContentType;
}

std::string HTTPRequest::getHeader(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator iter = this->headers.find(name);
	if (iter != this->headers.end())
		return iter->second;
	return "";
}
