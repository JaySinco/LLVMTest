#include "./pixel-shader.h"
#include <rlgl.h>
#include <glad.h>
#include <ctime>

PixelShader::PixelShader() {}

PixelShader::~PixelShader()
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

static RenderTexture2D load_buffer_texture(int width, int height)
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

void PixelShader::set_channel_texture(ChannelIndex idx, std::string const& texture,
                                      std::string const& type)
{
    iChannel[idx].type = CHANNEL_TEXTURE;
    iChannel[idx].text = load_texture(texture, type);
    iChannel[idx].textType = type;
}

void PixelShader::set_channel_shader(ChannelIndex idx, std::string const& vertex,
                                     std::string const& fragment)
{
    iChannel[idx].type = CHANNEL_SHADER;
    iChannel[idx].buffer = load_buffer_texture(screenWidth, screenHeight);
    iChannel[idx].bufferPrev = load_buffer_texture(screenWidth, screenHeight);
    iChannel[idx].shader = load_shader(vertex, fragment);
    get_location(iChannel[idx].shader);
    // glBindTexture(GL_TEXTURE_2D, iChannel[idx].bufferPrev.texture.id);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    // glGenerateMipmap(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, 0);
}

void PixelShader::get_location(Shader shader)
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

void PixelShader::update_uniform()
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

void PixelShader::bind_shader_uniform(Shader shader)
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
            case CHANNEL_TEXTURE: {
                if (iChannel[i].textType == "2d") {
                    SetShaderValueTexture(shader, shaderLoc[shader.id][s], iChannel[i].text);
                } else if (iChannel[i].textType == "cube") {
                    int sampler = 0;
                    SetShaderValue(shader, shaderLoc[shader.id][s], &sampler, SHADER_UNIFORM_INT);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, iChannel[i].text.id);
                }
                break;
            }
            case CHANNEL_SHADER:
                SetShaderValueTexture(shader, shaderLoc[shader.id][s],
                                      iChannel[i].bufferPrev.texture);
                break;
        }
    }
}

void PixelShader::draw_rect(Shader shader)
{
    BeginShaderMode(shader);
    bind_shader_uniform(shader);
    DrawTriangle3D(Vector3{-1, -1, 1}, Vector3{1, 1, 1}, Vector3{-1, 1, 1}, WHITE);
    DrawTriangle3D(Vector3{1, 1, 1}, Vector3{-1, -1, 1}, Vector3{1, -1, 1}, WHITE);
    EndShaderMode();
}

void PixelShader::load_manifest(nlohmann::json const& j)
{
    iResolution[0] = screenWidth;
    iResolution[1] = screenHeight;
    iResolution[2] = 1.0;
    iTime = 0;
    iTimeDelta = 0;
    iFrame = 0;
    std::memset(iDate, 0, sizeof(iDate));
    std::memset(iMouse, 0, sizeof(iMouse));
    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        std::string k = fmt::format("channel{}", i);
        if (!j.contains(k)) {
            iChannel[i].type = CHANNEL_UNUSED;
        } else {
            auto& ch = j[k];
            if (ch["type"] == "shader") {
                set_channel_shader(ChannelIndex(i), ch["vertexShader"], ch["fragmentShader"]);
            } else if (ch["type"] == "texture") {
                set_channel_texture(ChannelIndex(i), ch["texture"]["file"], ch["texture"]["type"]);
            }
        }
    }
    mainShader = load_shader(j["vertexShader"], j["fragmentShader"]);
    get_location(mainShader);
}

void PixelShader::render(nlohmann::json const& j)
{
    BaseShader::load_manifest(j);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Pixel Shader");

    SetTargetFPS(60);
    PixelShader::load_manifest(j);

    while (!WindowShouldClose()) {
        update_uniform();
        for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
            if (iChannel[i].type == CHANNEL_SHADER) {
                BeginTextureMode(iChannel[i].buffer);
                glDisable(GL_BLEND);
                draw_rect(iChannel[i].shader);
                glEnable(GL_BLEND);
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
        DrawText(TextFormat("%8i", iFrame), screenWidth - 65, screenHeight - 20, 18, GRAY);
        EndDrawing();
    }

    CloseWindow();
}
