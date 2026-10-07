#include "Net.h"
#include <iostream>

int main()
{
	Net net;
	if (net.Init() == false)
	{
		std::cout << "Error. net.init fail" << std::endl;
		return -1;
	}
	while (true)
	{
		net.Tick();
	}


	return 0;
}
