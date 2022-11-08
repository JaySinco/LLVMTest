#pragma once
#include "./base-shader.h"

class PixelShader: public BaseShader
{
public:
    PixelShader();
    ~PixelShader() override;

    void render(nlohmann::json const& j) override;

private:
    enum ChannelIndex
    {
        CHANNEL_0,
        CHANNEL_1,
        CHANNEL_2,
        CHANNEL_3,
        CHANNEL_MAX,
    };

    enum ChannelType
    {
        CHANNEL_UNUSED,
        CHANNEL_TEXTURE,
        CHANNEL_SHADER,
    };

    struct Channel
    {
        ChannelType type;
        Texture text;
        std::string textType;
        RenderTexture2D buffer;
        RenderTexture2D bufferPrev;
        std::string filter;
        std::string wrap;
        Shader shader;
    };

    void loadManifest(nlohmann::json const& j);
    void setChannelTexture(ChannelIndex idx, std::string const& texture, std::string const& type,
                           std::string const& filter, std::string const& wrap);
    void setChannelShader(ChannelIndex idx, std::string const& vertex, std::string const& fragment,
                          std::string const& filter, std::string const& wrap);
    void updateUniform();
    void getLocation(Shader shader);
    void bindShaderUniform(Shader shader);
    void drawRect(Shader shader);

    float iResolution_[3];
    float iTime_;
    float iTimeDelta_;
    int iFrame_;
    float iMouse_[4];
    float iDate_[4];
    Channel iChannel_[CHANNEL_MAX];

    std::map<decltype(std::declval<Shader>().id), std::map<std::string, int>> shaderLoc_;
    Shader mainShader_;
};
