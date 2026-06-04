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

static std::string readFile(const std::string& filePath)
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

static std::string buildFilePath(const LocationConfig& loc, const std::string& reqPath)
{
    std::string filePath = loc.getRoot() + reqPath;
    // std::cout << "buildFilePath: " << filePath << "\n";
    if (filePath.back() == '/')
        filePath += loc.getIndex();
    // std::cout << "buildFilePath final: " << filePath << "\n";
    return filePath;
}


bool tryParseRequest(Connection& client, Request& req) {
    std::string& rbuf = client.getReadBuffer();
    if (rbuf.empty())
        return false;
    req.parse(rbuf);
    rbuf.clear();
    if (req.isError())
        return true;
    return req.isDone();
}

void removeClient(size_t& i, ServerState& state) {
    state.clients.erase(state.clients.begin() + (i - 1));
    state.requests.erase(state.requests.begin() + (i - 1));
    state.poll_fds.erase(state.poll_fds.begin() + i);
    state.connectTime.erase(state.connectTime.begin() + (i - 1));
    --i;
}

void addClient(ServerSocket& server, ServerState& state) {
    Socket clientSock = server.accept_client();
    state.clients.push_back(Connection(std::move(clientSock)));
    state.requests.push_back(Request{});
    state.connectTime.push_back(std::chrono::steady_clock::now());

    pollfd client_pfd;
    client_pfd.fd      = state.clients.back().fd();
    client_pfd.events  = POLLIN;
    client_pfd.revents = 0;
    state.poll_fds.push_back(client_pfd);

    std::cout << "New client connected! fd=" << state.clients.back().fd() << "\n";
}

bool checkTimeout(size_t& i, ServerState& state) {
    auto now     = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>
                       (now - state.connectTime[i - 1]).count();

    if (elapsed > 10 && !state.requests[i - 1].isDone()) {
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

bool handleRead(size_t& i, ServerState& state) {
    pollfd&     pfd    = state.poll_fds[i];
    Connection& client = state.clients[i - 1];
    Request&    req    = state.requests[i - 1];

    if (!(pfd.revents & POLLIN))
        return false;

    if (!client.read_from_socket()) {
        std::cout << "Client Disconnected! fd=" << client.fd() << "\n";
        removeClient(i, state);
        return true;
    }

    state.connectTime[i - 1] = std::chrono::steady_clock::now();

    if (tryParseRequest(client, req)) {
        std::cout << "\n--- Request received from fd=" << client.fd() << " ---\n"
                  << "Method: " << req.method << "\n"
                  << "Path:   " << req.path << "\n"
                  << "Host:   " << req.headers["Host"] << "\n"
                  << "Body:   " << req.body << "\n";

        if (req.isError()) {
            Response resp = Response::makeError(req.getStatusCode());
            client.getWriteBuffer() += resp.build(false);
            client.setCloseAfterWrite(true);
        } else {

            bool found = false;
            for (const LocationConfig& loc : state.config.getLocations())
            {
                // std::cout << "comparing: '" << loc.getLocationPath() << "' == '" << req.path << "'\n";
                if (loc.getLocationPath() == req.path)
                {
                    found = true;
                    auto methods = loc.getAllowedMethods();
                    auto it = std::find(methods.begin(), methods.end(), req.method);
                    if (it != methods.end())
                    {
                        // find method
                        std::string filePath = buildFilePath(loc, req.path);
                        std::string content = readFile(filePath);
                        Response resp;
                        if (content.empty())
                        {
                            resp = Response::makeError(404);
                            client.getWriteBuffer() += resp.build(false);
                            client.setCloseAfterWrite(true);
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
                    else
                    {
                        Response resp = Response::makeError(405);
                        client.getWriteBuffer() += resp.build(false);
                        client.setCloseAfterWrite(true);
                    }
                    break;
                }
                
            }
            if (!found)
            {
                // can not found location → 404
                Response resp = Response::makeError(404);
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

bool handleWrite(size_t& i, ServerState& state) {
    pollfd&     pfd    = state.poll_fds[i];
    Connection& client = state.clients[i - 1];

    if (!(pfd.revents & POLLOUT))
        return false;

    client.write_to_socket();
    state.connectTime[i - 1] = std::chrono::steady_clock::now();

    if (client.getWriteBuffer().empty()) {
        pfd.events &= ~POLLOUT;
        if (client.shouldCloseAfterWrite()) {
            removeClient(i, state);
            return true;
        }
    }
    return false;
}