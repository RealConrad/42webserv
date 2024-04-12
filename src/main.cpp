#include "ConfigManager.hpp"
#include "SocketManager.hpp"
#include "Logger.hpp"
#include <stdexcept>
#include <signal.h>
#include "Utils.hpp"

int main(int argc, char** argv) {

	Logger::initialize(true);
	INFO("Starting program");
    if (argc > 2) {
        ERROR("Invalid number of parameters");
        INFO("Usage: ./webserv OR ./webserv <config-file>");
        return 1;
    }
    try {
		INFO("Parsing Config");
        ConfigManager configManager;
        if (argc == 1)
            configManager.parseConfigFile("config/default.config");
        else
            configManager.parseConfigFile(argv[1]);
		SocketManager socketManager(configManager.getConfig());
		socketManager.setupServerSockets();
		socketManager.run();
    } catch (const std::runtime_error& e) {
    	ERROR(e.what());
    }

	INFO("Exiting program");
    return 0;
}
