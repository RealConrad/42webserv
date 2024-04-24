#include "HTTPResponse.hpp"

const std::map<int, std::string> HTTPResponse::statusCodes = HTTPResponse::initializeStatusCodes();

HTTPResponse::HTTPResponse() {}

HTTPResponse::~HTTPResponse() {}

std::map<int, std::string> HTTPResponse::initializeStatusCodes() {
	std::map<int, std::string> statusCodes;
	statusCodes[200] = "OK";
	statusCodes[201] = "Created";
	statusCodes[302] = "Found";
	statusCodes[403] = "Forbidden";
	statusCodes[404] = "Not Found";
	statusCodes[405] = "Method Not Allowed";
	statusCodes[408] = "Request Timeout";
	statusCodes[413] = "Payload Too Large";
	statusCodes[500] = "Internal Server Error";
	statusCodes[501] = "Not Implemented";
	return statusCodes;
}

void HTTPResponse::prepareResponse(HTTPRequest& request, ClientState& client) {
	std::string method = request.getMethod();
	if (!isMethodAllowed(method, request.getURI(), client.serverConfig)) {
		assignGenericResponse(405);
		ERROR("Method '" << method << "' not allowed for server '" << client.serverConfig.serverName << request.getURI() <<"'");
		return;
	}
	std::string redirection = isRedirection(request.getURI(), client.serverConfig);
	if (!redirection.empty()) {
		WARNING("Redirecting client to: " << redirection);
		if (redirection.substr(0, 7) != "http://" && redirection.substr(0, 8) != "https://") {
			redirection = "http://" + redirection;
		}
		setHeader("Location", redirection);
		setStatusCode(302);
		setBody("");
		return;
	}
	switch (stringToRequestType(method)) {
		case GET:
			handleRequestGET(request, client);
			break;
		case POST:
			handleRequestPOST(request, client.serverConfig);
			break;
		case DELETE:
			handleRequestDELETE(request, client.serverConfig);
			break;
		default:
			assignGenericResponse(501);
			throw std::runtime_error("Method '" + method + "' not implemented");
			break;
	}
}

/* -------------------------------------------------------------------------- */
/*                                 GET Method                                 */
/* -------------------------------------------------------------------------- */

void HTTPResponse::handleRequestGET(const HTTPRequest& request, ClientState& client) {
	std::string requestURI = request.getURI();
	static int image = 0;
	
	if (requestURI == "/get-images") {
		image++;
		INFO("/get-images endpoint called for server: " << client.serverConfig.serverName);
		std::vector<std::string> images;
		images.push_back("/images/image1.jpg");
		images.push_back("/images/image2.jpg");
		images.push_back("/images/image3.jpg");
		std::string imagePath = client.serverConfig.rootDirectory + images[image % images.size()];
		std::ifstream file(imagePath.c_str());
		INFO("Serving image: " << imagePath);
		if (file) {
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			assignResponse(200, content, determineContentType(imagePath));
			file.close();
		} else {
			WARNING("Image: '" << imagePath << "' not found. Serving 404 page");
			assignGenericResponse(404, "These Are Not the Images You Are Looking For");
		}
	} else {
		serveFile(client, requestURI);
	}
}

/* -------------------------------------------------------------------------- */
/*                                 POST Method                                */
/* -------------------------------------------------------------------------- */

void HTTPResponse::handleRequestPOST(const HTTPRequest& request, const ServerConfig& serverConfig) {
	std::string requestURI = request.getURI();
	std::string savePath = serverConfig.rootDirectory + requestURI + request.getFileName();

	bool fileExists = (access(savePath.c_str(), F_OK) != -1);
	if (fileExists) {
		WARNING("File already exists: " + savePath);
		setHeader("Location", requestURI + request.getFileName());
		setStatusCode(302);
		setBody("");
		return;
	}
	std::ofstream outFile(savePath.c_str());
	if (outFile) {
		outFile.write(request.getBody().c_str(), request.getBody().size());
		outFile.close();
		if (!outFile.fail()) {
			INFO("File uploaded successfully: " + savePath);
			assignGenericResponse(201, savePath);
		} else {
			ERROR("Failed to store file");
			assignGenericResponse(500);
		}
	} else {
		ERROR("Unable to open file for writing: " + savePath);
		assignGenericResponse(500);
	}
}

/* -------------------------------------------------------------------------- */
/*                                DELETE Method                               */
/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */
/*                              Helper Functions                              */
/* -------------------------------------------------------------------------- */

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

void HTTPResponse::serveDirectoryListing(const std::string& uri, const std::string& fullPath) {
	DIR* dir = opendir(fullPath.c_str());
	if (dir != NULL) {
		INFO("Serving Directory Listing of: " << fullPath);
		struct dirent* entry;
		std::string content = "";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			std::string name = entry->d_name;
			std::string link = uri + (uri[uri.size() - 1] == '/' ? "" : "/") + name;
			content += "<li><a href='" + link + "'>" + name + "</a></li>";
		}
		closedir(dir);
		std::ostringstream stream;
		stream << "<!DOCTYPE html>"
			<< "<html lang=\"en\">"
			<< "<head>"
			<< "<meta charset=\"UTF-8\">"
			<< "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
			<< "<title>Directory Listing of " << uri << "</title>"
			<< "<link rel=\"stylesheet\" href=\"\\styles.css\">"
			<< "<link rel=\"icon\" type=\"image/x-icon\" href=\"favicon.ico\">"
			<< "</head>"
			<< "<body class=\"background\">"
			<< "<div class=\"error\">Directory Listing of " << uri << "</div>"
			<< "<hr>"
			<< "<div class=\"info\">" << content << "</div>"
			<< "<button onclick=\"window.history.back()\" class=\"back-button\">Back</button>"
			<< "</body>"
			<< "</html>";
		assignResponse(200, stream.str(), "text/html");
	} else {
		WARNING("Failed to open directory: '" << fullPath << "'. Serving 404 page");
		assignGenericResponse(404, "This should never happen! HOW?!");
	}
}

void HTTPResponse::serveDeletePage(const std::string& uri, const std::string& fullPath) {
	DIR* dir = opendir(fullPath.c_str());
	if (dir != NULL) {
		INFO("Serving Delete page of: " << fullPath);
		struct dirent* entry;
		std::string content = "";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			std::string name = entry->d_name;
			std::string link = uri + (uri[uri.size() - 1] == '/' ? "" : "/") + name;
			std::string deleteButton = "<button onclick=\""
									"fetch('" + link + "', {method: 'DELETE'})"
									".then(function(response) { "
									"if (response.ok) { "
									"window.location.reload();"
									"} else { "
									"alert('Delete failed with status: ' + response.status);"
									"}"
									"})"
									".catch(function(error) {"
									"alert('Network error or no response from server');"
									"})\">"
									"Delete</button>";
			content += "<li><a href='" + link + "'>" + name + "</a>" + deleteButton + "</li>";
		}
		closedir(dir);
		std::ostringstream stream;
		stream << "<!DOCTYPE html>"
			<< "<html lang=\"en\">"
			<< "<head>"
			<< "<meta charset=\"UTF-8\">"
			<< "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
			<< "<title>Delete page of " << uri << "</title>"
			<< "<link rel=\"stylesheet\" href=\"\\styles.css\">"
			<< "<link rel=\"icon\" type=\"image/x-icon\" href=\"favicon.ico\">"
			<< "</head>"
			<< "<body class=\"background\">"
			<< "<div class=\"error\">Delete page of " << uri << "</div>"
			<< "<hr>"
			<< "<div class=\"info\">" << content << "</div>"
			<< "<button onclick=\"window.history.back()\" class=\"back-button\">Back</button>"
			<< "</body>"
			<< "</html>";
		assignResponse(200, stream.str(), "text/html");
	} else {
		WARNING("Failed to open directory: '" << fullPath << "'. Serving 404 page");
		assignGenericResponse(404, "This should never happen. Yet it did. How?");
	}
}

void HTTPResponse::serveRegularFile(const std::string& uri, const std::string& fullPath) {
	std::ifstream file(fullPath.c_str());
	if (!file.fail()) {
		INFO("Serving file: " << fullPath);
		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		assignResponse(200, content, determineContentType(uri));
		file.close();
	} else {
		WARNING("File '" << fullPath << "' not found. Serving 404 page");
		assignGenericResponse(404, "These Are Not the Files You Are Looking For");
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

void HTTPResponse::serveFile(ClientState& client, const std::string& uri) {
	std::string fullPath = client.serverConfig.rootDirectory + uri;
	struct stat path_stat;
	stat(fullPath.c_str(), &path_stat);
	if (S_ISDIR(path_stat.st_mode)) {
		if (cheekySlashes(uri) && serveIndex(client.serverConfig))
			return;
		if (serveDefaultFile(uri, fullPath))
			return;
		if (uri == "/uploads")
			serveDeletePage(uri, fullPath);
		else if (client.serverConfig.directoryListing)
			serveDirectoryListing(uri, fullPath);
		else
			assignGenericResponse(405, "This Directory is over 9000!!!");
	} else if (S_ISREG(path_stat.st_mode)) {
		serveRegularFile(uri, fullPath);
	} else {
		WARNING("Path '" << fullPath << "' could not be recognised! Serving 404 page");
		assignGenericResponse(404, "These Are Not the Files You Are Looking For");
	}
}

std::string HTTPResponse::determineContentType(std::string requestURI) {
	if (endsWith(requestURI, ".css")) {
		return "text/css";
	} else if (endsWith(requestURI, ".jpg") || endsWith(requestURI, ".jpeg")) {
		return "image/jpeg";
	} else if (endsWith(requestURI, ".png")) {
		return "image/png";
	} else if (endsWith(requestURI, ".pdf")) {
		return "application/pdf";
	} else if (endsWith(requestURI, ".ico")) {
		return "image/x-icon";
	}
	return "text/html";
}

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
	for (std::vector<LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
		if (uri.find(it->locationPath) == 0) {
			if (!mostSpecificMatch || it->locationPath.length() > mostSpecificMatch->locationPath.length()) {
				mostSpecificMatch = &(*it);
			}
		}
	}
	if (mostSpecificMatch) {
		if (mostSpecificMatch->allowedRequestTypes.empty()) {
			return false;
		}
		for (std::vector<RequestTypes>::const_iterator iter = mostSpecificMatch->allowedRequestTypes.begin(); iter != mostSpecificMatch->allowedRequestTypes.end(); ++iter) {
			if (method == requestTypeToString(*iter)) {
				return true;
			}
		}
	}
	return false;
}

std::string HTTPResponse::isRedirection(const std::string& uri, const ServerConfig& serverConfig) {
	std::string redirection;
	const LocationConfig* mostSpecificMatch = NULL;
	for (std::vector<LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); ++it) {
		if (uri.find(it->locationPath) == 0) {
			if (!mostSpecificMatch || it->locationPath.length() > mostSpecificMatch->locationPath.length()) {
				mostSpecificMatch = &(*it);
			}
		}
	}
	if (mostSpecificMatch) {
		redirection = mostSpecificMatch->redirection;
	}
	return redirection;
}

/* -------------------------------------------------------------------------- */
/*                             Creating Responses                             */
/* -------------------------------------------------------------------------- */

void HTTPResponse::assignResponse(int statusCode, const std::string& body, std::string contentType) {
	setHeader("Content-Type", contentType);
	setHeader("Content-Length", ::toString(body.size()));
	setBody(body);
	setStatusCode(statusCode);
}

void HTTPResponse::assignGenericResponse(int statusCode, const std::string& message) {
	std::ostringstream stream;
	std::string code = ::toString(statusCode);
	std::string codeMessage = statusCodes.find(statusCode) != statusCodes.end() ? statusCodes.at(statusCode) : "Unknown Code In Map";
	stream << "<!DOCTYPE html>"
			<< "<html lang=\"en\">"
			<< "<head>"
			<< "<meta charset=\"UTF-8\">"
			<< "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
			<< "<title>Webserv - " << code << "</title>"
			<< "<link rel=\"stylesheet\" href=\"\\styles.css\">"
			<< "<link rel=\"icon\" type=\"image/x-icon\" href=\"favicon.ico\">"
			<< "</head>"
			<< "<body class=\"background\">"
			<< "<div class=\"error\">" << code << " - " << codeMessage << "</div>"
			<< "<hr>"
			<< "<div class=\"info\">" << message << "</div>"
			<< "<button onclick=\"window.history.back()\" class=\"back-button\">Back</button>"
			<< "</body>"
			<< "</html>";
	assignResponse(statusCode, stream.str(), "text/html");
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
