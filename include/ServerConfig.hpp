#pragma once
#include "LocationConfig.hpp"
#include "webserv.hpp"

/*
server {
    listen 127.0.0.1:8080;       → _host = "127.0.0.1", _port = 8080
    server_name localhost;        → _serverName = "localhost"
    client_max_body_size 1M;      → _clientMaxBodySize = 1048576
    root ./www;                   → _root = "./www"
    index index.html;             → _index = "index.html"

    location / { }               ┐
    location /upload { }         ├→ _locations (vector of LocationConfig)
    location /cgi-bin/ { }       ┘
}
*/

class ServerConfig
{
    private:
        std::string _host;
        int _port;
        std::string _serverName;
        size_t _clientMaxBodySize;
        std::string _root;
        std::string _index;
        std::vector<LocationConfig> _locations;

    public:
        ServerConfig();

        // Getter
        const std::string& getHost() const;
        int getPort() const;
        const std::string& getServerName() const;
        size_t getClientMaxBodySize() const;
        const std::string& getRoot() const;
        const std::string& getIndex() const;
        const std::vector<LocationConfig>& getLocations() const;

        // Setter
        void setHost(const std::string& host);
        void setPort(int port);
        void setServerName(const std::string& serverName);
        void setClientMaxBodySize(size_t clientMaxBodySize);
        void setRoot(const std::string& root);
        void setIndex(const std::string& index);
        void setLocations(const std::vector<LocationConfig>& locations);

};