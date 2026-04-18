#include <exception>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

struct Location
{
    std::string path;
    std::string root;
};

struct Server
{
    std::string name;
    std::string port;
    std::vector<Location> locations;
};

std::string route(std::vector<Server>& servers, std::string host, std::string path);
void parseServer(std::ifstream& file, std::vector<Server>& servers);