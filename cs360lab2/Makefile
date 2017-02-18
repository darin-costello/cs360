all: download
download: server.cpp
	g++ -o server server.cpp
clean:
	rm server
debug:
	g++ -g -o server server.cpp
	gdb ./server
