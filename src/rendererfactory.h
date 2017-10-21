#pragma once

#include "common.h"
#include "renderer.h"

class RendererFactory {
    
public:
    static void Init() {
        static bool initialized = false;
        if (!initialized) {
            RegisterRenderer<Renderer>("default");
        }
    }

    template<typename T>
    static void RegisterRenderer(const std::string& name) {
        renderers[name] = []() { return std::make_shared<T>(); };
    }

    static RendererPtr MakeRenderer(const std::string& name) {
        return renderers[name]();
    }

    static std::unordered_map<std::string, std::function<RendererPtr()>> renderers;
};