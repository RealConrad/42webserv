#include "ConfigManager.hpp"
#include "Logger.hpp"
#include <stdexcept>

int main(int argc, char** argv) {
	Logger::initialize(true);
	// Logger::hide(Logger::INFO);
	INFO("Starting program");
    if (argc > 2) {
        std::cerr << "Error: Invalid number of parameters" << std::endl;
        std::cerr << "Usage: ./webserv OR ./webserv <config-file>" << std::endl;
        return 1;
    }
    try {
		INFO("Parsing Config");
        ConfigManager configManager;
        if (argc == 1)
            configManager.parseConfigFile("config/default.config");
        else
            configManager.parseConfigFile(argv[1]);
        // printHTTPConfig(configManager.getConfig());
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

	INFO("Exiting program");
    return 0;
}
