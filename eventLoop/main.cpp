/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 10:22:32 by atran             #+#    #+#             */
/*   Updated: 2026/04/02 13:51:47 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "ServerSocket.hpp"
#include "Connection.hpp"
#include <iostream>
#include <poll.h>
#include <vector>
#include <algorithm>

int main(){
    const int PORT = 8080;
    ServerSocket server(PORT);
    server.bind_and_listen();

    std::vector<Connection> clients;
    std::vector<pollfd> poll_fds;
    pollfd server_pfd;
    server_pfd.fd = server.fd();
    server_pfd.events = POLLIN;
    server_pfd.events = 0;
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
            
            //Read
            if (pfd.revents & POLLIN){
                if (!client.read_from_socket()){
                    std::cout << "Client Disconnected! fd=" << client.fd() << std::endl;
                    clients.erase(clients.begin() + (i -1));
                    poll_fds.erase(poll_fds.begin() + 1);
                    continue;
                }

                std::string &rbuf = client.getReadBuffer();
                if(!rbuf.empty()){
                    std::cout << "Received " << rbuf.size() << " bytes from fd=" << client.fd() <<std::endl;

                    //move data into the write buffer
                    client.getWriteBuffer() += rbuf;
                    rbuf.clear();

                    //enable POLLOUT
                    pfd.events |= POLLOUT;
                }
            }
            //write
            if (pfd.revents & POLLOUT){
                client.write_to_socket();
                //Write buffer drained: disable POLLOUT so poll() can sleep properly
                if (client.getWriteBuffer().empty())
                    pfd.events &= ~POLLOUT;
            }
        }
    }
    return 0;
}