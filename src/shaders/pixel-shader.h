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
        std::string text_type;
        RenderTexture2D buffer;
        RenderTexture2D buffer_prev;
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

    float resolution_[3];
    float time_;
    float time_delta_;
    int frame_;
    float mouse_[4];
    float date_[4];
    Channel channel_[CHANNEL_MAX];

    std::map<decltype(std::declval<Shader>().id), std::map<std::string, int>> shader_loc_;
    Shader main_shader_;
};
