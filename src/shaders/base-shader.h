#pragma once
#include "utils/base.h"
#include <raylib.h>
#include <nlohmann/json.hpp>

class BaseShader
{
public:
    virtual ~BaseShader();
    virtual void render(nlohmann::json const& j) = 0;

protected:
    void load_manifest(nlohmann::json const& j);
    static Shader load_shader(std::string const& vertex, std::string const& fragment);
    static Texture load_texture(std::string const& texture, std::string const& type,
                                std::string const& filter, std::string const& wrap);
    static Model load_model(std::string const& model);
    static RenderTexture2D load_buffer_texture(int width, int height, std::string const& filter,
                                               std::string const& wrap);

    int screenWidth;
    int screenHeight;
};
