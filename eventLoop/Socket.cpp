/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 16:37:25 by atran             #+#    #+#             */
/*   Updated: 2026/02/11 14:04:05 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(): _fd(-1){}

Socket::Socket(int fd): _fd(fd){}

Socket::~Socket(){
    if (_fd != -1) {
        ::close(_fd);
        printf("CLOSING %d\n", _fd);
    }
}

Socket::Socket(Socket &&src): _fd(src._fd){
    src._fd = -1;
}

Socket &Socket::operator=(Socket &&src){
    if (this != &src){
        close_fd();
        _fd = src._fd;
        src._fd = -1;
    }
    return *this;
}

Socket Socket::create_tcp(){
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("Failed to create socket");
    return Socket(fd);
}

int Socket::fd() const{
    return _fd;
}

void Socket::close_fd(){
    if (_fd != -1){
        ::close(_fd);
        _fd = -1;
    }
}

// Socket::Socket(const Socket &src)
// {
//     _fd = src._fd;
// }

// Socket &Socket::operator=(const Socket &src)
// {
//     if (this != &src)
//         _fd = src._fd;
//     return *this;
// }