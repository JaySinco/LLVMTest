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

Texture BaseShader::load_texture(std::string const& texture)
{
    return LoadTexture((utils::source_repo / fmt::format("models/{}", texture)).string().c_str());
}

Model BaseShader::load_model(std::string const& model)
{
    return LoadModel((utils::source_repo / fmt::format("models/{}", model)).string().c_str());
}