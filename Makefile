CXX := clang++
CXXFLAGS := -Wall -Wextra -std=c++20
SRC_DIR := src
BUILD_DIR := build
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
TARGET := $(BUILD_DIR)/app

DEBUG_CXXFLAGS := $(CXXFLAGS) -g
RELEASE_CXXFLAGS := $(CXXFLAGS)

.PHONY: all debug release clean

all: debug  # Explicitly defining how to build 'all'

debug: CXXFLAGS := $(DEBUG_CXXFLAGS)
debug: $(TARGET)

release: CXXFLAGS := $(RELEASE_CXXFLAGS)
release: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*
