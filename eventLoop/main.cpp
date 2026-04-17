/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 10:22:32 by atran             #+#    #+#             */
/*   Updated: 2026/04/14 10:32:59 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "ServerSocket.hpp"
#include "Connection.hpp"
#include "parse.hpp"
#include <iostream>
#include <poll.h>
#include <vector>
#include <algorithm>

bool tryParseRequest(Connection &client, Request &req){
    std::string &rbuf = client.getReadBuffer();
    if (rbuf.empty())
        return false;
    req.parse(rbuf);
    rbuf.clear();

    if (req.isError()){
        return false;
    }

    return req.isDone();
}

int main(){
    const int PORT = 8080;
    ServerSocket server(PORT);
    server.bind_and_listen();

    std::vector<Connection> clients;
    std::vector<Request> requests;
    std::vector<pollfd> poll_fds;

    pollfd server_pfd;
    server_pfd.fd = server.fd();
    server_pfd.events = POLLIN;
    server_pfd.revents = 0;
    poll_fds.push_back(server_pfd);

    std::cout << "Server running on port " << PORT << std::endl;
    while (true){
        //wait for activity on any socket
        int ret = poll(poll_fds.data(), poll_fds.size(), -1);
        if (ret < 0){
            perror("poll");
            break;
        }
        //New connection on the server socket
        if (poll_fds[0].revents & POLLIN){
            Socket clientSock = server.accept_client();
            clients.push_back(Connection(std::move(clientSock)));
            requests.push_back(Request{});

            //Add client to poll_fds
            pollfd client_pfd;
            client_pfd.fd = clients.back().fd();
            client_pfd.events = POLLIN;
            client_pfd.revents = 0;
            poll_fds.push_back(client_pfd);

            std::cout << "New client connected! fd =" << clients.back().fd() << std::endl;
        }

        //Handle existing client
        for (size_t i = 1; i < poll_fds.size(); ++i){
            pollfd& pfd = poll_fds[i];
            Connection& client = clients[i -1];
            Request& req = requests[i - 1];

            //Read
            if (pfd.revents & POLLIN){
                if (!client.read_from_socket()){
                    std::cout << "Client Disconnected! fd=" << client.fd() << std::endl;
                    clients.erase(clients.begin() + (i -1));
                    requests.erase(requests.begin() + (i -1));
                    poll_fds.erase(poll_fds.begin() + i);
                    --i;
                    continue;
                }

                if (tryParseRequest(client, req)){
                    std::cout << "\n--- Request received from fd=" <<client.fd() << " ---\n"
                            << "Method: " << req.method << "\n"
                            << "Path: " << req.path << "\n"
                            << "Host: " << req.headers["Host"] << "\n"
                            << "Body: " << req.body << "\n";

                    // TODO: replace with buildResponse(req) when ready
                    client.getWriteBuffer() +=
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 2\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "OK";
                    if (req.headers["Connection"] == "close")
                        client.setCloseAfterWrite(true);
                    req = Request{};
                    pfd.events |= POLLOUT;
                }
                else{
                    client.getWriteBuffer() +=
                        "HTTP/1.1 " +
                        std::to_string(req.getStatusCode()) +
                        " Error MEDTHOD"
                        "\r\n"
                        "Content-Length: 0\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "OK";
                    client.setCloseAfterWrite(true);
                    req = Request{};
                    pfd.events |= POLLOUT;
                }
            }
            //write
            if (pfd.revents & POLLOUT){
                client.write_to_socket();
                //Write buffer drained: disable POLLOUT so poll() can sleep properly
                if (client.getWriteBuffer().empty()) {
                    pfd.events &= ~POLLOUT;
                    if (client.shouldCloseAfterWrite()){
                        //remove client from pool_fds, clients, requests
                        clients.erase(clients.begin() + (i -1));
                        requests.erase(requests.begin() + (i -1));
                        poll_fds.erase(poll_fds.begin() + 1);
                        --i;
                    }
                }
            }
        }
    }
    return 0;
}
