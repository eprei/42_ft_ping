NAME = ft_ping

SRCS = src/main.c src/ping.c src/parsing.c src/signal_handler.c src/rtt.c src/net_utils.c src/error.c
OBJ = $(SRCS:.c=.o)

TEST_NAME = test

CC = gcc
CFLAGS = -Wall -Wextra -Werror
LIB_MATH = -lm

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIB_MATH)

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)
	rm -rf $(TEST_NAME)


test: all
	$(CC) $(CFLAGS) src/$(TEST_NAME).c -o $(TEST_NAME) $(LIB_MATH)

run: test
	./$(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re test

#export PS1="\e[0;31m[\u@\h \W]\$ \e[m "