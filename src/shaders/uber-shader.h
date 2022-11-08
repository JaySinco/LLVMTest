#pragma once
#include "./base-shader.h"

class UberShader: public BaseShader
{
public:
    UberShader();
    ~UberShader() override;
    void render(nlohmann::json const& j) override;

private:
    void loadManifest(nlohmann::json const& j);

    Shader shader_;
    Texture texture_;
    Model model_;
};
