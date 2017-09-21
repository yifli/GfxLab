#include "rendererfactory.h"
#include "renderpass.h"

std::unordered_map<std::string, std::function<RendererPtr()>> RendererFactory::renderers = {};
