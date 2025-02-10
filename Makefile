# Compiler and flags
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -pthread

# Target executable name
NAME = webserver

# Source files
SRCS = serv.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(NAME)

# Link the executable
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "\033[0;31mBuild with Makefile\033[0m"

# Compile object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files
clean:
	rm -f $(OBJS)
	@echo "\033[0;31mRemove object files\033[0m"

# Clean object files and executable
fclean: clean
	rm -f $(NAME)
	@echo "\033[0;31mRemove executable file\033[0m"

# Rebuild the project
re: fclean all
	@echo "\033[0;31mRebuild project\033[0m"

# Phony targets
.PHONY: all clean fclean re
