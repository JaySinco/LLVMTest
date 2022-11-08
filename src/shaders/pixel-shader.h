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

    void load_manifest(nlohmann::json const& j);
    void set_channel_texture(ChannelIndex idx, std::string const& texture, std::string const& type,
                             std::string const& filter, std::string const& wrap);
    void set_channel_shader(ChannelIndex idx, std::string const& vertex,
                            std::string const& fragment, std::string const& filter,
                            std::string const& wrap);
    void update_uniform();
    void get_location(Shader shader);
    void bind_shader_uniform(Shader shader);
    void draw_rect(Shader shader);

    float iResolution[3];
    float iTime;
    float iTimeDelta;
    int iFrame;
    float iMouse[4];
    float iDate[4];
    Channel iChannel[CHANNEL_MAX];

    std::map<decltype(std::declval<Shader>().id), std::map<std::string, int>> shaderLoc;
    Shader mainShader;
};
