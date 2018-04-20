TARGET = run-lua

default: test

build:
	cc -O3 -o $(TARGET) main.cpp -L/usr/local/lib -llua -lm -ldl -I/usr/local/include

test: build
	./$(TARGET) test.lua
