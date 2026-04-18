#include "config.hpp"

std::string nextToken(std::ifstream& file)
{
    std::string word;
    file >> word; // 自动跳过所有空白（空格、Tab、换行）, 然后读取字符直到遇到下一个空白为止
    return word;
}

void expect(std::ifstream& file, std::string expected)
{
    if (nextToken(file) != expected)
        throw std::runtime_error("expected '" + expected + "'");
}

void parseLocation(std::ifstream& file, std::vector<Location>& locations)
{
    // Parse the start of the location block.
    Location location = {}; // 创建空的 Location 结构体
    location.path = nextToken(file);
    expect(file, "{");

    // Parse the contents of the location block.
    while (file) {
        std::string key = nextToken(file);
        if (key == "}") {
            break;
        } else if (key == "root") {
            location.root = nextToken(file);
        } else {
            throw std::runtime_error("Unknown key '" + key + "'");
        }
    }

    // Check that the location is not already taken.
    for (Location& other: locations)
        if (other.path == location.path)
            throw std::runtime_error("Duplicate location '" + location.path + "'");

    // Add the location to the list of locations.
    locations.push_back(location);
}

void parseServer(std::ifstream& file, std::vector<Server>& servers) // parse one block
{
    // Check for the start of a server block.
    expect(file, "server");
    expect(file, "{");

    // Parse the contents of the server block.
    Server server = {};
    while (file) {
        std::string key = nextToken(file);
        if (key == "}") {
            break;
        } else if (key == "location") {
            parseLocation(file, server.locations);
        } else if (key == "listen") {
            server.port = nextToken(file);
        } else if (key == "server_name") {
            server.name = nextToken(file);
        } else {
            throw std::runtime_error("Unknown key '" + key + "'");
        }
    }

    // Check that the server received a name.
    if (server.name.empty())
        throw std::runtime_error("Missing server name");
    
    // Check that the name is not already taken.
    for (Server& other: servers)
        if (other.name == server.name)
            throw std::runtime_error("Duplicate server '" + server.name + "'");

    // Add the server to the list.
    servers.push_back(server);
}

std::string route(std::vector<Server>& servers, std::string host, std::string path)
{
    Location bestMatch = {};
    for (Server& server: servers) {
        if (host == server.name) {
            for (Location& location: server.locations) {
                if (path.find(location.path) == 0
                 && location.path.size() > bestMatch.path.size())
                    bestMatch = location;
            }
        }
    }
    if (!bestMatch.root.empty()) {
        if (bestMatch.root != "/")
            bestMatch.root += "/";
        return bestMatch.root + path.substr(bestMatch.path.size());
    }
    return "";
}

// int main()
// {
//     try {

//         // Parse the config file.
//         std::vector<Server> servers;
//         std::ifstream file("example.conf");
//         while (!(file >> std::ws).eof())  // skip the spaces and not end of the file
//             parseServer(file, servers);

//         // Write out the configuration.
//         // for (Server& server: servers) {
//         //     std::cout << "server\n";
//         //     std::cout << "    port: " << server.port << "\n";
//         //     std::cout << "    name: " << server.name << "\n";
//         //     for (Location& location: server.locations) {
//         //         std::cout << "    location:\n";
//         //         std::cout << "        path: " << location.path << "\n";
//         //         std::cout << "        root: " << location.root << "\n";
//         //     }
//         // }
//         std::cout << "\n" << route(servers, "example.com", "/def/index.html") << "\n";

//     // Print any errors.
//     } catch (std::exception& error) {
//         std::cout << "[ERROR] " << error.what() << "\n";
//     }
// }