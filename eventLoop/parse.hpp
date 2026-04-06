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

    bool        _parseRequestLine();
    bool        _parseHeaders();
    bool        _parseBody();
    bool        _parseChunkedBody();
    void        _processHeadersComplete();
    std::string _trim(const std::string &s) const;
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
    int     getStatusCode() const;
    void    reset();
};

#endif
