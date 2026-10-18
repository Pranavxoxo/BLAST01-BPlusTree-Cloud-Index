CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2
INCLUDES  = -Iinclude

APP_SRC  = src/main.cpp src/CloudIndexApp.cpp src/BPlusTree.cpp src/CloudRecord.cpp src/InputUtils.cpp

all: cloud_index

cloud_index: $(APP_SRC) include/*.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(APP_SRC) -o $@

run: cloud_index
	./cloud_index

clean:
	rm -f cloud_index

.PHONY: all run clean
