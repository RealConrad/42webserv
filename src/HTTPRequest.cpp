#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() {}

HTTPRequest::HTTPRequest(const std::string& request) {
	parseRequest(request);
}

HTTPRequest::~HTTPRequest() {}

void HTTPRequest::parseRequest(const std::string& request) {
	std::istringstream requestStream(request);
	parseHeaders(requestStream);
	parseBody(requestStream);
	SUCCESS("Recived HTTP Request: " << method << " on " << uri);
}

void HTTPRequest::parseHeaders(std::istringstream& stream) {
	std::string line;
	std::getline(stream, line);
	std::istringstream lineStream(line);
	lineStream >> this->method;
	lineStream >> this->uri;
	lineStream >> this->version;
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

void HTTPRequest::parseMultipartFile(std::istringstream& stream, const std::string& boundary) {
	std::string tempLine;
	bool fileSection = false;
	bool startCapture = false;
	std::ostringstream fileStream;

	while (getline(stream, tempLine)) {
		// Normalize line endings to Unix style for consistent processing
		if (!tempLine.empty() && tempLine.back() == '\r') {
			tempLine.pop_back();
		}
		if (tempLine == boundary) {
			if (fileSection) {
				// DEBUG("End of file section detected.");
				break; // End of file section, break the loop
			} else {
				fileSection = true;
				continue;
			}
		}

		if (fileSection && !startCapture) {
			// Process headers
			if (tempLine.find("Content-Disposition:") != std::string::npos) {
				size_t namePos = tempLine.find("filename=\"");
				if (namePos != std::string::npos) {
					size_t start = namePos + 10; // Length of 'filename="'
					size_t end = tempLine.find("\"", start);
					this->fileName = tempLine.substr(start, end - start);
				}
			} else if (tempLine.find("Content-Type:") != std::string::npos) {
				this->fileContentType = tempLine.substr(tempLine.find(":") + 2);
			} else if (tempLine.empty()) {
				startCapture = true;  // Start capturing file content after this
				continue;
			}
		}

		if (fileSection && startCapture) {
			if (tempLine == (boundary + "--")) {
				break; // Stop if we reach the final boundary
			}
			fileStream << tempLine << "\n";
		}
	}

	this->body = fileStream.str();
	if (!body.empty()) {
		this->body.pop_back(); // remove trailing new line
	}
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
