/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 10:22:32 by atran             #+#    #+#             */
/*   Updated: 2026/05/23 19:06:49 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"
#include "../include/ConfigParser.hpp"
#include <csignal>

volatile sig_atomic_t g_signal = 0;

void signalHandler(int sig)
{
    g_signal = sig;
}

int main(int ac, char **av)
{
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);

    // 1. parse config file
    ConfigParser parser;
    parser.setFilepath(ac, av);
    auto result = parser.parseConfig();
    if (!result.has_value() || result->empty())
    {
        std::cerr << "Failed to parse config file\n";
        return 1;
    }

    ServerState state;
    state.configs = *result;

    // 2. one socket per port, but multiple configs per port
    std::vector<ServerSocket*> servers;
    std::map<int, ServerSocket*> portToSocket;

    for (size_t idx = 0; idx < state.configs.size(); idx++)
    {
        int port = state.configs[idx].getPort();

        if (portToSocket.find(port) == portToSocket.end())
        {
            // new port, create a socket
            ServerSocket* srv = new ServerSocket(port);
            srv->bind_and_listen();
            servers.push_back(srv);
            portToSocket[port] = srv;

            pollfd pfd;
            pfd.fd      = srv->fd();
            pfd.events  = POLLIN;
            pfd.revents = 0;
            state.poll_fds.push_back(pfd);

            std::cout << "Server running on port " << port << "\n";
        }

        // map this fd to this config
        state.fdToConfigs[portToSocket[port]->fd()].push_back(&state.configs[idx]);
    }

    state.numServers = servers.size();

    // 3. main loop
    while (true)
    {
        if (g_signal)
        {
            std::cout << "Shutting down server...\n";
            break;
        }

        int ret = poll(state.poll_fds.data(), state.poll_fds.size(), 1000);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            perror("poll");
            break;
        }

        // check all listening fds
        for (size_t idx = 0; idx < servers.size(); idx++)
        {
            if (state.poll_fds[idx].revents & POLLIN)
                addClient(*servers[idx], state, state.fdToConfigs[servers[idx]->fd()]);
        }

        // handle clients
        for (size_t i = servers.size(); i < state.poll_fds.size(); ++i)
        {
            if (state.poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                removeClient(i, state);
                continue;
            }
            if (checkTimeout(i, state))
                continue;
            if (handleRead(i, state))
                continue;
            handleWrite(i, state);
        }
    }

    for (ServerSocket* srv : servers)
        delete srv;

    return 0;
}
