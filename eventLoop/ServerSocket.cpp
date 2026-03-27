/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 14:12:54 by atran             #+#    #+#             */
/*   Updated: 2026/02/16 11:05:13 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerSocket.hpp"

ServerSocket::ServerSocket(int port): _socket(Socket::create_tcp()), _port(port){
    std::memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(_port);
    _addr.sin_addr.s_addr = INADDR_ANY;
}

void ServerSocket::bind_and_listen(){
    int opt = 1;

    if (setsockopt(_socket.fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt toreuseof port failed");

    if (bind(_socket.fd(), reinterpret_cast<sockaddr*>(&_addr), sizeof(_addr)) < 0)
        throw std::runtime_error("bind failed");

    if (listen(_socket.fd(), SOMAXCONN) < 0)
        throw std::runtime_error("listen failed");

    std::cout << "Listening on port " << _port << std::endl;
}

Socket ServerSocket::accept_client() {
    int client_fd = accept(_socket.fd(), nullptr, nullptr);
    if (client_fd < 0)
        throw std::runtime_error("accept failed");

    return Socket(client_fd);
}

int ServerSocket::fd() const {
    return _socket.fd();
}