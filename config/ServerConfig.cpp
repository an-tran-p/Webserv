#include "../include/ServerConfig.hpp"

ServerConfig::ServerConfig() : _port(0), _clientMaxBodySize(0)
{
}

const std::string& ServerConfig::getHost() const
{
    return _host;
}

int ServerConfig::getPort() const
{
    return _port;
}

const std::vector<std::string>& ServerConfig::getServerNames() const
{
    return _serverNames;
}

size_t ServerConfig::getClientMaxBodySize() const
{
    return _clientMaxBodySize;
}

const std::string& ServerConfig::getRoot() const
{
    return _root;
}
const std::string& ServerConfig::getIndex() const
{
    return _index;
}
const std::vector<LocationConfig>& ServerConfig::getLocations() const
{
    return _locations;
}
const std::map<int, std::string>& ServerConfig::getErrorPages() const
{
    return _errorPages;
}

// Setter
void ServerConfig::setHost(const std::string& host)
{
    _host = host;
}
void ServerConfig::setPort(int port)
{
    _port = port;
}
void ServerConfig::setServerNames(const std::vector<std::string>& serverNames)
{
    _serverNames = serverNames;
}
void ServerConfig::setClientMaxBodySize(size_t clientMaxBodySize)
{
    _clientMaxBodySize = clientMaxBodySize;
}
void ServerConfig::setRoot(const std::string& root)
{
    _root = root;
}
void ServerConfig::setIndex(const std::string& index)
{
    _index = index;
}
void ServerConfig::setLocations(const std::vector<LocationConfig>& locations)
{
    _locations = locations;
}

void ServerConfig::setErrorPages(const std::map<int, std::string>& errorPages)
{
    _errorPages = errorPages;
}