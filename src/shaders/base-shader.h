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
    Shader load_shader(std::string const& vertex, std::string const& fragment);
    Texture load_texture(std::string const& texture);
    Model load_model(std::string const& model);

    int screenWidth;
    int screenHeight;
};
