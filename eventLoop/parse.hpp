/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: juhyeonl <juhyeonl@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 00:15:36 by juhyeonl          #+#    #+#             */
/*   Updated: 2026/01/27 16:46:30 by juhyeonl         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <cstdlib>

enum	e_parse_state
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	DONE,
	ERROR
};

class	Request
{
	private: 
		std::string		_buffer;
		e_parse_state	_state;
		size_t			_content_length;
	public:
		std::string		method;
		std::string		path;
		std::string		protocol;
		std::map<std::string, std::string> headers;
		std::string		body;
		
		Request();
		~Request();

		void	parse(std::string chunk);
		bool	isDone() const;
};

#endif
