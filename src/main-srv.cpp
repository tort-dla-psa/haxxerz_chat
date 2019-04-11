#include <iostream>

#include <server.h>

int main(int argc, char* argv[]){
	server srv(10);
	srv.init(atoi(argv[1]));
	srv.work();
	return 0;
}
