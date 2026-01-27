/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: juhyeonl <juhyeonl@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 00:16:44 by juhyeonl          #+#    #+#             */
/*   Updated: 2026/01/27 16:49:29 by juhyeonl         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parse.hpp"

Request::Request() : _state(REQUEST_LINE), _content_length(0)
{
}

Request::~Request()
{
}

bool	Request::isDone() const
{
	return (_state == DONE);
}

void Request::parse(std::string chunk)
{
    _buffer += chunk;
    while (_state != DONE && _state != ERROR)
    {
        if (_state == REQUEST_LINE)
        {
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos)
				return;
            std::string line = _buffer.substr(0, pos);
            std::stringstream ss(line);
            ss >> method >> path >> protocol;
            _buffer.erase(0, pos + 2);
            _state = HEADERS;
        }
        else if (_state == HEADERS)
        {
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos)
				return ;
            std::string line = _buffer.substr(0, pos);
            if (line.empty())
            {
                _buffer.erase(0, pos + 2);
                if (headers.count("Content-Length"))
                {
                    _content_length = std::atoi(headers["Content-Length"].c_str());
                    _state = BODY;
                }
                else
                    _state = DONE;
            }
            else
            {
                size_t colon = line.find(":");
                if (colon != std::string::npos)
                {
                    std::string key = line.substr(0, colon);
                    std::string value = line.substr(colon + 1);
                    headers[key] = value;
                }
                _buffer.erase(0, pos + 2);
            }
        }
        else if (_state == BODY)
        {
            if (_buffer.length() >= _content_length)
            {
                body = _buffer.substr(0, _content_length);
                _buffer.erase(0, _content_length);
                _state = DONE;
            }
            else
                return ;
        }
    }
}
