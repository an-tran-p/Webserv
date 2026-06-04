#pragma once
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

#include "webserv.hpp"

class ConfigParser
{
    private:
        std::string _filepath;

    public:
        ConfigParser();

        // 设置配置文件路径（检查 argc/argv）
        int setFilepath(int argc, char **argv);


        // 解析配置文件，返回所有服务器配置
        std::optional<std::vector<ServerConfig>> parseConfig();
};