CXX := g++
SRC := $(wildcard src/*.cpp)
TARGET := awa
CXXFLAGS := -std=c++20 -O2

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)