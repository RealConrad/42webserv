#include "ConfigManager.hpp"
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "Error: Invalid number of parameters" << std::endl;
        std::cerr << "Usage: ./webserv OR ./webserv <config-file>" << std::endl;
        return 1;
    }

    try {
        ConfigManager configManager;
        if (argc == 1)
            configManager.parseConfigFile("config/default.config");
        else
            configManager.parseConfigFile(argv[1]);
        printHTTPConfig(configManager.getConfig());
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "Exiting program" << std::endl;
    return 0;
}
