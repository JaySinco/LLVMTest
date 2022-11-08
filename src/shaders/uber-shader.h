#pragma once
#include "./base-shader.h"

class UberShader: public BaseShader
{
public:
    UberShader();
    ~UberShader() override;
    void render(nlohmann::json const& j) override;

private:
    void load_manifest(nlohmann::json const& j);

    Shader shader;
    Texture texture;
    Model model;
};
