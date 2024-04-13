#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse() {}

HTTPResponse::~HTTPResponse() {}

void HTTPResponse::prepareResponse(HTTPRequest& request, const ServerConfig& ServerConfig) {
	std::string method = request.getMethod();

	if (!isMethodAllowed(method, request.getURI(), ServerConfig)) {
		throw std::runtime_error("Method '" + method + "' not allowed for server '" + ServerConfig.serverName + "'");
	}

	switch (stringToRequestType(method)) {
		case GET:
			handleRequestGET(request, ServerConfig);
			break;
		case POST:
			// handleRequestPOST(request, ServerConfig);
			break;
		case DELETE:
			ERROR("DELETE REQUEST NOT IMPLEMENTED YET!");
			break;
		default:
			// TODO: SEND ERROR BACK TO CLIENT
			throw std::runtime_error("Method '" + method + "' not implemented");
			break;
	}
}

void HTTPResponse::handleRequestGET(const HTTPRequest& request, const ServerConfig& serverConfig) {
	std::string requestURI = request.getURI();
	
	if (requestURI == "/get-images") {
		INFO("/get-images endpoint called for server: " << serverConfig.serverName);
		// TODO: CHANGE THIS TO READ FROM A DIRECTORY MAYBE?
		std::vector<std::string> images;
		images.push_back("/images/image1.jpg");
		images.push_back("/images/image2.jpg");
		images.push_back("/images/image3.jpg");
		// Get random image
		std::string imagePath = serverConfig.rootDirectory + images[rand() % images.size()];
		std::ifstream file(imagePath.c_str());
		INFO("Serving image: " << imagePath);
		if (file) {
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			assignResponse(200, content, determineContentType(imagePath));
			file.close();
		} else {
			WARNING("Image: '" << imagePath << "' not found. Serving 404 page");
			assignPageNotFoundContent(serverConfig);
		}
	} else {
		serveFile(serverConfig, requestURI);
	}
}

void HTTPResponse::handleRequestPOST(const HTTPRequest& request, const ServerConfig& serverConfig) {
	std::string requestURI = request.getURI();

	if (requestURI == "/upload-file") {
		INFO("File upload endpoint called for server: " << serverConfig.serverName);
	}
}

void HTTPResponse::serveFile(const ServerConfig& serverConfig, const std::string& uri) {
	std::string filePath = serverConfig.rootDirectory + (uri == "/" ? "/index.html" : uri);
	std::ifstream file(filePath.c_str());
	INFO("Serving file: " << filePath);
	if (file) {
		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		assignResponse(200, content, determineContentType(uri));
		file.close();
	} else {
		WARNING("File '" << filePath << "' not found. Serving 404 page");
		assignPageNotFoundContent(serverConfig);
	}
}

void HTTPResponse::assignPageNotFoundContent(const ServerConfig& serverConfig) {
	std::string notFoundPagePath = serverConfig.rootDirectory + "/pages/NotFound.html";
	std::ifstream notFoundFile(notFoundPagePath.c_str());
	std::string notFoundContent;

	if (notFoundFile) {
		notFoundContent.assign((std::istreambuf_iterator<char>(notFoundFile)), std::istreambuf_iterator<char>());
		notFoundFile.close();
	} else {
		// Fallback if the NotFound.html file does not exist in given root directory
		notFoundContent = "<html><body><h1>404 Not Found</h1><p>The requested file was not found.</p></body></html>";
	}
	assignResponse(404, notFoundContent, "text/html");
}

void HTTPResponse::assignResponse(int statusCode, const std::string& body, std::string contentType) {
	setHeader("Content-Type", contentType);
	setHeader("Content-Length", ::toString(body.size() - 1));
	setHeader("Connection", "keep-alive");
	setHeader("Keep-Alive", "timeout=60, max=50");
	setBody(body);
	setStatusCode(statusCode);
}

std::string HTTPResponse::determineContentType(std::string requestURI) {
	if (endsWith(requestURI, ".css")) {
		return "text/css";
	} else if (endsWith(requestURI, ".jpg") || endsWith(requestURI, ".jpeg")) {
		return "image/jpeg";
	} else if (endsWith(requestURI, ".png")) {
		return "image/png";
	}
	return "text/html";
}

// Needs to be more in depth for handling more HTTP methods, content types, and status codes
std::string HTTPResponse::convertToString() const {
	std::ostringstream responseStream;
	responseStream << "HTTP/1.1 " << this->statusCode << " " << "\r\n";
	for (std::map<std::string, std::string>::const_iterator const_iter = this->headers.begin(); const_iter != this->headers.end(); const_iter++) {
		responseStream << const_iter->first << ": " << const_iter->second << "\r\n";
	}
	responseStream << "\r\n" << this->body;
	return responseStream.str();
}

bool HTTPResponse::isMethodAllowed(const std::string& method, const std::string& uri, const ServerConfig& serverConfig) {
	for (std::vector<LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
		if (uri.find(it->locationPath) == 0) {
			for (std::vector<RequestTypes>::const_iterator iter = it->allowedRequestTypes.begin(); iter != it->allowedRequestTypes.end(); ++iter) {
				if (method == requestTypeToString(*iter)) {
					return true;
				}
			}
			// If the URI matches but the method is not allowed, return false
			return false;
		}
	}
	return false;
}

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
