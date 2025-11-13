#include <LexviEngine.hpp>
#include <iostream>

#include "Application.hpp"

#ifndef _DEBUG
#include <fstream>
#endif

int main() {
#ifndef _DEBUG
	std::ofstream logFile("log.txt");
	auto coutBuf = std::cout.rdbuf(logFile.rdbuf());
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

#ifndef _DEBUG
#if _WIN32

#include <Windows.h>

int APIENTRY WinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPSTR     lpCmdLine,
	_In_     int       nCmdShow
)
{
	return main();
}


#endif // _WIN32
#endif // !_DEBUG