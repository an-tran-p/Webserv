/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/11 14:23:32 by atran             #+#    #+#             */
/*   Updated: 2026/02/15 14:12:36 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Socket.hpp"
#include <netinet/in.h>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

class ServerSocket{
    private:
        Socket _socket;
        sockaddr_in _addr;
        int _port;

    public:
        explicit ServerSocket(int port);
        ServerSocket(const ServerSocket &src) = delete;
        ServerSocket &operator=(const ServerSocket &src) = delete;
        ServerSocket(ServerSocket&& src);
        ServerSocket &operator=(ServerSocket &&src);
        ~ServerSocket() = default;

        void createSocket(int port);
        void bind_and_listen();
        Socket accept_client();
        int fd() const;
};