#pragma once
#include "./base-shader.h"

class PixelShader: public BaseShader
{
public:
    PixelShader();
    ~PixelShader();

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
        RenderTexture2D buffer;
        RenderTexture2D bufferPrev;
        Shader shader;
    };

    void load_manifest(nlohmann::json const& j);
    void set_channel_texture(ChannelIndex idx, std::string const& texture, bool cube);
    void set_channel_shader(ChannelIndex idx, std::string const& vertex,
                            std::string const& fragment);
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
