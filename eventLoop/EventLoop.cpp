/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/23 14:41:15 by atran             #+#    #+#             */
/*   Updated: 2026/05/23 19:05:46 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <cerrno>

static std::string readFile(const std::string &filePath)
{
    // std::cout << "readFile: " << filePath << "\n";
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        // std::cout << "readFile: cannot open file\n";
        return "";
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    if (size < 0)
    {
        // std::cout << "readFile: invalid size\n";
        return "";
    }
    file.seekg(0, std::ios::beg);

    std::string content(size, '\0');
    file.read(&content[0], size);
    return content;
}

static std::string buildFilePath(const LocationConfig &loc, const std::string &reqPath)
{
    std::string filePath = loc.getRoot() + reqPath;
    // std::cout << "buildFilePath: " << filePath << "\n";
    if (filePath.back() == '/')
        filePath += loc.getIndex();
    // std::cout << "buildFilePath final: " << filePath << "\n";
    return filePath;
}

static std::string executeCGI(const std::string &scriptPath, const std::string &interpreter)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
        return "";

    // 2. 创建子进程
    pid_t pid = fork();
    if (pid == -1)
        return "";

    if (pid == 0)
    {
        // ===== kid =====
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        // run python3 ./www/cgi-bin/hello.py
        char *args[] = {
            (char *)interpreter.c_str(), // "python3"
            (char *)scriptPath.c_str(),  // "./www/cgi-bin/hello.py"
            nullptr};
        execve(interpreter.c_str(), args, nullptr);
        _exit(1);
    }
    else
    {
        // ===== dad =====
        close(pipefd[1]);

        // set time
        time_t start = time(nullptr);
        std::string output;
        char buf[4096];
        ssize_t n;

        // set pipe nonblock
        fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

        while (true)
        {
            n = read(pipefd[0], buf, sizeof(buf));
            if (n > 0)
                output.append(buf, n);
            else if (n == 0)
                break; // EOF，child end
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // check time
                if (difftime(time(nullptr), start) > 5)
                {
                    // kill child
                    kill(pid, SIGKILL);
                    waitpid(pid, nullptr, 0);
                    close(pipefd[0]);
                    return "TIMEOUT"; // return 0/  504
                }
                usleep(10000);
            }
            else
                break;
        }
        close(pipefd[0]);
        waitpid(pid, nullptr, 0);
        return output;
    }
}

static Response makeErrorResponse(int code, const ServerConfig *config)
{
    auto pages = config->getErrorPages();
    std::string content;

    // find from config first: error_page 404 /errors/404.html;
    if (pages.count(code))
        content = readFile(config->getRoot() + pages.at(code));

    // find default route
    if (content.empty())
        // content = readFile("./www/errors/404.html");
        content = readFile(config->getRoot() + "/errors/" + std::to_string(code) + ".html");

    Response resp;
    resp.setStatus(code);
    resp.setContentType("text/html");
    if (content.empty())
        resp.setBody(std::to_string(code) + " Error");
    else
        resp.setBody(content);
    return resp;
}

static std::string generateAutoIndex(const std::string &dirPath, const std::string &reqPath)
{
    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
        return "";

    std::string html = "<html><body><h1>Index of " + reqPath + "</h1><ul>";

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;
        if (name == ".")
            continue;
        html += "<li><a href=\"" + name + "\">" + name + "</a></li>";
    }
    closedir(dir);
    html += "</ul></body></html>";
    return html;
}

static int handlePostUpload(const Request &req, const LocationConfig &loc)
{
    /*
    --------------------------26d42f3bcd2750b9\r\n
    Content-Disposition: form-data; name="file"; filename="test.txt"\r\n
    Content-Type: text/plain\r\n
    \r\n
    hello world\r\n
    --------------------------26d42f3bcd2750b9--
    */

    // find boundary
    std::string contentType = req.headers.at("content-type");
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        return 400;
    std::string boundary = "--" + contentType.substr(pos + 9);

    // find filename="..."
    size_t fnPos = req.body.find("filename=\"");
    if (fnPos == std::string::npos)
        return 400;
    fnPos += 10;                               // skip filename="（10 char）
    size_t fnEnd = req.body.find("\"", fnPos); // end "\"
    std::string filename = req.body.substr(fnPos, fnEnd - fnPos);
    // filename = "test.txt"

    // find content
    size_t headerEnd = req.body.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return 400;
    size_t contentStart = headerEnd + 4; // skip \r\n\r\n

    // find end boundary
    std::string endBoundary = "\r\n" + boundary + "--";
    size_t contentEnd = req.body.find(endBoundary, contentStart);
    if (contentEnd == std::string::npos)
        return 400;

    // get file content
    std::string fileContent = req.body.substr(contentStart, contentEnd - contentStart);

    // save
    std::string savePath = loc.getUploadDir() + filename;
    std::ofstream outFile(savePath, std::ios::binary);
    if (!outFile.is_open())
        return 500;
    outFile.write(fileContent.c_str(), fileContent.size());
    outFile.close();
    return 200;
}

static int handleDelete(const Request &req, const LocationConfig &loc)
{
    // 1. get file name
    size_t lastSlash = req.path.find_last_of('/');
    std::string filename = req.path.substr(lastSlash + 1);

    // 2. get file path
    std::string filePath = loc.getUploadDir() + filename;
    // std::cout << "DELETE filePath: " << filePath << "\n";

    // 3. detele
    if (unlink(filePath.c_str()) == 0)
        return 204; // Success → No Content
    else
        return 404; // Fail → NO file
}

static std::string getMimeType(const std::string &filePath)
{
    static const std::map<std::string, std::string> mimeMap = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".txt", "text/plain"},
        {".ico", "image/x-icon"},
    };

    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos)
        return "application/octet-stream";

    std::string ext = filePath.substr(dotPos);
    auto it = mimeMap.find(ext);
    if (it != mimeMap.end())
        return it->second;
    return "application/octet-stream";
}

bool tryParseRequest(Connection &client, Request &req)
{
    std::string &rbuf = client.getReadBuffer();
    if (rbuf.empty())
        return false;
    req.parse(rbuf);
    rbuf.clear();
    if (req.isError())
        return true;
    return req.isDone();
}

void removeClient(size_t &i, ServerState &state)
{
    size_t ci = i - state.numServers; // client index
    state.clients.erase(state.clients.begin() + ci);
    state.requests.erase(state.requests.begin() + ci);
    state.poll_fds.erase(state.poll_fds.begin() + i);
    state.connectTime.erase(state.connectTime.begin() + ci);
    state.clientConfigs.erase(state.clientConfigs.begin() + ci);
    state.clientListenFds.erase(state.clientListenFds.begin() + ci);
    --i;
}

void addClient(ServerSocket &server, ServerState &state, std::vector<ServerConfig*>& configs)
{
    Socket clientSock = server.accept_client();
    state.clients.push_back(Connection(std::move(clientSock)));
    state.requests.push_back(Request{});
    state.connectTime.push_back(std::chrono::steady_clock::now());
    state.clientConfigs.push_back(configs[0]);       // default: first config
    state.clientListenFds.push_back(server.fd());    // record which listen fd this client came from

    pollfd client_pfd;
    client_pfd.fd = state.clients.back().fd();
    client_pfd.events = POLLIN;
    client_pfd.revents = 0;
    state.poll_fds.push_back(client_pfd);

    std::cout << "New client connected! fd=" << state.clients.back().fd() << "\n";
}

bool checkTimeout(size_t &i, ServerState &state)
{
    size_t ci = i - state.numServers;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - state.connectTime[ci]).count();

    if (elapsed > 10 && !state.requests[ci].isDone())
    {
        std::cout << "Client timeout! fd=" << state.clients[ci].fd() << "\n";
        Response resp = Response::makeError(408);
        state.clients[ci].getWriteBuffer() += resp.build(false);
        state.clients[ci].setCloseAfterWrite(true);
        state.poll_fds[i].events |= POLLOUT;
        state.connectTime[ci] = now;
        return true;
    }
    return false;
}

bool handleRead(size_t &i, ServerState &state)
{
    size_t ci = i - state.numServers;
    pollfd &pfd = state.poll_fds[i];
    Connection &client = state.clients[ci];
    Request &req = state.requests[ci];

    if (!(pfd.revents & POLLIN))
        return false;

    if (!client.read_from_socket())
    {
        std::cout << "Client Disconnected! fd=" << client.fd() << "\n";
        removeClient(i, state);
        return true;
    }

    state.connectTime[ci] = std::chrono::steady_clock::now();

    if (tryParseRequest(client, req))
    {
        std::cout << "\n--- Request received from fd=" << client.fd() << " ---\n"
                  << "Method: " << req.method << "\n"
                  << "Path:   " << req.path << "\n";

        if (req.isError())
        {
            Response resp = Response::makeError(req.getStatusCode());
            client.getWriteBuffer() += resp.build(false);
            client.setCloseAfterWrite(true);
        }
        else
        {

            ServerConfig *currentConfig = state.clientConfigs[ci];

            // select server config based on Host header
            // extract Host header and strip port for virtual hosting
            std::string host = req.headers.count("host") ? req.headers.at("host") : "";
            size_t colonPos = host.find(':');
            if (colonPos != std::string::npos)
                host = host.substr(0, colonPos);
            int listenFd = state.clientListenFds[ci];
            if (state.fdToConfigs.count(listenFd))
            {
                for (ServerConfig* cfg : state.fdToConfigs[listenFd])
                {
                    for (const std::string& name : cfg->getServerNames())
                    {
                        if (name == host)
                        {
                            currentConfig = cfg;
                            break;
                        }
                    }
                }
            }

            bool found = false;
            const LocationConfig *bestMatch = nullptr;
            for (const LocationConfig &loc : currentConfig->getLocations())
            {
                if (req.path.rfind(loc.getLocationPath(), 0) == 0) // match from begining
                {
                    if (bestMatch == nullptr ||
                        loc.getLocationPath().size() > bestMatch->getLocationPath().size())
                        bestMatch = &loc; // choose longest match one
                }
            }

            if (bestMatch != nullptr)
            {
                found = true;
                const LocationConfig &loc = *bestMatch;
                // check CGI
                bool isCGI = (req.path.find(loc.getCgiExtension()) != std::string::npos) && !loc.getCgiPath().empty();
                auto methods = loc.getAllowedMethods();

                // check body size
                if (req.body.size() > currentConfig->getClientMaxBodySize())
                {
                    Response resp = Response::makeError(413);
                    client.getWriteBuffer() += resp.build(false);
                    client.setCloseAfterWrite(true);
                }

                // find method.
                auto it = std::find(methods.begin(), methods.end(), req.method);

                // if find method
                if (it != methods.end())
                {
                    // if redirect
                    if (loc.getRedirect().first != 0)
                    {
                        Response resp;
                        resp.setStatus(loc.getRedirect().first);    // 302
                        resp.setLocation(loc.getRedirect().second); // https://google.com
                        client.getWriteBuffer() += resp.build(false);
                        client.setCloseAfterWrite(true);
                    }
                    // if CGI
                    else if (isCGI)
                    {
                        // run CGI
                        std::string scriptPath = loc.getRoot() + req.path;
                        std::string output = executeCGI(scriptPath, loc.getCgiPath());

                        if (output == "TIMEOUT")
                        {
                            Response resp = Response::makeError(504);
                            client.getWriteBuffer() += resp.build(false);
                            client.setCloseAfterWrite(true);
                        }
                        if (output.empty())
                        {
                            Response resp = makeErrorResponse(404, currentConfig);
                            client.getWriteBuffer() += resp.build(false);
                            client.setCloseAfterWrite(true);
                        }
                        else
                        {
                            // find empty line
                            size_t sep = output.find("\n\n");
                            std::string body;
                            if (sep != std::string::npos)
                                body = output.substr(sep + 2); // content after 2 new line
                            else
                                body = output; // can not find new line, body

                            Response resp;
                            resp.setStatus(200);
                            resp.setContentType("text/html");
                            resp.setBody(body);
                            client.getWriteBuffer() += resp.build(req.keepAlive);
                            client.setCloseAfterWrite(!req.keepAlive);
                        }
                    }
                    else if (req.method == "POST")
                    {
                        int status = handlePostUpload(req, loc);
                        Response resp;
                        if (status == 200)
                        {
                            resp.setStatus(200);
                            resp.setContentType("text/html");
                            resp.setBody("<h1>File uploaded successfully!</h1>");
                        }
                        else
                            resp = Response::makeError(status);
                        client.getWriteBuffer() += resp.build(req.keepAlive);
                        client.setCloseAfterWrite(!req.keepAlive);
                    }
                    else if (req.method == "DELETE")
                    {
                        int status = handleDelete(req, loc);
                        Response resp;
                        resp.setStatus(status);
                        resp.setBody("");
                        client.getWriteBuffer() += resp.build(false);
                        client.setCloseAfterWrite(true);
                    }
                    else
                    {
                        std::string filePath = buildFilePath(loc, req.path);
                        std::string content = readFile(filePath);
                        Response resp;

                        // autoindex
                        if (content.empty() && loc.getAutoindex() && req.path.back() == '/')
                        {
                            std::string dirPath = loc.getRoot() + req.path;
                            content = generateAutoIndex(dirPath, req.path);
                            if (!content.empty())
                            {
                                Response resp;
                                resp.setStatus(200);
                                resp.setContentType("text/html");
                                resp.setBody(content);
                                client.getWriteBuffer() += resp.build(req.keepAlive);
                                client.setCloseAfterWrite(!req.keepAlive);
                            }
                        }
                        else // static
                        {
                            if (content.empty())
                            {
                                Response errResp = makeErrorResponse(404, currentConfig);
                                client.getWriteBuffer() += errResp.build(false);
                                client.setCloseAfterWrite(true);
                            }
                            else
                            {
                                resp.setStatus(200);
                                resp.setContentType(getMimeType(filePath));
                                resp.setBody(content);
                                client.getWriteBuffer() += resp.build(req.keepAlive);
                                client.setCloseAfterWrite(!req.keepAlive);
                            }
                        }
                    }
                }
                else
                {
                    Response resp = Response::makeError(405);
                    client.getWriteBuffer() += resp.build(false);
                    client.setCloseAfterWrite(true);
                }
            }
            if (!found)
            {
                // can not found location → 404
                Response resp = makeErrorResponse(404, currentConfig);
                client.getWriteBuffer() += resp.build(false);
                client.setCloseAfterWrite(true);
            }

            // client.getWriteBuffer() += resp.build(req.keepAlive);
            // client.setCloseAfterWrite(!req.keepAlive);
        }
        req = Request{};
        pfd.events |= POLLOUT;
        state.connectTime[ci] = std::chrono::steady_clock::now();
    }
    return false;
}

bool handleWrite(size_t &i, ServerState &state)
{
    size_t ci = i - state.numServers;
    pollfd &pfd = state.poll_fds[i];
    Connection &client = state.clients[ci];

    if (!(pfd.revents & POLLOUT))
        return false;

    client.write_to_socket();
    state.connectTime[ci] = std::chrono::steady_clock::now();

    if (client.getWriteBuffer().empty())
    {
        pfd.events &= ~POLLOUT;
        if (client.shouldCloseAfterWrite())
        {
            removeClient(i, state);
            return true;
        }
    }
    return false;
}