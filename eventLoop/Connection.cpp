/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/27 09:58:35 by atran             #+#    #+#             */
/*   Updated: 2026/04/04 13:41:15 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

Connection::Connection(Socket &&socket): _socket(std::move(socket)), _readBuffer(), _writeBuffer() {}

int Connection::fd() const{
    return _socket.fd();
}

std::string &Connection::getReadBuffer(){
    return _readBuffer;
}

std::string &Connection::getWriteBuffer(){
    return _writeBuffer;
}

bool Connection::read_from_socket(){
    char buffer[1024];
    int bytes = recv(_socket.fd(), buffer, sizeof(buffer), 0);

    if (bytes == 0)
        return false;
    else if (bytes < 0){
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return true;
        perror("recv");
        return false;
    }
    _readBuffer.append(buffer, bytes);
    return true;
}

bool Connection::write_to_socket(){
    if (_writeBuffer.empty())
        return true;

    int bytes = send(_socket.fd(), _writeBuffer.c_str(), _writeBuffer.size(), 0);
    if (bytes <= 0){
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return true;
        perror("send");
        return false;
    }
    _writeBuffer.erase(0, bytes);
    return true;
}