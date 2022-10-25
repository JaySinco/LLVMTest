#include "base-shader.h"

BaseShader::~BaseShader() {}

void BaseShader::load_manifest(nlohmann::json const& j)
{
    if (j.contains("screenWidth")) {
        screenWidth = j["screenWidth"];
    }
    if (j.contains("screenHeight")) {
        screenHeight = j["screenHeight"];
    }
}

Shader BaseShader::load_shader(std::string const& vertex, std::string const& fragment)
{
    return LoadShader((__DIRNAME__ / vertex).string().c_str(),
                      (__DIRNAME__ / fragment).string().c_str());
}

Texture BaseShader::load_texture(std::string const& texture, std::string const& type)
{
    auto abspath = (utils::source_repo / fmt::format("models/{}", texture)).string();
    if (type == "2d") {
        return LoadTexture(abspath.c_str());
    } else if (type == "cube") {
        Image image = LoadImage(abspath.c_str());
        return LoadTextureCubemap(image, CUBEMAP_LAYOUT_AUTO_DETECT);
    } else {
        throw std::runtime_error(fmt::format("texture type not support: {}", type));
    }
}

Model BaseShader::load_model(std::string const& model)
{
    return LoadModel((utils::source_repo / fmt::format("models/{}", model)).string().c_str());
}