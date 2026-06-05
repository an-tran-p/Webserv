#include "../include/ConfigParser.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    ConfigParser parser;

    if (parser.setFilepath(argc, argv) == failure)
        return 1;

    auto result = parser.parseConfig();
    if (!result.has_value())
    {
        std::cout << "parseConfig returned nullopt\n";
        return 1;
    }

    for (const ServerConfig& server : *result)
    {
        std::cout << "=== Server ===\n";
        std::cout << "host:              " << server.getHost() << "\n";
        std::cout << "port:              " << server.getPort() << "\n";
        std::cout << "server_name:       " << server.getServerName() << "\n";
        std::cout << "root:              " << server.getRoot() << "\n";
        std::cout << "index:             " << server.getIndex() << "\n";
        std::cout << "max_body_size:     " << server.getClientMaxBodySize() << "\n";

        for (const LocationConfig& loc : server.getLocations())
        {
            std::cout << "  --- Location ---\n";
            std::cout << "  path:     " << loc.getLocationPath() << "\n";
            std::cout << "  methods:  ";
            for (const auto& m : loc.getAllowedMethods())
                std::cout << m << " ";
            std::cout << "\n";
            std::cout << "  upload:   " << loc.getUploadDir() << "\n";
            std::cout << "  cgi_path: " << loc.getCgiPath() << "\n";
            std::cout << "  redirect: " << loc.getRedirect().first << " " << loc.getRedirect().second << "\n";
        }
    }

   

    return 0;
}