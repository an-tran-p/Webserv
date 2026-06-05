# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: atran <atran@student.hive.fi>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/09 11:16:10 by atran             #+#    #+#              #
#    Updated: 2026/05/23 19:07:17 by atran            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = eventLoop

SRCS = main.cpp Socket.cpp ServerSocket.cpp Connection.cpp parse.cpp parse_body.cpp \
	parse_request.cpp Response.cpp EventLoop.cpp \
	../config/ConfigParser.cpp ../config/ServerConfig.cpp ../config/LocationConfig.cpp

OBJS = $(SRCS:%.cpp=%.o)

CXX = c++

FLAGS = -Wall -Wextra -Werror -std=c++20 -I ../include

RM = rm -rf

all: $(NAME)

$(OBJS): %.o: %.cpp
	$(CXX) $(FLAGS) -c $< -o $@

$(NAME) : $(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

clean :
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
