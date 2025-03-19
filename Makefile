# Program name
NAME = microshell

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -Werror -g

# Directories
#SRC_DIR = src
#OBJ_DIR = obj
#INC_DIR = inc

# Source and header files
SRCS = main.c
HEADERS = microshell.h

# Object files
OBJS = $(SRCS:%.c=%.o)

# Dependency files
DEPS = $(OBJS:.o=.d)

# Rule to create the executable
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

# Compile .c files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -I$(INC_DIR) -c $< -o $@

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
