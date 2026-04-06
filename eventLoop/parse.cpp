/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 00:16:44 by juhyeonl          #+#    #+#             */
/*   Updated: 2026/04/06 15:18:23 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parse.hpp"

Request::Request()
    : _state(REQUEST_LINE),
      _contentLength(0),
      _bodyBytesRead(0),
      _chunkSize(0),
      _chunkSizeParsed(false),
      _statusCode(0),
      keepAlive(false),
      isChunked(false)
{}

Request::~Request() {}

void    Request::reset()
{
    _buffer.clear();
    _state           = REQUEST_LINE;
    _contentLength   = 0;
    _bodyBytesRead   = 0;
    _chunkSize       = 0;
    _chunkSizeParsed = false;
    _statusCode      = 0;
    keepAlive        = false;
    isChunked        = false;
    method.clear();
    path.clear();
    queryString.clear();
    protocol.clear();
    headers.clear();
    body.clear();
}

void    Request::parse(const std::string &chunk)
{
    _buffer += chunk;

    bool    progress = true;
    while (progress && _state != DONE && _state != ERROR)
    {
        progress = false;
        if (_state == REQUEST_LINE)
            progress = _parseRequestLine();
        else if (_state == HEADERS)
            progress = _parseHeaders();
        else if (_state == BODY)
            progress = _parseBody();
        else if (_state == CHUNKED_BODY)
            progress = _parseChunkedBody();
    }
}

bool    Request::isDone() const
{
    return (_state == DONE);
}

bool    Request::isError() const
{
    return (_state == ERROR);
}

int    Request::getStatusCode() const
{
    return (_statusCode);
}

void    Request::_setError(int code)
{
    _statusCode = code;
    _state      = ERROR;
}

std::string    Request::_trim(const std::string &s) const
{
    const std::string    ws = " \t\r\n";
    size_t                start = s.find_first_not_of(ws);

    if (start == std::string::npos)
        return ("");
    size_t    end = s.find_last_not_of(ws);
    return (s.substr(start, end - start + 1));
}

bool    Request::_findCRLF(size_t &pos) const
{
    size_t    found = _buffer.find("\r\n");

    if (found == std::string::npos)
        return (false);
    pos = found;
    return (true);
}
