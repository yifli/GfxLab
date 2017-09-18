#include "rendererfactory.h"

std::unordered_map<std::string, std::function<RendererPtr()>> RendererFactory::renderers = {};
