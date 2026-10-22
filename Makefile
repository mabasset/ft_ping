NAME = ft_ping

S_DIR = src/
O_DIR = obj/

SRCS = $(wildcard $(S_DIR)*.c)

OBJ = $(patsubst $(S_DIR)%.c, $(O_DIR)%.o, $(SRCS))

HDR = $(addprefix $(S_DIR), ft_ping.h)

CFLAGS = -Wextra -Werror -Wall -Wconversion -std=gnu2x
LIBFLAGS = -lm

PREFIX = /usr/local

all: $(NAME)

$(NAME): $(LIB) $(OBJ)
	gcc -o $@ $(OBJ) $(LIBFLAGS)

$(O_DIR)%.o: $(S_DIR)%.c $(HDR) | $(O_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(O_DIR):
	mkdir $@

clean:
	rm -rf $(O_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

setuid: $(NAME)
	sudo chown root:root ./$(NAME)
	sudo chmod u+s ./$(NAME)

install: $(NAME)
	sudo install -o root -g root -m 4755 $(NAME) $(PREFIX)/bin/$(NAME)

uninstall:
	sudo rm -f $(PREFIX)/bin/$(NAME)

.PHONY: all clean fclean re setuid install uninstall