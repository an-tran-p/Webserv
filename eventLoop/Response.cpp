/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 15:40:09 by atran             #+#    #+#             */
/*   Updated: 2026/05/22 13:34:36 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(): _statusCode(200), _contentType("text/plain"), _body(""){}

void Response::setStatus(int code){
    _statusCode = code;
}

void Response::setContentType(const std::string &type){
    _contentType = type;
}

void Response::setBody(const std::string &body){
    _body = body;
}

std::string Response::_getReason(int code){
    static const std::map<int, std::string> reasons = {
        {200, "OK"},
        {201, "Created"},
        {204, "No Content"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {400, "Bad Request"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {408, "Request Timeout"},
        {413, "Content Too Large"},
        {414, "URI Too Long"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {504, "Gateway Timeout"},
    };
    auto it = reasons.find(code);
    if (it != reasons.end())
        return it->second;
    else
        return "Unknown";
}

std::string Response::build(bool keepAlive) const {
    std::string resp;
    std::string connection;

    if (keepAlive)
        connection = "keep-alive";
    else
        connection = "close";

    resp += "HTTP/1.1 " + std::to_string(_statusCode) + " " + _getReason(_statusCode) + "\r\n";
    resp += "Content-Type: " + _contentType + "\r\n";
    resp += "Content-Length: " + std::to_string(_body.size()) + "\r\n";
    resp += "Connection: " + connection + "\r\n";
    if (!_location.empty())
        resp += "Location: " + _location + "\r\n";
    resp += "\r\n";
    resp += _body;
    return resp;
}

Response Response::makeError(int code) {
    Response r;
    r.setStatus(code);
    r.setContentType("text/plain");
    r.setBody(std::to_string(code) + " " + _getReason(code));
    return r;
}

void Response::setLocation(const std::string& url)
{
    _location = url; 
}