# Compiler and flags
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -pthread -Iincludes

# Target executable name
NAME = webserv

# Source and Object directories
SRC_DIR = srcs
OBJ_DIR = objs

# Source files
SRCS = $(SRC_DIR)/test.cpp $(SRC_DIR)/EventConf.cpp $(SRC_DIR)/HttpConf.cpp \
       $(SRC_DIR)/LocationConf.cpp $(SRC_DIR)/ParseConf.cpp $(SRC_DIR)/ServerConf.cpp \
	   $(SRC_DIR)/HttpServer.cpp $(SRC_DIR)/WebServConf.cpp $(SRC_DIR)/ParsedRequest.cpp \
	   $(SRC_DIR)/FileHandler.cpp $(SRC_DIR)/Logger.cpp $(SRC_DIR)/CgiHandler.cpp \
	   $(SRC_DIR)/ConfigAccess.cpp $(SRC_DIR)/ConfigParser.cpp $(SRC_DIR)/ConfigBlock.cpp \
	   $(SRC_DIR)/ConfigDirective.cpp $(SRC_DIR)/Utils.cpp $(SRC_DIR)/ClientManager.cpp \
	   $(SRC_DIR)/SocketManager.cpp $(SRC_DIR)/RequestHandler.cpp

# Object files (convert .cpp to .o)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Project root source file (serv2.cpp)

# Default target
all: $(NAME)

# Link the executable
$(NAME): $(OBJS)
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "\033[0;31mBuild with Makefile\033[0m"

# Compile object files in srcs/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
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
