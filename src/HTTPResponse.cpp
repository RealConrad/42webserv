#include "HTTPResponse.hpp"

const std::map<int, std::string> HTTPResponse::statusCodes = HTTPResponse::initializeStatusCodes();

HTTPResponse::HTTPResponse() {}

HTTPResponse::~HTTPResponse() {}

std::map<int, std::string> HTTPResponse::initializeStatusCodes() {
	std::map<int, std::string> statusCodes;
	statusCodes[200] = "OK";
	statusCodes[201] = "Created";
	statusCodes[202] = "Accepted";
	statusCodes[301] = "Moved Permanently";
	statusCodes[302] = "Found";
	statusCodes[400] = "Bad Request";
	statusCodes[403] = "Forbidden";
	statusCodes[404] = "Not Found";
	statusCodes[405] = "Method Not Allowed";
	statusCodes[408] = "Request Timeout";
	statusCodes[500] = "Internal Server Error";
	statusCodes[501] = "Not Implemented";
	statusCodes[504] = "Gateway Timeout";
	return statusCodes;
}

void HTTPResponse::prepareResponse(HTTPRequest& request, const ServerConfig& serverConfig) {
	std::string method = request.getMethod();
	// std::string uri = request.getURI();
	if (!isMethodAllowed(method, request.getURI(), serverConfig)) {
		assignGenericResponse(405);
		ERROR("Method '" << method << "' not allowed for server '" << serverConfig.serverName << request.getURI() <<"'");
		return;
	}

	switch (stringToRequestType(method)) {
		case GET:
			handleRequestGET(request, serverConfig);
			break;
		case POST:
			handleRequestPOST(request, serverConfig);
			break;
		case DELETE:
			handleRequestDELETE(request, serverConfig);
			break;
		default:
			assignGenericResponse(501);
			throw std::runtime_error("Method '" + method + "' not implemented");
			break;
	}
}

void HTTPResponse::handleRequestGET(const HTTPRequest& request, const ServerConfig& serverConfig) {
	std::string requestURI = request.getURI();
	static int image = 0;
	
	if (requestURI == "/get-images") {
		image++;
		INFO("/get-images endpoint called for server: " << serverConfig.serverName);
		std::vector<std::string> images;
		images.push_back("/images/image1.jpg");
		images.push_back("/images/image2.jpg");
		images.push_back("/images/image3.jpg");
		std::string imagePath = serverConfig.rootDirectory + images[image % images.size()];
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

//TODO: DELETE
	// else if (requestURI == "/get-files") {
	// 	INFO("/get-files endpoint called for server: " << serverConfig.serverName);
	// 	std::string dirPath = serverConfig.rootDirectory + "/uploads"; 
	// 	DIR *dir = opendir(dirPath.c_str());
	// 	if (dir) {
	// 		std::vector<std::string> fileNames;
	// 		struct dirent* entry;
	// 		while (true) {
	// 			entry = readdir(dir);
	// 			if (!entry)
	// 				break;
	// 			if (entry->d_name[0] == '.')
	// 				continue;
	// 			fileNames.push_back(entry->d_name);
	// 		}
	// 		std::string json = "[";
	// 		for (size_t i = 0; i < fileNames.size(); ++i) {
	// 			json += "\"" + fileNames[i] + "\"";
	// 			if (i < fileNames.size() - 1)
	// 				json += ", ";
	// 		}
	// 		json += "]";
	// 		assignResponse(200, json, "application/json");
	// 	} else {
	// 		ERROR("Directory path not found: " << dirPath);
	// 		assignPageNotFoundContent(serverConfig);
	// 	}
	// } 

void HTTPResponse::handleRequestPOST(const HTTPRequest& request, const ServerConfig& serverConfig) {
	std::string requestURI = request.getURI();

	std::string savePath = serverConfig.rootDirectory + request.getURI();
	DEBUG("URI: " << request.getURI());
	DEBUG("Saving file to: " << savePath);
	std::ofstream outFile(savePath.c_str());
	if (outFile) {
		outFile.write(request.getBody().c_str(), request.getBody().size());
		outFile.close();

		if (!outFile.fail()) {
			setHeader("Location", serverConfig.rootDirectory + requestURI);
			INFO("File uploaded successfully: " + savePath);
			assignGenericResponse(201);
		} else {
			ERROR("Failed to store file");
			assignGenericResponse(500);
		}
	} else {
		ERROR("Unable to open file for writing: " + savePath);
		assignGenericResponse(500);
	}
}

void HTTPResponse::handleRequestDELETE(const HTTPRequest& request, const ServerConfig& serverConfig) {
	std::string requestURI = request.getURI();

	INFO("DELETE method called for server: " << serverConfig.serverName);
	requestURI = serverConfig.rootDirectory + requestURI; 
	if (access(requestURI.c_str(), F_OK) != 0) {
		ERROR("File does not exist: " + requestURI);
		assignGenericResponse(404);
		return;
	}
	if (remove(requestURI.c_str()) == 0) {
		SUCCESS("Deleted file: " << requestURI);
		assignGenericResponse(200);
	} else {
		ERROR("Could not delete file: " << requestURI);
		assignGenericResponse(500);
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
		std::string indexPath = serverConfig.rootDirectory + (serverConfig.rootDirectory[serverConfig.rootDirectory.size() - 1] == '/' ? "" : "/") + serverConfig.indexFile;
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

bool HTTPResponse::serveDefaultFile(const std::string& uri, const std::string& fullPath) {
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

void HTTPResponse::serveDirectoryListing(const ServerConfig& serverConfig, const std::string& uri, const std::string& fullPath) {
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

void HTTPResponse::serveDeletePage(const ServerConfig& serverConfig, const std::string& uri, const std::string& fullPath) {
	DIR* dir = opendir(fullPath.c_str());
	if (dir != NULL) {
		INFO("Serving Delete page of: " << fullPath);
		struct dirent* entry;
		std::string content = "<html><body><h1>Files of " + uri + "</h1><ul>";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			std::string name = entry->d_name;
			std::string link = uri + (uri[uri.size() - 1] == '/' ? "" : "/") + name;
			std::string deleteButton = "<button onclick=\"fetch(origin + '" + link + "', {method: 'DELETE'})\">Delete</button>";

			content += "<li><a href='" + link + "'>" + name + "</a>" + deleteButton + "</li>";
		}
		content += "</ul></body></html>";
		closedir(dir);
		assignResponse(200, content, "text/html");
	} else {
		WARNING("Failed to open directory: '" << fullPath << "'. Serving 404 page");
		assignPageNotFoundContent(serverConfig);
	}
}

void HTTPResponse::serveRegularFile(const ServerConfig& serverConfig, const std::string& uri, const std::string& fullPath) {
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

bool HTTPResponse::cheekySlashes(const std::string& uri) {
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
		if (uri == "/uploads/")
			serveDeletePage(serverConfig, uri, fullPath);
		else if (serverConfig.directoryListing)
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

std::string HTTPResponse::determineContentType(std::string requestURI) {
	if (endsWith(requestURI, ".css")) {
		return "text/css";
	} else if (endsWith(requestURI, ".jpg") || endsWith(requestURI, ".jpeg")) {
		return "image/jpeg";
	} else if (endsWith(requestURI, ".png")) {
		return "image/png";
	} else if (endsWith(requestURI, ".ico")) {
		return "image/x-icon";
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
	const LocationConfig* mostSpecificMatch = NULL;
	// Iterate over the location configurations to find the most specific matching location block
	for (std::vector<LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
		if (uri.find(it->locationPath) == 0) {  // Check if the URI starts with the location path
			if (!mostSpecificMatch || it->locationPath.length() > mostSpecificMatch->locationPath.length()) {
				mostSpecificMatch = &(*it);  // Update to the more specific location match
			}
		}
	}
	// Check the found most specific location
	if (mostSpecificMatch) {
		if (mostSpecificMatch->allowedRequestTypes.empty()) {
			return false;  // If no types are allowed, return false
		}
		// Check if the method is allowed in the most specific location
		for (std::vector<RequestTypes>::const_iterator iter = mostSpecificMatch->allowedRequestTypes.begin(); iter != mostSpecificMatch->allowedRequestTypes.end(); ++iter) {
			if (method == requestTypeToString(*iter)) {
				return true;
			}
		}
		return false;  // If method not found in the allowed types, return false
	}
	// Default to false if no location configuration matches the URI
	return false;
}

/* -------------------------------------------------------------------------- */
/*                             Creating Responses                             */
/* -------------------------------------------------------------------------- */

void HTTPResponse::assignResponse(int statusCode, const std::string& body, std::string contentType) {
	setHeader("Content-Type", contentType);
	setHeader("Content-Length", ::toString(body.size()));
	setHeader("Connection", "keep-alive");
	setHeader("Keep-Alive", "timeout=60, max=50");
	setBody(body);
	setStatusCode(statusCode);
}

void HTTPResponse::assignGenericResponse(int statusCode) {
	std::ostringstream stream;
    std::string code = ::toString(statusCode);
    std::string message = statusCodes.find(statusCode) != statusCodes.end() ? statusCodes.at(statusCode) : "Unknown Code In Map";

	stream << "<!DOCTYPE html>"
			<< "<html lang=\"en\">"
			<< "<head>"
			<< "<meta charset=\"UTF-8\">"
			<< "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
			<< "<title>Webserv - " << code << "</title>"
			<< "<link rel=\"stylesheet\" href=\"styles.css\">"
			<< "<link rel=\"icon\" type=\"image/x-icon\" href=\"favicon.ico\">"
			<< "</head>"
			<< "<body class=\"background\">"
			<< "<div class=\"credits\">"
			<< "Made by "
			<< "<a target=\"_blank\" href=\"https://github.com/RealConrad\" class=\"github-link\">Conrad Wenz</a> & "
			<< "<a target=\"_blank\" href=\"https://github.com/kglebows\" class=\"github-link\">Konrad Glebowski</a>"
			<< "</div>"
			<< "<div>" << code << " - " << message << "</div>"
			<< "</body>"
			<< "</html>";
	assignResponse(statusCode, stream.str(), "text/html");
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
