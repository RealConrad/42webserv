#include "HTTPResponse.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

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
			ERROR("POST REQUEST NOT IMPLEMENTED YET!");
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
		images.push_back("/data/image1.jpg");
		images.push_back("/data/image2.jpg");
		images.push_back("/data/image3.jpg");

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

std::string HTTPResponse::extractFolderName(const std::string& uri) {
	if (uri.empty())
		return ("");
	size_t lastSlashPos;
	if (uri[uri.size() - 1] == '/')
		lastSlashPos = uri.find_last_of('/', uri.length() - 2);
	else
		lastSlashPos = uri.find_last_of('/');
	if (lastSlashPos == std::string::npos || lastSlashPos == uri.length() - 1)
		return ("");
	size_t start = lastSlashPos + 1;
	size_t end = uri.find('/', start);
	if (end == std::string::npos)
		end = uri.length();
	return (uri.substr(start, end - start));
}

bool HTTPResponse::serveIndex(const ServerConfig& serverConfig){
		std::string indexPath = serverConfig.rootDirectory + (serverConfig.rootDirectory[serverConfig.rootDirectory.size() - 1] == '/' ? "" : "/") + "index.html";
		std::ifstream indexFile(indexPath.c_str());
		if (!indexFile.fail()) {
			INFO("Serving index: " << indexPath);
			std::string content((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
			assignResponse(200, content, "text/html");
			indexFile.close();
			return (true);
		} else if (indexFile.fail())
			WARNING("Failed to open index.html!");
		return (false);
}

bool HTTPResponse::serveDefaultFile(const std::string& uri, const std::string& fullPath){
		std::string folderNameHtml = fullPath + (fullPath[fullPath.size() - 1] == '/' ? "" : "/") + extractFolderName(uri) + ".html";
		std::ifstream folderHtmlFile(folderNameHtml.c_str());
		if (!folderHtmlFile.fail()) {
			INFO("Serving Default File for Folder: " << folderNameHtml);
			std::string content((std::istreambuf_iterator<char>(folderHtmlFile)), std::istreambuf_iterator<char>());
			assignResponse(200, content, "text/html");
			folderHtmlFile.close();
			return (true);
		} else
			WARNING("Failed to open " << folderNameHtml);
		return (false);
}

void HTTPResponse::serveDirectoryListing(const ServerConfig& serverConfig, const std::string& uri, const std::string& fullPath){
	DIR* dir = opendir(fullPath.c_str());
	if (dir != NULL) {
		INFO("Serving Directory Listing of: " << fullPath);
		struct dirent* entry;
		std::string content = "<html><body><h1>Directory Listing of " + uri + "</h1><ul>";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			std::string name = entry->d_name;
			std::string link = uri + (uri[uri.size() - 1] == '/' ? "" : "/") + name;
			content += "<li><a href='" + link + "'>" + name + "</a></li>";
		}
		content += "</ul></body></html>";
		closedir(dir);
		assignResponse(200, content, "text/html");
	} else {
		WARNING("Failed to open directory: '" << fullPath << "'. Serving 404 page");
		assignPageNotFoundContent(serverConfig);
	}
}

void HTTPResponse::serveRegularFile(const ServerConfig& serverConfig, const std::string& uri, const std::string& fullPath){
	std::ifstream file(fullPath.c_str());
	if (!file.fail()) {
		INFO("Serving file: " << fullPath);
		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		assignResponse(200, content, determineContentType(uri));
		file.close();
	} else {
		WARNING("File '" << fullPath << "' not found. Serving 404 page");
		assignPageNotFoundContent(serverConfig);
	}
}

bool HTTPResponse::cheekySlashes(const std::string& uri){
	if (uri.size() == 0)
		return (true);
	for (size_t i = 0; i < uri.size(); i++)
	{
		if (uri[i] != '/')
			return (false);
	}
	return (true);
}

void HTTPResponse::serveFile(const ServerConfig& serverConfig, const std::string& uri) {
	std::string fullPath = serverConfig.rootDirectory + uri;
	struct stat path_stat;
	stat(fullPath.c_str(), &path_stat);
	if (S_ISDIR(path_stat.st_mode)) {
		if (cheekySlashes(uri) && serveIndex(serverConfig))
			return;
		if (serveDefaultFile(uri, fullPath))
			return;
		if (serverConfig.directoryListing)
			serveDirectoryListing(serverConfig, uri, fullPath);
		else
			assignPageNotFoundContent(serverConfig);
	} else if (S_ISREG(path_stat.st_mode)) {
			serveRegularFile(serverConfig, uri, fullPath);
	} else {
		WARNING("Path '" << fullPath << "' could not be recognised! Serving 404 page");
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
