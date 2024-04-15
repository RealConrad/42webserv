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
			parseMultipartFile(stream, boundary);
		} else {
			ERROR("Could not find boundry for multipart/form-data");
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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

void HTTPRequest::parseMultipartFile(std::istream& stream, const std::string& boundary) {
    std::string boundaryString = "\r\n" + boundary;
    std::vector<char> boundaryBytes(boundaryString.begin(), boundaryString.end());
    std::vector<char> buffer;
    bool readingHeaders = true;
    std::string filename;
    std::string contentType;
    std::vector<char> fileContent;

    char ch;
    while (stream.get(ch)) {
        buffer.push_back(ch);

        if (readingHeaders) {
            // Check if we reached the end of headers
            if (buffer.size() > 4 && std::equal(buffer.end() - 4, buffer.end(), "\r\n\r\n")) {
                std::string headers(buffer.begin(), buffer.end() - 4); // Exclude the last \r\n\r\n
                std::istringstream headerStream(headers);
                std::string line;
                while (std::getline(headerStream, line)) {
                    if (line.find("Content-Disposition:") != std::string::npos) {
                        size_t namePos = line.find("filename=\"");
                        if (namePos != std::string::npos) {
                            namePos += 10; // Length of 'filename="'
                            size_t endPos = line.find("\"", namePos);
                            filename = line.substr(namePos, endPos - namePos);
                        }
                    } else if (line.find("Content-Type:") != std::string::npos) {
                        size_t startPos = line.find(":") + 2;
                        contentType = line.substr(startPos);
                    }
                }
                // Prepare to read file content
                readingHeaders = false;
                buffer.clear(); // Clear the buffer to start fresh for content
                continue; // Skip the rest of the loop to start content capture
            }
        } else {
            // Check for boundary indicating the end of the file content
            if (buffer.size() >= boundaryBytes.size() &&
                std::equal(boundaryBytes.begin(), boundaryBytes.end(), buffer.end() - boundaryBytes.size())) {
                // Stop capturing file content just before the boundary
                fileContent.insert(fileContent.end(), buffer.begin(), buffer.end() - boundaryBytes.size());
                break;
            }
        }
    }

    this->fileName = filename;
    this->fileContentType = contentType;
    this->body.assign(fileContent.begin(), fileContent.end());
}

std::string HTTPRequest::extractBoundary(const std::string& contentType) const {
	size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos) {
		return "";
	}

	std::string boundary = contentType.substr(boundaryPos + 9);
	size_t semicolonPos = boundary.find(";");
	if (semicolonPos != std::string::npos) {
		boundary = boundary.substr(0, semicolonPos);
	}

	// Ensure boundary is properly prefixed
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
