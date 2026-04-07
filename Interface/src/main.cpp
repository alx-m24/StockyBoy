#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <LexviEngine.hpp>

#include <Utils/Logging.hpp>
#include <iostream>


#include "Application.hpp"

#ifndef _DEBUG
#include <fstream>
#endif

int main() {
#ifndef _DEBUG
	Lexvi::RedirectLogToFile("log.txt");
#endif

	Lexvi::Engine engine;
	try {
		engine.Init("Stocky Boy", std::make_unique<Application>(), true, false, 30);

		engine.run();
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
