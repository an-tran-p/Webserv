# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: atran <atran@student.hive.fi>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/09 11:16:10 by atran             #+#    #+#              #
#    Updated: 2026/04/06 14:17:25 by atran            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = eventLoop

SRCS = main.cpp Socket.cpp ServerSocket.cpp Connection.cpp parse.cpp parse_body.cpp \
	parse_request.cpp

OBJS = $(SRCS:%.cpp=%.o)

CXX = c++

FLAGS = -Wall -Wextra -Werror -std=c++20

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
