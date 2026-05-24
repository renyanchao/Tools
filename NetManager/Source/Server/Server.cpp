#include "Select.h"
#include <iostream>

int main()
{
	Select selector;
	if (selector.Init() == false)
	{
		std::cout << "Error. selector.init fail" << std::endl;
		return -1;
	}
	while (true)
	{
		selector.Tick();
	}


	return 0;
}