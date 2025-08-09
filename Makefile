CXX := g++
SRC := $(wildcard src/*.cpp)
TARGET := awa
CXXFLAGS := -std=c++20 -Oz -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections,--build-id=none,--as-needed,--icf=all -fuse-ld=gold

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)