/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 00:15:36 by juhyeonl          #+#    #+#             */
/*   Updated: 2026/04/06 15:07:11 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSE_HPP
#define PARSE_HPP
#include <string>
#include <map>
#include <cstdlib>
#include <iostream>
#include <ctime>

// === [LEE 2026-04-16] limits & timeout for request validation ===
#define MAX_URI_LENGTH   8192
#define MAX_BODY_SIZE    1048576
#define REQUEST_TIMEOUT  30
// === [LEE end] ===

enum e_parse_state
{
    REQUEST_LINE,
    HEADERS,
    BODY,
    CHUNKED_BODY,
    DONE,
    ERROR
};

// HTTP request parser — called by An's event loop via parse() and isDone()
// Results are read directly by Jasmine's routing layer via public fields
class Request
{
private:
    std::string     _buffer;
    e_parse_state   _state;
    size_t          _contentLength;
    size_t          _bodyBytesRead;
    size_t          _chunkSize;
    bool            _chunkSizeParsed;
    int             _statusCode;
    time_t          _lastActivity;  // [LEE] for request timeout tracking

    bool        _parseRequestLine();
    bool        _parseHeaders();
    bool        _parseBody();
    bool        _parseChunkedBody();
    void        _processHeadersComplete();
    std::string _trim(const std::string &s) const;
    // === [LEE 2026-04-16] validation helpers ===
    std::string _toLower(const std::string &s) const;
    bool        _isValidPath(const std::string &p) const;
    bool        _isValidProtocol(const std::string &p) const;
    // === [LEE end] ===
    bool        _findCRLF(size_t &pos) const;
    void        _setError(int code);

public:
    std::string                         method;
    std::string                         path;
    std::string                         queryString;
    std::string                         protocol;
    std::map<std::string, std::string>  headers;
    std::string                         body;
    bool                                keepAlive;
    bool                                isChunked;

    Request();
    ~Request();

    void    parse(const std::string &chunk);
    bool    isDone() const;
    bool    isError() const;
    // === [LEE 2026-04-16] incomplete request / timeout API ===
    bool    isWaiting() const;
    bool    hasTimedOut(time_t now) const;
    // === [LEE end] ===
    int     getStatusCode() const;
    time_t  getLastActivity() const;  // [LEE]
    void    reset();
};

#endif
