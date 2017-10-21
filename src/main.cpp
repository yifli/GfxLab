#include "jsonparser.h"
#include "rendererfactory.h"
#include "renderpass.h"
#include "window.h"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Example Usage: gfxlab input.json" << std::endl;
        return -1;
    }

    RendererFactory::Init();
    SceneParser parser;
    auto window = parser.Parse(argv[1]);
    window->Display();
    return 0;
}