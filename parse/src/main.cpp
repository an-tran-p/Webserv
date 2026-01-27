/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: juhyeonl <juhyeonl@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 22:07:11 by juhyeonl          #+#    #+#             */
/*   Updated: 2026/01/27 16:49:15 by juhyeonl         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parse.hpp"
#include <iostream>
#include <vector>

int main()
{
	Request req;

	std::vector<std::string> chunks;
	chunks.push_back("GET /index.html HT");
	chunks.push_back("TP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\n");
	chunks.push_back("hello");

	std::cout << "--- Parsing Start ---" << std::endl;

	for (size_t i = 0; i < chunks.size(); ++i)
	{
		std::cout << "[Chunk " << i << " delivering...]" << std::endl;
		req.parse(chunks[i]);

		if (req.isDone())
		{
			std::cout << ">> parse done!" << std::endl;
			break;
		}
		else
			std::cout << ">> It's need more data (now state: " << i << ")" << std::endl;
	}
	if (req.isDone())
	{
		std::cout << "\n--- result ---" << std::endl;
		std::cout << "Method: " << req.method << std::endl;
		std::cout << "Path: " << req.path << std::endl;
		std::cout << "Host: " << req.headers["Host"] << std::endl;
		std::cout << "Body: " << req.body << std::endl;
	}
	return 0;
}
