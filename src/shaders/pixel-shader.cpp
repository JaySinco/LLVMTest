#include "./pixel-shader.h"
#include <rlgl.h>
#include <glad.h>
#include <ctime>

PixelShader::PixelShader() = default;

PixelShader::~PixelShader()
{
    for (int i = kChannel0; i < kChannelMax; ++i) {
        switch (channel_[i].type) {
            case kChannelUnused:
                break;
            case kChannelTexture:
                UnloadTexture(channel_[i].text);
                break;
            case kChannelShader:
                UnloadRenderTexture(channel_[i].buffer);
                UnloadRenderTexture(channel_[i].buffer_prev);
                UnloadShader(channel_[i].shader);
                break;
        }
    }
    UnloadShader(main_shader_);
}

void PixelShader::setChannelTexture(ChannelIndex idx, std::string const& texture,
                                    std::string const& type, std::string const& filter,
                                    std::string const& wrap)
{
    channel_[idx].type = kChannelTexture;
    channel_[idx].text = loadTexture(texture, type, filter, wrap);
    channel_[idx].text_type = type;
    channel_[idx].filter = filter;
    channel_[idx].wrap = wrap;
}

void PixelShader::setChannelShader(ChannelIndex idx, std::string const& vertex,
                                   std::string const& fragment, std::string const& filter,
                                   std::string const& wrap)
{
    channel_[idx].type = kChannelShader;
    channel_[idx].buffer = loadBufferTexture(screen_width, screen_height, filter, wrap);
    channel_[idx].buffer_prev = loadBufferTexture(screen_width, screen_height, filter, wrap);
    channel_[idx].shader = loadShader(vertex, fragment);
    channel_[idx].filter = filter;
    channel_[idx].wrap = wrap;
    getLocation(channel_[idx].shader);
}

void PixelShader::getLocation(Shader shader)
{
    shader_loc_[shader.id]["iResolution"] = GetShaderLocation(shader, "iResolution");
    shader_loc_[shader.id]["iTime"] = GetShaderLocation(shader, "iTime");
    shader_loc_[shader.id]["iTimeDelta"] = GetShaderLocation(shader, "iTimeDelta");
    shader_loc_[shader.id]["iFrame"] = GetShaderLocation(shader, "iFrame");
    shader_loc_[shader.id]["iMouse"] = GetShaderLocation(shader, "iMouse");
    shader_loc_[shader.id]["iDate"] = GetShaderLocation(shader, "iDate");
    for (int i = kChannel0; i < kChannelMax; ++i) {
        std::string s = FSTR("iChannel{}", i);
        shader_loc_[shader.id][s] = GetShaderLocation(shader, s.c_str());
    }
}

void PixelShader::updateUniform()
{
    time_delta_ = GetFrameTime();
    time_ += time_delta_;
    ++frame_;

    auto mpos = GetMousePosition();
    mpos.y = screen_height - mpos.y;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        mouse_[2] = mpos.x;
        mouse_[3] = mpos.y;
    } else {
        mouse_[3] = -1 * std::abs(mouse_[3]);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        mouse_[0] = mpos.x;
        mouse_[1] = mpos.y;
        mouse_[2] = std::abs(mouse_[2]);
    } else {
        mouse_[2] = -1 * std::abs(mouse_[2]);
    }

    time_t now = std::time(nullptr);
    tm gt{0};
#ifdef __linux__
    localtime_r(&now, &gt);
#elif _WIN32
    localtime_s(&gt, &now);
#endif
    date_[0] = gt.tm_year;
    date_[1] = gt.tm_mon;
    date_[2] = gt.tm_mday;
    date_[3] = gt.tm_hour * 3600 + gt.tm_min * 60 + gt.tm_sec;
}

void PixelShader::bindShaderUniform(Shader shader)
{
    SetShaderValue(shader, shader_loc_[shader.id]["iResolution"], resolution_, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, shader_loc_[shader.id]["iTime"], &time_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, shader_loc_[shader.id]["iTimeDelta"], &time_delta_,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, shader_loc_[shader.id]["iFrame"], &frame_, SHADER_UNIFORM_INT);
    SetShaderValue(shader, shader_loc_[shader.id]["iMouse"], mouse_, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, shader_loc_[shader.id]["iDate"], date_, SHADER_UNIFORM_VEC4);

    for (int i = kChannel0; i < kChannelMax; ++i) {
        std::string s = FSTR("iChannel{}", i);
        switch (channel_[i].type) {
            case kChannelUnused:
                break;
            case kChannelTexture: {
                if (channel_[i].text_type == "2d") {
                    SetShaderValueTexture(shader, shader_loc_[shader.id][s], channel_[i].text);
                } else if (channel_[i].text_type == "cube") {
                    int sampler = 0;
                    SetShaderValue(shader, shader_loc_[shader.id][s], &sampler, SHADER_UNIFORM_INT);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, channel_[i].text.id);
                }
                break;
            }
            case kChannelShader:
                SetShaderValueTexture(shader, shader_loc_[shader.id][s],
                                      channel_[i].buffer_prev.texture);
                break;
        }
    }
}

void PixelShader::drawRect(Shader shader)
{
    BeginShaderMode(shader);
    bindShaderUniform(shader);
    DrawTriangle3D(Vector3{-1, -1, 1}, Vector3{1, 1, 1}, Vector3{-1, 1, 1}, WHITE);
    DrawTriangle3D(Vector3{1, 1, 1}, Vector3{-1, -1, 1}, Vector3{1, -1, 1}, WHITE);
    EndShaderMode();
}

void PixelShader::loadManifest(nlohmann::json const& j)
{
    resolution_[0] = screen_width;
    resolution_[1] = screen_height;
    resolution_[2] = 1.0;
    time_ = 0;
    time_delta_ = 0;
    frame_ = 0;
    std::memset(date_, 0, sizeof(date_));
    std::memset(mouse_, 0, sizeof(mouse_));
    for (int i = kChannel0; i < kChannelMax; ++i) {
        std::string k = FSTR("channel{}", i);
        if (!j.contains(k)) {
            channel_[i].type = kChannelUnused;
        } else {
            auto& ch = j[k];
            if (ch["type"] == "shader") {
                setChannelShader(static_cast<ChannelIndex>(i), ch["vertexShader"],
                                 ch["fragmentShader"], ch["filter"], ch["wrap"]);
            } else if (ch["type"] == "texture") {
                auto& text = ch["texture"];
                setChannelTexture(static_cast<ChannelIndex>(i), text["file"], text["type"],
                                  text["filter"], text["wrap"]);
            }
        }
    }
    main_shader_ = loadShader(j["vertexShader"], j["fragmentShader"]);
    getLocation(main_shader_);
    int fps = j["targetFPS"];
    if (fps > 0) {
        SetTargetFPS(fps);
    }
}

void PixelShader::render(nlohmann::json const& j)
{
    BaseShader::loadManifest(j);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screen_width, screen_height, "Pixel Shader");

    PixelShader::loadManifest(j);

    while (!WindowShouldClose()) {
        updateUniform();
        for (int i = kChannel0; i < kChannelMax; ++i) {
            if (channel_[i].type == kChannelShader) {
                BeginTextureMode(channel_[i].buffer);
                glDisable(GL_BLEND);
                drawRect(channel_[i].shader);
                glEnable(GL_BLEND);
                EndTextureMode();
                glCopyImageSubData(channel_[i].buffer.texture.id, GL_TEXTURE_2D, 0, 0, 0, 0,
                                   channel_[i].buffer_prev.texture.id, GL_TEXTURE_2D, 0, 0, 0, 0,
                                   screen_width, screen_height, 1);
                if (channel_[i].filter == "mipmap") {
                    glBindTexture(GL_TEXTURE_2D, channel_[i].buffer_prev.texture.id);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawRect(main_shader_);
        DrawFPS(10, 10);
        DrawText(TextFormat("%8i", frame_), screen_width - 65, screen_height - 20, 18, GRAY);
        EndDrawing();
    }

    CloseWindow();
}
