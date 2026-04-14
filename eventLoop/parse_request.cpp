/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_request.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 14:15:54 by atran             #+#    #+#             */
/*   Updated: 2026/04/10 18:20:56 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parse.hpp"

bool    Request::_parseRequestLine()
{
    size_t    pos;

    if (!_findCRLF(pos))
        return (false);

    std::string    line = _buffer.substr(0, pos);
    _buffer.erase(0, pos + 2);

    if (_trim(line).empty())
        return (true);

    size_t    sp1 = line.find(' ');
    if (sp1 == std::string::npos)
        return (_setError(400), false);

    size_t    sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos)
        return (_setError(400), false);

    method   = line.substr(0, sp1);
    protocol = line.substr(sp2 + 1);

    if (method != "GET" && method != "POST" && method != "DELETE")
        return (_setError(405), false);

    std::string    rawUri = line.substr(sp1 + 1, sp2 - sp1 - 1);
    if (rawUri.empty() || rawUri[0] != '/')
        return (_setError(400), false);

    size_t    qmark = rawUri.find('?');
    if (qmark != std::string::npos)
    {
        path        = rawUri.substr(0, qmark);
        queryString = rawUri.substr(qmark + 1);
    }
    else
    {
        path        = rawUri;
        queryString = "";
    }
    _state = HEADERS;
    return (true);
}

bool    Request::_parseHeaders()
{
	std::string    key;
	std::string    value;
    while (true)
    {
        size_t    pos;
        if (!_findCRLF(pos))
            return (false);
        std::string    line = _buffer.substr(0, pos);
        _buffer.erase(0, pos + 2);
        if (!key.empty() && line.empty())
        {
            _processHeadersComplete();
            return (true);
        }
        size_t    colon = line.find(':');
        if (colon == std::string::npos)
            return (_setError(400), false);
        key   = _trim(line.substr(0, colon));
        value = _trim(line.substr(colon + 1));

        if (key.empty())
            return (_setError(400), false);
        headers[key] = value;
    }
}

void    Request::_processHeadersComplete()
{
    std::map<std::string, std::string>::iterator    it;

    it = headers.find("Connection");
    if (it != headers.end())
        keepAlive = (_trim(it->second) == "keep-alive");
    else
        keepAlive = (protocol == "HTTP/1.1");

    it = headers.find("Transfer-Encoding");
    if (it != headers.end() && _trim(it->second) == "chunked")
        isChunked = true;

    it = headers.find("Content-Length");
    if (it != headers.end())
    {
        char    *endPtr = nullptr;
        long    len = std::strtol(it->second.c_str(), &endPtr, 10);

        if (*endPtr != '\0' || len < 0)
        {
            _setError(400);
            return ;
        }
        _contentLength = static_cast<size_t>(len);
    }
    if (isChunked)
        _state = CHUNKED_BODY;
    else if (_contentLength > 0)
        _state = BODY;
    else
        _state = DONE;
}
