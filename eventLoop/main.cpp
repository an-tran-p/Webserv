/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atran <atran@student.hive.fi>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 10:22:32 by atran             #+#    #+#             */
/*   Updated: 2026/02/11 14:19:49 by atran            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include <iostream>

int main(){
    try{
        Socket s = Socket::create_tcp();
        std::cout << "Socket fd: " << s.fd() << std::endl;
    } catch (const std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    Socket s = Socket::create_tcp();
    Socket s1 = std::move(s);
    std::cout << "After move constructor:" << std::endl;
    std::cout << "s.fd() = " << s.fd() << std::endl;
    std::cout << "s1.fd() = " << s1.fd() << std::endl;

    Socket s3 = Socket::create_tcp();
    Socket s2 = Socket::create_tcp();

    std::cout << "Before move assignment:" << std::endl;
    std::cout << "s3.fd() = " << s3.fd() << std::endl;
    std::cout << "s2.fd() = " << s2.fd() << std::endl;

    // Move s1 into s2
    s2 = std::move(s3);  // move assignment is called

    std::cout << "After move assignment:" << std::endl;
    std::cout << "s3.fd() = " << s3.fd() << std::endl; // -1
    std::cout << "s2.fd() = " << s2.fd() << std::endl;

    return 0;
}