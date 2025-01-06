PROG                                  =       pipex

CC                                              =       gcc
CFLAGS                                  =       -Wall -Wextra -Werror
RM                                              =       rm -rf

SRC                                     =       pipex.c

OBJ_DIR                                 =       obj
PROG_OBJ                              =       $(SRC:%.c=$(OBJ_DIR)/%.o)


INCLS                                   =       -I ./includes/ -I ./libft

LIBFT_PATH                              =       ./libft
LIBFT                                   =       $(LIBFT_PATH)/libft.a
LIBINCL                                 =       -L $(LIBFT_PATH) -lft

all:                                    $(PROG)
										@echo "\033[32m[Pipex prêt à l'emploi]\033[0m"

$(PROG):                              $(LIBFT) $(OBJ_DIR) $(PROG_OBJ)
												$(CC) $(CFLAGS) $(PROG_OBJ) -o $(PROG) $(LIBINCL)
												@echo "\033[32m[pipex créé]\033[0m"


$(OBJ_DIR)/%.o: %.c
												$(CC) $(CFLAGS) $(INCLS) -c $< -o $@

$(LIBFT):
												$(MAKE) -C $(LIBFT_PATH) all

$(OBJ_DIR):
												mkdir -p $(OBJ_DIR)

clean:
												$(RM) $(OBJ_DIR)
												@echo "\033[33m[Nettoyé]\033[0m"

fclean:                                 clean
												$(MAKE) -C $(LIBFT_PATH) fclean
												$(RM) $(PROG)
												@echo "\033[33m[Suppression totale]\033[0m"

re:                                             fclean all
