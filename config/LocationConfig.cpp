#include "../include/LocationConfig.hpp"


LocationConfig::LocationConfig() : _autoindex(false)
{
}
// getter

const std::string& LocationConfig::getLocationPath() const
{
    return _locationPath;
}

const std::vector<std::string>& LocationConfig::getAllowedMethods() const
{
    return _allowedMethods;
}
const std::string& LocationConfig::getIndex() const
{
    return _index;
}
const std::string& LocationConfig::getRoot() const
{
    return _root;
}
const std::pair<int, std::string>& LocationConfig::getRedirect() const
{
    return _redirect;
}
bool LocationConfig::getAutoindex() const
{
    return _autoindex;
}
const std::string& LocationConfig::getUploadDir() const
{
    return _uploadDir;
}
const std::string& LocationConfig::getCgiExtension() const
{
    return _cgiExtension;
}
const std::string& LocationConfig::getCgiPath() const
{
    return _cgiPath;
}


// setter

void LocationConfig::setLocationPath(const std::string& path)
{
    _locationPath = path;
}

void LocationConfig::setAllowedMethods(const std::vector<std::string>& allowedMethods)
{
    _allowedMethods = allowedMethods;
}
void LocationConfig::setIndex(const std::string& index)
{
    _index = index;
}
void LocationConfig::setRoot(const std::string& root)
{
    _root = root;
}
void LocationConfig::setRedirect(const std::pair<int, std::string>& redirect)
{
    _redirect = redirect;
}
void LocationConfig::setAutoindex(const bool autoindex)
{
    _autoindex = autoindex;
}
void LocationConfig::setUploadDir(const std::string& uploadDir)
{
    _uploadDir = uploadDir;
}
void LocationConfig::setCgiExtension(const std::string& cgiExtension)
{
    _cgiExtension = cgiExtension;
}

void LocationConfig::setCgiPath(const std::string& cgiPath)
{
    _cgiPath = cgiPath;
}
