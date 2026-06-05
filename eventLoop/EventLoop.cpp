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

static std::string readFile(const std::string &filePath)
{
    // std::cout << "readFile: " << filePath << "\n";
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "readFile: cannot open file\n";
        return "";
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    if (size < 0)
    {
        std::cout << "readFile: invalid size\n";
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
    // 1. 创建管道：pipefd[0] 是读端，pipefd[1] 是写端
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
        close(pipefd[0]);               // 关闭读端，子进程不需要读
        dup2(pipefd[1], STDOUT_FILENO); // print() → 写进管道
        close(pipefd[1]);               // dup2 完了可以关掉

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

        std::string output;
        char buf[4096];
        ssize_t n;
        while ((n = read(pipefd[0], buf, sizeof(buf))) > 0)
            output.append(buf, n);
        close(pipefd[0]);

        waitpid(pid, nullptr, 0);
        return output;
    }
}

static Response make404Response(const std::string &root)
{
    std::string content = readFile(root + "/errors/404.html");
    Response resp;
    resp.setStatus(404);
    resp.setContentType("text/html");
    if (content.empty())
        resp.setBody("404 Not Found");
    else
        resp.setBody(content);
    return resp;
}

static std::string generateAutoIndex(const std::string& dirPath, const std::string& reqPath)
{
    DIR* dir = opendir(dirPath.c_str());
    if (!dir)
        return "";

    std::string html = "<html><body><h1>Index of " + reqPath + "</h1><ul>";

    struct dirent* entry;
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
    std::cout << "DELETE filePath: " << filePath << "\n";

    // 3. detele
    if (unlink(filePath.c_str()) == 0)
        return 204; // Success → No Content
    else
        return 404; // Fail → NO file
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
    state.clients.erase(state.clients.begin() + (i - 1));
    state.requests.erase(state.requests.begin() + (i - 1));
    state.poll_fds.erase(state.poll_fds.begin() + i);
    state.connectTime.erase(state.connectTime.begin() + (i - 1));
    --i;
}

void addClient(ServerSocket &server, ServerState &state)
{
    Socket clientSock = server.accept_client();
    state.clients.push_back(Connection(std::move(clientSock)));
    state.requests.push_back(Request{});
    state.connectTime.push_back(std::chrono::steady_clock::now());

    pollfd client_pfd;
    client_pfd.fd = state.clients.back().fd();
    client_pfd.events = POLLIN;
    client_pfd.revents = 0;
    state.poll_fds.push_back(client_pfd);

    std::cout << "New client connected! fd=" << state.clients.back().fd() << "\n";
}

bool checkTimeout(size_t &i, ServerState &state)
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - state.connectTime[i - 1]).count();

    if (elapsed > 10 && !state.requests[i - 1].isDone())
    {
        std::cout << "Client timeout! fd=" << state.clients[i - 1].fd() << "\n";
        Response resp = Response::makeError(408);
        state.clients[i - 1].getWriteBuffer() += resp.build(false);
        state.clients[i - 1].setCloseAfterWrite(true);
        state.poll_fds[i].events |= POLLOUT;
        state.connectTime[i - 1] = now;
        return true;
    }
    return false;
}

bool handleRead(size_t &i, ServerState &state)
{
    pollfd &pfd = state.poll_fds[i];
    Connection &client = state.clients[i - 1];
    Request &req = state.requests[i - 1];

    if (!(pfd.revents & POLLIN))
        return false;

    if (!client.read_from_socket())
    {
        std::cout << "Client Disconnected! fd=" << client.fd() << "\n";
        removeClient(i, state);
        return true;
    }

    state.connectTime[i - 1] = std::chrono::steady_clock::now();

    if (tryParseRequest(client, req))
    {
        std::cout << "\n--- Request received from fd=" << client.fd() << " ---\n"
                  << "Method: " << req.method << "\n"
                  << "Path:   " << req.path << "\n"
                  << "Host:   " << req.headers["Host"] << "\n"
                  << "Body:   " << req.body << "\n";

        if (req.isError())
        {
            Response resp = Response::makeError(req.getStatusCode());
            client.getWriteBuffer() += resp.build(false);
            client.setCloseAfterWrite(true);
        }
        else
        {

            bool found = false;
            const LocationConfig *bestMatch = nullptr;
            for (const LocationConfig &loc : state.config.getLocations())
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
                if (req.body.size() > state.config.getClientMaxBodySize())
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

                        if (output.empty())
                        {
                            Response resp = make404Response(state.config.getRoot());
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
                        else
                        {
                            resp.setStatus(200);
                            resp.setContentType("text/html");
                            resp.setBody(content);
                            client.getWriteBuffer() += resp.build(req.keepAlive);
                            client.setCloseAfterWrite(!req.keepAlive);
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
                Response resp = make404Response(state.config.getRoot());
                client.getWriteBuffer() += resp.build(false);
                client.setCloseAfterWrite(true);
            }

            // client.getWriteBuffer() += resp.build(req.keepAlive);
            // client.setCloseAfterWrite(!req.keepAlive);
        }
        req = Request{};
        pfd.events |= POLLOUT;
        state.connectTime[i - 1] = std::chrono::steady_clock::now();
    }
    return false;
}

bool handleWrite(size_t &i, ServerState &state)
{
    pollfd &pfd = state.poll_fds[i];
    Connection &client = state.clients[i - 1];

    if (!(pfd.revents & POLLOUT))
        return false;

    client.write_to_socket();
    state.connectTime[i - 1] = std::chrono::steady_clock::now();

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