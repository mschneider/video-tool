all:
	g++ --std=c++11 `pkg-config --cflags opencv` main.cpp `pkg-config --libs opencv` -o main
