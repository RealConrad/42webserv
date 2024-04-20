NAME                := webserv
CPP                 := c++
INCLUDES			:= -I./includes
CXXFLAGS            := -std=c++98 -Wall -Wextra -Werror -g $(INCLUDES)

# ------------------------------- Source files ------------------------------- #
OBJ_DIR             := ./objs

VPATH               := ./src/ ./src/config-manager/ ./src/utils/ ./src/logger/

SRC                 := main.cpp SocketManager.cpp HTTPRequest.cpp HTTPResponse.cpp
CONFIG_MANAGER      := ConfigManager.cpp
LOGGER				:= Logger.cpp
UTILS			    := utils.cpp

SRCS                := $(SRC) $(CONFIG_MANAGER) $(UTILS) $(LOGGER)
OBJS                := $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CPP) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
