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


int main() {
    const int PORT = 8080;
    ServerSocket server(PORT);
    server.bind_and_listen();

    ServerState state;

    pollfd server_pfd;
    server_pfd.fd      = server.fd();
    server_pfd.events  = POLLIN;
    server_pfd.revents = 0;
    state.poll_fds.push_back(server_pfd);

    std::cout << "Server running on port " << PORT << "\n";
    while (true) {
        int ret = poll(state.poll_fds.data(), state.poll_fds.size(), 1000);
        if (ret < 0) {
            perror("poll");
            break;
        }
        if (state.poll_fds[0].revents & POLLIN)
            addClient(server, state);

        for (size_t i = 1; i < state.poll_fds.size(); ++i) {
            if (checkTimeout(i, state))
                continue;
            if (handleRead(i, state))
                continue;
            handleWrite(i, state);
        }
    }
    return 0;
}