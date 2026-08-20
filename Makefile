NAME = ft_ping

S_DIR = src/
O_DIR = obj/
L_DIR = libmb/

SRCS = $(wildcard $(S_DIR)*.c)

OBJ = $(patsubst $(S_DIR)%.c, $(O_DIR)%.o, $(SRCS))

HDR = $(addprefix $(S_DIR), ft_ping.h)
LIB = $(addprefix $(L_DIR), libmb.a)

CFLAGS = -Wall -Wextra -Werror -g
LIBFLAGS = -Llibmb -lmb

all: $(NAME)

$(NAME): $(LIB) $(HDR) $(OBJ)
	gcc -o $@ $(OBJ) $(LIBFLAGS)

$(LIB):
	make -C $(L_DIR)

$(O_DIR)%.o: $(S_DIR)%.c $(HDR) | $(O_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(O_DIR):
	mkdir $@

clean:
	rm -rf $(O_DIR)
	make -C $(L_DIR) clean

fclean: clean
	rm -f $(NAME)
	make -C $(L_DIR) fclean

re: fclean all

.PHONY: all clean fclean re