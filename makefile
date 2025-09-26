# Compilador
CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -I/usr/include

# Bibliotecas
LIBS = -lglfw -ldl -lGL -lassimp

# Fontes e objeto
SRCS = src/main.cpp src/glad.c src/Terrain.cpp src/Shader.cpp src/Camera.cpp src/Sun.cpp src/Water.cpp src/GrassField.cpp
OBJS = $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)

# Executável
TARGET = apk

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Regra genérica para .cpp e .c
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/%.o: src/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -f $(OBJS) $(TARGET)
