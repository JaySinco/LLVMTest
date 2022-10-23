#include "utils/base.h"
#include <raylib.h>
#include <rlgl.h>
#include <glad.h>
#include <argparse/argparse.hpp>
#include <ctime>

class ShaderToy
{
public:
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

    ShaderToy(int screenWidth, int screenHeight, std::filesystem::path const& fragShader);
    ~ShaderToy();
    void set_channel_texture(ChannelIndex idx, std::filesystem::path const& texture);
    void set_channel_shader(ChannelIndex idx, std::filesystem::path const& fragShader);
    void render();

private:
    void update_uniform();
    void get_location(Shader shader);
    void bind_shader_uniform(Shader shader);
    void draw_rect(Shader shader);

    std::filesystem::path const vertexShader;
    std::map<decltype(std::declval<Shader>().id), std::map<std::string, int>> shaderLoc;
    int screenWidth;
    int screenHeight;
    Shader mainShader;

    float iResolution[3];
    float iTime;
    float iTimeDelta;
    int iFrame;
    float iMouse[4];
    float iDate[4];
    Channel iChannel[CHANNEL_MAX];
};

ShaderToy::ShaderToy(int screenWidth, int screenHeight, std::filesystem::path const& fragShader)
    : vertexShader{__DIRNAME__ / "identity.vs"}, iMouse{0}, iDate{0}
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    mainShader = LoadShader(vertexShader.string().c_str(), fragShader.string().c_str());
    get_location(mainShader);
    iResolution[0] = screenWidth;
    iResolution[1] = screenHeight;
    iResolution[2] = 1.0;
    iTime = 0;
    iTimeDelta = 0;
    iFrame = 0;
    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        iChannel[i].type = CHANNEL_UNUSED;
    }
}

ShaderToy::~ShaderToy()
{
    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        switch (iChannel[i].type) {
            case CHANNEL_UNUSED:
                break;
            case CHANNEL_TEXTURE:
                UnloadTexture(iChannel[i].text);
                break;
            case CHANNEL_SHADER:
                UnloadRenderTexture(iChannel[i].buffer);
                UnloadRenderTexture(iChannel[i].bufferPrev);
                UnloadShader(iChannel[i].shader);
                break;
        }
    }
    UnloadShader(mainShader);
}

RenderTexture2D load_buffer_texture(int width, int height)
{
    RenderTexture2D target = {0};

    target.id = rlLoadFramebuffer(width, height);  // Load an empty framebuffer

    if (target.id > 0) {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id =
            rlLoadTexture(NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;  // DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0,
                            RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH,
                            RL_ATTACHMENT_RENDERBUFFER, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) {
            spdlog::info("FBO: [ID {}] Framebuffer object created successfully", target.id);
        }
        rlDisableFramebuffer();
    } else {
        spdlog::error("FBO: Framebuffer object can not be created");
    }

    return target;
}

void ShaderToy::set_channel_texture(ChannelIndex idx, std::filesystem::path const& texture)
{
    iChannel[idx].type = CHANNEL_TEXTURE;
    iChannel[idx].text = LoadTexture(texture.string().c_str());
}

void ShaderToy::set_channel_shader(ChannelIndex idx, std::filesystem::path const& fragShader)
{
    iChannel[idx].type = CHANNEL_SHADER;
    iChannel[idx].buffer = load_buffer_texture(screenWidth, screenHeight);
    BeginTextureMode(iChannel[idx].buffer);
    ClearBackground(BLACK);
    EndTextureMode();
    iChannel[idx].bufferPrev = load_buffer_texture(screenWidth, screenHeight);
    iChannel[idx].shader = LoadShader(vertexShader.string().c_str(), fragShader.string().c_str());
    get_location(iChannel[idx].shader);
}

void ShaderToy::get_location(Shader shader)
{
    shaderLoc[shader.id]["iResolution"] = GetShaderLocation(shader, "iResolution");
    shaderLoc[shader.id]["iTime"] = GetShaderLocation(shader, "iTime");
    shaderLoc[shader.id]["iTimeDelta"] = GetShaderLocation(shader, "iTimeDelta");
    shaderLoc[shader.id]["iFrame"] = GetShaderLocation(shader, "iFrame");
    shaderLoc[shader.id]["iMouse"] = GetShaderLocation(shader, "iMouse");
    shaderLoc[shader.id]["iDate"] = GetShaderLocation(shader, "iDate");
    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        std::string s = fmt::format("iChannel{}", i);
        shaderLoc[shader.id][s] = GetShaderLocation(shader, s.c_str());
    }
}

void ShaderToy::update_uniform()
{
    iTimeDelta = GetFrameTime();
    iTime += iTimeDelta;
    ++iFrame;

    auto mpos = GetMousePosition();
    mpos.y = screenHeight - mpos.y;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        iMouse[2] = mpos.x;
        iMouse[3] = mpos.y;
    } else {
        iMouse[3] = -1 * std::abs(iMouse[3]);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        iMouse[0] = mpos.x;
        iMouse[1] = mpos.y;
        iMouse[2] = std::abs(iMouse[2]);
    } else {
        iMouse[2] = -1 * std::abs(iMouse[2]);
    }

    time_t now = std::time(0);
    tm gt{0};
    localtime_s(&gt, &now);
    iDate[0] = gt.tm_year;
    iDate[1] = gt.tm_mon;
    iDate[2] = gt.tm_mday;
    iDate[3] = gt.tm_hour * 3600 + gt.tm_min * 60 + gt.tm_sec;
}

void ShaderToy::bind_shader_uniform(Shader shader)
{
    SetShaderValue(shader, shaderLoc[shader.id]["iResolution"], iResolution, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, shaderLoc[shader.id]["iTime"], &iTime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, shaderLoc[shader.id]["iTimeDelta"], &iTimeDelta, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, shaderLoc[shader.id]["iFrame"], &iFrame, SHADER_UNIFORM_INT);
    SetShaderValue(shader, shaderLoc[shader.id]["iMouse"], iMouse, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, shaderLoc[shader.id]["iDate"], iDate, SHADER_UNIFORM_VEC4);

    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        std::string s = fmt::format("iChannel{}", i);
        switch (iChannel[i].type) {
            case CHANNEL_UNUSED:
                break;
            case CHANNEL_TEXTURE:
                SetShaderValueTexture(shader, shaderLoc[shader.id][s], iChannel[i].text);
                break;
            case CHANNEL_SHADER:
                SetShaderValueTexture(shader, shaderLoc[shader.id][s],
                                      iChannel[i].bufferPrev.texture);
                break;
        }
    }
}

void ShaderToy::draw_rect(Shader shader)
{
    BeginShaderMode(shader);
    bind_shader_uniform(shader);
    DrawTriangle3D(Vector3{-1, -1, 1}, Vector3{1, 1, 1}, Vector3{-1, 1, 1}, WHITE);
    DrawTriangle3D(Vector3{1, 1, 1}, Vector3{-1, -1, 1}, Vector3{1, -1, 1}, WHITE);
    EndShaderMode();
}

void ShaderToy::render()
{
    update_uniform();

    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        if (iChannel[i].type == CHANNEL_SHADER) {
            BeginTextureMode(iChannel[i].buffer);
            draw_rect(iChannel[i].shader);
            EndTextureMode();
            glCopyImageSubData(iChannel[i].buffer.texture.id, GL_TEXTURE_2D, 0, 0, 0, 0,
                               iChannel[i].bufferPrev.texture.id, GL_TEXTURE_2D, 0, 0, 0, 0,
                               screenWidth, screenHeight, 1);
        }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    draw_rect(mainShader);
    DrawFPS(10, 10);
    EndDrawing();
}

int main(int argc, char** argv)
{
    argparse::ArgumentParser prog("pixel-shader");
    prog.add_argument("-f", "--fragment-shader")
        .default_value(std::string("testing"))
        .required()
        .help("fragment shader name");
    prog.add_argument("-cs0", "--channel-0-shader")
        .default_value(std::string(""))
        .required()
        .help("channel-0 shader name");
    prog.add_argument("-ct0", "--channel-0-texture")
        .default_value(std::string(""))
        .required()
        .help("channel-0 texture name");

    try {
        prog.parse_args(argc, argv);
    } catch (std::exception const& err) {
        spdlog::error("{}\n", err.what());
        std::cerr << prog;
        std::exit(1);
    }

    int const screenWidth = 640;
    int const screenHeight = 360;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "pixel shader");

    auto fragShader =
        __DIRNAME__ / fmt::format("{}.fs", prog.get<std::string>("--fragment-shader"));

    auto cs0 = prog.get<std::string>("--channel-0-shader");
    auto ct0 = prog.get<std::string>("--channel-0-texture");
    auto sha = std::make_unique<ShaderToy>(screenWidth, screenHeight, fragShader);
    if (cs0.size() > 0) {
        sha->set_channel_shader(ShaderToy::CHANNEL_0, __DIRNAME__ / fmt::format("{}.fs", cs0));

    } else if (ct0.size() > 0) {
        sha->set_channel_texture(ShaderToy::CHANNEL_0,
                                 utils::source_repo / fmt::format("models/{}_diffuse.png", ct0));
    }

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        sha->render();
    }
    CloseWindow();
    return 0;
}
