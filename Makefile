CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2
INCLUDES  = -Iinclude

APP_SRC  = src/main.cpp src/CloudIndexApp.cpp src/BPlusTree.cpp src/CloudRecord.cpp src/InputUtils.cpp
TEST_SRC = tests/test_bplustree.cpp src/BPlusTree.cpp src/CloudRecord.cpp

all: cloud_index

cloud_index: $(APP_SRC) include/*.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(APP_SRC) -o $@

test_bplustree: $(TEST_SRC) include/*.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_SRC) -o $@

run: cloud_index
	./cloud_index

test: test_bplustree
	./test_bplustree

demo: cloud_index
	./cloud_index < docs/demo-input.txt

clean:
	rm -f cloud_index test_bplustree

.PHONY: all run test demo clean
