/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 16:26:49 by atran             #+#    #+#             */
/*   Updated: 2026/02/11 13:52:38 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/types.h>

class Socket{
    private:
        int _fd;
    public:
        Socket();
        explicit Socket(int fd);
        Socket(const Socket &src) = delete;
        Socket &operator=(const Socket &src) = delete;
        Socket (Socket &&src);
        Socket &operator=(Socket &&src);
        ~Socket();

        static Socket create_tcp();
        int fd() const;
        void close_fd();
};