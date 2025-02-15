# Compiler and flags
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -pthread -Iincludes  # ✅ includes 경로 추가

# Target executable name
NAME = webserver

# Source and Object directories
SRC_DIR = srcs
OBJ_DIR = objs

# Source files
SRCS = $(SRC_DIR)/EventConf.cpp $(SRC_DIR)/HttpConf.cpp \
       $(SRC_DIR)/LocationConf.cpp $(SRC_DIR)/ParseConf.cpp $(SRC_DIR)/ServerConf.cpp

# Object files (convert .cpp to .o)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Project root source file (serv2.cpp)
SERV_SRC = serv2.cpp
SERV_OBJ = $(OBJ_DIR)/serv2.o

# Default target
all: $(NAME)

# Link the executable
$(NAME): $(OBJS) $(SERV_OBJ)  # ✅ serv2.o 포함해야 함
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(SERV_OBJ)
	@echo "\033[0;31mBuild with Makefile\033[0m"

# Compile object files in srcs/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile serv2.o separately
$(SERV_OBJ): $(SERV_SRC)
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files
clean:
	rm -rf $(OBJ_DIR)
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
