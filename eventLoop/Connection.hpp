/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/27 09:39:49 by atran             #+#    #+#             */
/*   Updated: 2026/04/14 10:25:34 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Socket.hpp"
#include <string>

class Connection {
    private:
        Socket _socket;
        std::string _readBuffer;
        std::string _writeBuffer;
        bool _closeAfterWrite;

    public:
        explicit Connection(Socket && socket);
        Connection (const Connection &src) = delete;
        Connection &operator=(const Connection &src) = delete;
        Connection(Connection &&src) noexcept = default;
        Connection &operator=(Connection &&src) noexcept = default;
        ~Connection() = default;
        int fd() const;
        std::string &getReadBuffer();
        std::string &getWriteBuffer();
        bool read_from_socket();
        bool write_to_socket();
        void setCloseAfterWrite(int status);
        bool shouldCloseAfterWrite() const;
};