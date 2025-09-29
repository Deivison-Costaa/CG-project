CXX = g++
TARGET = apk
CXXFLAGS = -std=c++17 -g -Wall -O3

SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(filter %.cpp, $(SRCS)))
OBJS += $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(filter %.c, $(SRCS)))

LIBS = -lglfw -lGL -ldl -lassimp
RM = rm -f

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	$(RM) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	$(RM) $(TARGET)
	$(RM) -rf $(src)/*.o

.PHONY: all clean
