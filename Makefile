NAME = ft_ping

SRCS = src/main.c src/ping.c src/parsing.c src/signal_handler.c src/rtt.c src/net_utils.c src/print.c src/time.c src/packet_utils.c
OBJ = $(SRCS:.c=.o)

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

re: fclean all

.PHONY: all clean fclean re
	