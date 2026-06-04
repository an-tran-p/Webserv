/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 13:33:15 by atran             #+#    #+#             */
/*   Updated: 2026/04/17 15:46:26 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <map>

class Response {
    private:
        int _statusCode;
        std::string _contentType;
        std::string _body;
        std::string _location; // for redirection

        static std::string _getReason(int code);
    public:
        Response();
        Response(const Response &src) = default;
        Response &operator=(const Response &src) = default;
        ~Response() = default;
        void setStatus(int code);
        void setContentType(const std::string &type);
        void setBody(const std::string &body);
        std::string build(bool keepAlive) const;
        static Response makeError(int code);
        void setLocation(const std::string& url);
};