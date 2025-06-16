# Compiler and flags
CC = c++
CFLAGS = -std=c++98 -Iincludes -Wall -Wextra -Werror

# Target executable name
NAME = webserv

# Source and Object directories
SRC_DIR = srcs
OBJ_DIR = objs

# Source files
SRCS =  $(SRC_DIR)/AConfigBase.cpp $(SRC_DIR)/HttpConfig.cpp $(SRC_DIR)/LocationConfig.cpp \
		$(SRC_DIR)/ServerConfig.cpp $(SRC_DIR)/main.cpp $(SRC_DIR)/Utils.cpp \
		$(SRC_DIR)/PollServer.cpp  $(SRC_DIR)/RequestsManager.cpp $(SRC_DIR)/Request.cpp \
		$(SRC_DIR)/Response.cpp $(SRC_DIR)/CgiHandler.cpp \
		$(SRC_DIR)/Logger.cpp $(SRC_DIR)/Parser/Parser.cpp $(SRC_DIR)/Parser/ParserBlock.cpp \
		$(SRC_DIR)/Parser/ParserConfig.cpp $(SRC_DIR)/Parser/ParserFiller.cpp \
		$(SRC_DIR)/Parser/ParserUtils.cpp $(SRC_DIR)/Response/BuildDirPath.cpp \
		$(SRC_DIR)/Response/buildIndexPath.cpp $(SRC_DIR)/Response/GenerateListing.cpp \
		$(SRC_DIR)/Response/getMimeType.cpp $(SRC_DIR)/Response/MatchMethod.cpp \
		$(SRC_DIR)/Response/Utils.cpp

# Object files (convert .cpp to .o)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Default target
all: $(NAME)

# Link the executable
$(NAME): $(OBJS)
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "\033[0;31mBuild with Makefile\033[0m"

# Compile object files in srcs/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files
clean:
	rm -rf $(OBJ_DIR)
	@echo "\033[0;31mObject files removed\033[0m"

# Clean object files and executable
fclean: clean
	rm -f $(NAME)
	@echo "\033[0;31mExecutable files removed\033[0m"

# Rebuild the project
re: fclean all
	@echo "\033[0;31mProject rebuilded\033[0m"

# Phony targets
.PHONY: all clean fclean re
