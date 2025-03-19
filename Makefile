# Program name
NAME = microshell

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -Werror -g

# Directories
OBJ_DIR = obj

# Source and header files
SRCS = main.c
HEADERS = microshell.h

# Object files
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)

# Dependency files
DEPS = $(OBJS:.o=.d)

# Rule to create the executable
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

# Compile .c files to .o files
$(OBJ_DIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Include generated dependency files
-include $(DEPS)

# Clean object files
clean:
	rm -rf $(OBJ_DIR)

# Clean everything
fclean: clean
	rm -f $(NAME)

# Remake everything
re: fclean all

# Phony targets
.PHONY: all clean fclean re
