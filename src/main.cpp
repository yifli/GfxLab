#include "jsonparser.h"
#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cout << "Example Usage: gfxlab input.json" << std::endl;
		return -1;
	}

	SceneParser parser;
	parser.Parse(argv[1]);

	return 0;
}