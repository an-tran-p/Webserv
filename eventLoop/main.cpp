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

    // 1. config
    ConfigParser parser;
    parser.setFilepath(ac, av);
    auto result = parser.parseConfig();
    if (!result.has_value() || result->empty())
    {
        std::cerr << "Failed to parse config file\n";
        return 1;
    }

    ServerState state;
    state.configs = *result;  // all the configs

    // 2. sockets for different servers
    std::vector<ServerSocket*> servers;
    state.numServers = state.configs.size();
    for (size_t idx = 0; idx < state.configs.size(); idx++)
    {
        int port = state.configs[idx].getPort();
        ServerSocket* srv = new ServerSocket(port);
        srv->bind_and_listen();
        servers.push_back(srv);

        pollfd pfd;
        pfd.fd      = srv->fd();
        pfd.events  = POLLIN;
        pfd.revents = 0;
        state.poll_fds.push_back(pfd);

        // which fd for which server config
        state.fdToConfig[srv->fd()] = &state.configs[idx];

        std::cout << "Server running on port " << port << "\n";
    }

    
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

        for (size_t idx = 0; idx < servers.size(); idx++)
        {
            if (state.poll_fds[idx].revents & POLLIN)
                addClient(*servers[idx], state, &state.configs[idx]);
        }

        // client
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
