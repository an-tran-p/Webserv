#include "../include/ConfigParser.hpp"
static void parseLocation(std::ifstream &file, std::vector<LocationConfig> &locations,
                          const std::string &serverRoot, const std::string &serverIndex);
static void parseServer(std::ifstream &file, std::vector<ServerConfig> &servers);

static std::string stripSemicolon(const std::string &s)
{
    if (!s.empty() && s.back() == ';')
        return s.substr(0, s.size() - 1);
    return s;
}

std::string nextToken(std::ifstream &file)
{
    std::string word;
    file >> word; // 自动跳过所有空白（空格、Tab、换行）, 然后读取字符直到遇到下一个空白为止
    return word;
}

void expect(std::ifstream &file, std::string expected)
{
    if (nextToken(file) != expected)
        throw std::runtime_error("expected '" + expected + "'");
}

void parseLocation(std::ifstream &file, std::vector<LocationConfig> &locations, const std::string &serverRoot, const std::string &serverIndex)
{
    // Parse the start of the location block.
    LocationConfig location;
    location.setRoot(serverRoot);
    location.setIndex(serverIndex);

    location.setLocationPath(nextToken(file));
    expect(file, "{");

    // Parse the contents of the location block.
    while (file)
    {
        std::string key = nextToken(file);
        if (key == "}")
        {
            break;
        }
        else if (key == "limit_except")
        {
            std::vector<std::string> methods;
            std::string word;
            while (file >> word && word != "{")
                methods.push_back(word);
            location.setAllowedMethods(methods);
            // 跳过 { deny all; } 里面的内容
            std::string skip;
            int depth = 0;
            while (file >> skip)
            {
                if (skip == "{")
                    depth++;
                if (skip == "}")
                {
                    depth--;
                    if (depth < 0)
                        break;
                }
            }
        }
        else if (key == "upload_dir")
        {
            location.setUploadDir(stripSemicolon(nextToken(file)));
        }
        else if (key == "cgi_path")
        {
            location.setCgiPath(stripSemicolon(nextToken(file)));
        }
        else if (key == "cgi_extension")
        {
            location.setCgiExtension(stripSemicolon(nextToken(file)));
        }
        else if (key == "autoindex")
        {
            std::string value = stripSemicolon(nextToken(file));
            location.setAutoindex(value == "on");
        }
        else if (key == "return")
        {
            std::string code = nextToken(file);                // "302"
            std::string url = stripSemicolon(nextToken(file)); // "https://google.com"
            location.setRedirect({std::stoi(code), url});
        }
        else
        {
            throw std::runtime_error("Unknown key '" + key + "'");
        }
    }

    // Check that the location is not already taken.
    for (LocationConfig &other : locations)                        // ✅
        if (other.getLocationPath() == location.getLocationPath()) // ✅
            throw std::runtime_error("Duplicate location '" + location.getLocationPath() + "'");

    // Add the location to the list of locations.
    locations.push_back(location);
}

void parseServer(std::ifstream &file, std::vector<ServerConfig> &servers) // parse one block
{
    std::vector<LocationConfig> locations;
    // Check for the start of a server block.
    expect(file, "server");
    expect(file, "{");

    // Parse the contents of the server block.
    ServerConfig server = {};
    while (file)
    {
        std::string key = nextToken(file);
        if (key == "}")
        {
            break;
        }
        else if (key == "location")
        {
            parseLocation(file, locations, server.getRoot(), server.getIndex());
        }
        else if (key == "listen")
        {
            std::string value;
            file >> value;
            value = stripSemicolon(value);
            //  "127.0.0.1:8080"
            size_t colon = value.find(':');
            if (colon != std::string::npos) // if found ":"
            {
                server.setHost(value.substr(0, colon));
                server.setPort(std::stoi(value.substr(colon + 1)));
            }
            else
                server.setPort(std::stoi(value));
        }
        else if (key == "server_name")
        {
            std::vector<std::string> names;
            std::string word;
            while (file >> word)
            {
                bool last = (word.back() == ';');
                names.push_back(stripSemicolon(word));
                if (last)
                    break;
            }
            server.setServerNames(names);
        }
        else if (key == "root")
        {
            server.setRoot(stripSemicolon(nextToken(file)));
        }
        else if (key == "index")
        {
            server.setIndex(stripSemicolon(nextToken(file)));
        }
        else if (key == "client_max_body_size")
        {
            std::string value;
            value = stripSemicolon(nextToken(file));
            // 解析 1M / 1G / 1K
            size_t multiplier = 1;
            char unit = value.back();
            if (unit == 'M')
            {
                value.pop_back();
                multiplier = 1024 * 1024;
            }
            else if (unit == 'G')
            {
                value.pop_back();
                multiplier = 1024 * 1024 * 1024;
            }
            else if (unit == 'K')
            {
                value.pop_back();
                multiplier = 1024;
            }
            server.setClientMaxBodySize(std::stoul(value) * multiplier);
        }

        else
        {
            throw std::runtime_error("Unknown key '" + key + "'");
        }
    }

    // Check that the server received a name.
    if (server.getHost().empty())
        throw std::runtime_error("Missing server name");

    // Check that the name is not already taken.
    for (ServerConfig &other : servers)
        if (other.getHost() == server.getHost() && other.getPort() == server.getPort())
            throw std::runtime_error("Duplicate server '" + server.getHost() + "'");

    // Add the server to the list.
    server.setLocations(locations);
    servers.push_back(server);
}

ConfigParser::ConfigParser()
{
}

int ConfigParser::setFilepath(int argc, char **argv)
{
    if (argc > 2)
    {
        std::cerr << "Invalid usage: too many arguments\n";
        return failure;
    }
    if (argc == 2)
    {
        _filepath = argv[1];
        std::cout << "Using config file " << argv[1] << "...\n";
    }
    else
    {
        _filepath = "config/default.conf";
        std::cout << "Using default config file...\n";
    }
    return success;
}

std::optional<std::vector<ServerConfig>> ConfigParser::parseConfig()
{
    std::ifstream file(_filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open config file: " << _filepath << "\n";
        return std::nullopt;
    }

    std::vector<ServerConfig> servers;
    std::string token;

    while (!(file >> std::ws).eof())
    {
        parseServer(file, servers);
    }
    return servers;
}