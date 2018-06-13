#include "server.h"

int main(int argc, char* argv[])
{
	server::instance().start();
	return 0;
}