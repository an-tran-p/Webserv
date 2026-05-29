/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_body.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 14:15:50 by atran             #+#    #+#             */
/*   Updated: 2026/04/06 14:21:34 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parse.hpp"

bool    Request::_parseBody()
{
    size_t    remaining = _contentLength - _bodyBytesRead;
    size_t    available = remaining < _buffer.size() ? remaining : _buffer.size();

    if (available == 0)
        return (false);
    body.append(_buffer, 0, available);
    _buffer.erase(0, available);
    _bodyBytesRead += available;
    if (_bodyBytesRead >= _contentLength)
    {
        _state = DONE;
        return (true);
    }
    return (false);
}

bool    Request::_parseChunkedBody()
{
    while (true)
    {
        if (!_chunkSizeParsed)
        {
            size_t    pos;
            if (!_findCRLF(pos))
                return (false);

            std::string    sizeLine = _buffer.substr(0, pos);
            _buffer.erase(0, pos + 2);

            size_t    semi = sizeLine.find(';');
            if (semi != std::string::npos)
                sizeLine = sizeLine.substr(0, semi);

            sizeLine = _trim(sizeLine);
            if (sizeLine.empty())
                return (_setError(400), false);

            char    *endPtr = NULL;
            errno = 0;
            long    size = std::strtol(sizeLine.c_str(), &endPtr, 16);

            if (*endPtr != '\0' || size < 0 || errno == ERANGE)
                return (_setError(400), false);
            _chunkSize       = static_cast<size_t>(size);
            _chunkSizeParsed = true;
            // [LEE 2026-04-16] chunked body cumulative size limit → 413
            if (_chunkSize > MAX_BODY_SIZE || body.size() > MAX_BODY_SIZE - _chunkSize)
                return (_setError(413), false);
            if (_chunkSize == 0)
            {
                while (true)
                {
                    if (!_findCRLF(pos))
                        return (false);
                    std::string    trailer = _buffer.substr(0, pos);
                    _buffer.erase(0, pos + 2);
                    if (trailer.empty())
                    {
                        _state = DONE;
                        return (true);
                    }
                    size_t    colon = trailer.find(':');
                    if (colon == std::string::npos)
                        return (_setError(400), false);
                    std::string    key = _toLower(_trim(trailer.substr(0, colon)));
                    if (!_isHeaderToken(key))
                        return (_setError(400), false);
                }
            }
        }
        if (_buffer.size() < _chunkSize + 2)
            return (false);
        if (_buffer.compare(_chunkSize, 2, "\r\n") != 0)
            return (_setError(400), false);
        body.append(_buffer, 0, _chunkSize);
        _buffer.erase(0, _chunkSize + 2);
        _chunkSize       = 0;
        _chunkSizeParsed = false;
    }
}
