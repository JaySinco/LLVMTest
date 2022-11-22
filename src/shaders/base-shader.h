#pragma once
#include "utils/logging.h"
#include <raylib.h>
#include <nlohmann/json.hpp>

class BaseShader
{
public:
    virtual ~BaseShader();
    virtual void render(nlohmann::json const& j) = 0;

protected:
    void loadManifest(nlohmann::json const& j);
    static Shader loadShader(std::string const& vertex, std::string const& fragment);
    static Texture loadTexture(std::string const& texture, std::string const& type,
                               std::string const& filter, std::string const& wrap);
    static Model loadModel(std::string const& model);
    static RenderTexture2D loadBufferTexture(int width, int height, std::string const& filter,
                                             std::string const& wrap);

    int screen_width;
    int screen_height;
};
