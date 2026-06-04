/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/23 14:32:19 by atran             #+#    #+#             */
/*   Updated: 2026/05/23 14:40:42 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Socket.hpp"
#include "ServerSocket.hpp"
#include "Connection.hpp"
#include "Response.hpp"
#include "parse.hpp"
#include <iostream>
#include <poll.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include  "../include/ServerConfig.hpp"


struct ServerState {
    std::vector<Connection> clients;
    std::vector<Request> requests;
    std::vector<pollfd> poll_fds;
    std::vector<std::chrono::steady_clock::time_point> connectTime;
    ServerConfig config;
};

bool tryParseRequest(Connection& client, Request &req);
void removeClient(size_t &i, ServerState& state);
void addClient(ServerSocket &server, ServerState &state);
bool checkTimeout(size_t &i, ServerState &state);
bool handleRead(size_t &i, ServerState &state);
bool handleWrite(size_t &i, ServerState &state);
