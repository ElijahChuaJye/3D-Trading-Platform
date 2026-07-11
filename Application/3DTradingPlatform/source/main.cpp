#include <iostream>
#include "core/Application.h"

int main() {
	std::cout << "3D Trading platform" << std::endl;

	Application app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		//If Application throws std::runtime anywhere
		std::cerr << "ENGINE FAILURE: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}



	return EXIT_SUCCESS;
}