#pragma once

#include "webserv.hpp"


class LocationConfig
{
    private:
        std::string _locationPath;              // 路径名，如 "/" 或 "/upload"
        std::vector<std::string> _allowedMethods; // ["GET", "POST"]
        std::string _index;                     // 默认文件
        std::string _root;                      // 根目录
        std::pair<int, std::string> _redirect;  // 重定向，如 {302, "https://..."}
        bool _autoindex;                        // 是否显示目录列表
        std::string _uploadDir;                 // 上传目录
        std::string _cgiExtension;              // CGI文件扩展名
        std::string _cgiPath;                   // CGI程序路径

    public:
        LocationConfig();

        // Getter
        const std::string& getLocationPath() const;
        const std::vector<std::string>& getAllowedMethods() const;
        const std::string& getIndex() const;
        const std::string& getRoot() const;
        const std::pair<int, std::string>& getRedirect() const;
        bool getAutoindex() const;
        const std::string& getUploadDir() const;
        const std::string& getCgiExtension() const;
        const std::string& getCgiPath() const;


        // Setter
        void setLocationPath(const std::string& path);
        void setAllowedMethods(const std::vector<std::string>& allowedMethods);
        void setIndex(const std::string& index);
        void setRoot(const std::string& root);
        void setRedirect(const std::pair<int, std::string>& redirect);
        void setAutoindex(const bool autoindex);
        void setUploadDir(const std::string& uploadDir);
        void setCgiExtension(const std::string& cgiExtension);
        void setCgiPath(const std::string& cgiPath);

};