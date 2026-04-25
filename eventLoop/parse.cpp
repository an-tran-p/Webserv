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
      _lastActivity(std::time(nullptr)),  // [LEE]
      _emptyLineCount(0),  // [LEE 2026-04-26] empty line counter init
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
    _lastActivity    = std::time(nullptr);  // [LEE]
    _emptyLineCount  = 0;  // [LEE 2026-04-26]
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
    _lastActivity = std::time(nullptr);  // [LEE] reset idle timer on data

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

// === [LEE 2026-04-16] incomplete request / timeout API ===
bool    Request::isWaiting() const
{
    return (_state != DONE && _state != ERROR);
}

bool    Request::hasTimedOut(time_t now) const
{
    return (isWaiting() && (now - _lastActivity) > REQUEST_TIMEOUT);
}
// === [LEE end] ===

int    Request::getStatusCode() const
{
    return (_statusCode);
}

time_t    Request::getLastActivity() const  // [LEE]
{
    return (_lastActivity);
}

// === [LEE 2026-04-26] pipelining support ===
// Returns leftover bytes in _buffer that belong to the next pipelined request.
const std::string&    Request::getLeftover() const
{
    return (_buffer);
}

// Reset all state EXCEPT _buffer, then re-trigger parse() so any leftover
// bytes from a pipelined request start being processed immediately.
// main.cpp can call this with a single line instead of manually copying _buffer.
void    Request::resetKeepBuffer()
{
    _state           = REQUEST_LINE;
    _contentLength   = 0;
    _bodyBytesRead   = 0;
    _chunkSize       = 0;
    _chunkSizeParsed = false;
    _statusCode      = 0;
    _lastActivity    = std::time(nullptr);
    _emptyLineCount  = 0;
    keepAlive        = false;
    isChunked        = false;
    method.clear();
    path.clear();
    queryString.clear();
    protocol.clear();
    headers.clear();
    body.clear();
    parse("");
}
// === [LEE 2026-04-26 end] ===

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

// === [LEE 2026-04-16] validation helpers ===
std::string    Request::_toLower(const std::string &s) const
{
    std::string    out(s);
    for (size_t i = 0; i < out.size(); ++i)
    {
        if (out[i] >= 'A' && out[i] <= 'Z')
            out[i] = static_cast<char>(out[i] + 32);
    }
    return (out);
}

bool    Request::_isValidPath(const std::string &p) const
{
    if (p.empty() || p[0] != '/')
        return (false);
    for (size_t i = 0; i < p.size(); ++i)
    {
        unsigned char    c = static_cast<unsigned char>(p[i]);
        if (c < 0x20 || c == 0x7F)
            return (false);
    }
    if (p == "/.." || p.find("/../") != std::string::npos)
        return (false);
    if (p.size() >= 3 && p.compare(p.size() - 3, 3, "/..") == 0)
        return (false);
    return (true);
}

bool    Request::_isValidProtocol(const std::string &p) const
{
    return (p == "HTTP/1.0" || p == "HTTP/1.1");
}
// === [LEE end] ===
