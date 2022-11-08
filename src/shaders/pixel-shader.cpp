#include "./pixel-shader.h"
#include <rlgl.h>
#include <glad.h>
#include <ctime>

PixelShader::PixelShader() = default;

PixelShader::~PixelShader()
{
    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        switch (iChannel_[i].type) {
            case CHANNEL_UNUSED:
                break;
            case CHANNEL_TEXTURE:
                UnloadTexture(iChannel_[i].text);
                break;
            case CHANNEL_SHADER:
                UnloadRenderTexture(iChannel_[i].buffer);
                UnloadRenderTexture(iChannel_[i].bufferPrev);
                UnloadShader(iChannel_[i].shader);
                break;
        }
    }
    UnloadShader(mainShader_);
}

void PixelShader::setChannelTexture(ChannelIndex idx, std::string const& texture,
                                    std::string const& type, std::string const& filter,
                                    std::string const& wrap)
{
    iChannel_[idx].type = CHANNEL_TEXTURE;
    iChannel_[idx].text = loadTexture(texture, type, filter, wrap);
    iChannel_[idx].textType = type;
    iChannel_[idx].filter = filter;
    iChannel_[idx].wrap = wrap;
}

void PixelShader::setChannelShader(ChannelIndex idx, std::string const& vertex,
                                   std::string const& fragment, std::string const& filter,
                                   std::string const& wrap)
{
    iChannel_[idx].type = CHANNEL_SHADER;
    iChannel_[idx].buffer = loadBufferTexture(screenWidth_, screenHeight_, filter, wrap);
    iChannel_[idx].bufferPrev = loadBufferTexture(screenWidth_, screenHeight_, filter, wrap);
    iChannel_[idx].shader = loadShader(vertex, fragment);
    iChannel_[idx].filter = filter;
    iChannel_[idx].wrap = wrap;
    getLocation(iChannel_[idx].shader);
}

void PixelShader::getLocation(Shader shader)
{
    shaderLoc_[shader.id]["iResolution"] = GetShaderLocation(shader, "iResolution");
    shaderLoc_[shader.id]["iTime"] = GetShaderLocation(shader, "iTime");
    shaderLoc_[shader.id]["iTimeDelta"] = GetShaderLocation(shader, "iTimeDelta");
    shaderLoc_[shader.id]["iFrame"] = GetShaderLocation(shader, "iFrame");
    shaderLoc_[shader.id]["iMouse"] = GetShaderLocation(shader, "iMouse");
    shaderLoc_[shader.id]["iDate"] = GetShaderLocation(shader, "iDate");
    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        std::string s = fmt::format("iChannel{}", i);
        shaderLoc_[shader.id][s] = GetShaderLocation(shader, s.c_str());
    }
}

void PixelShader::updateUniform()
{
    iTimeDelta_ = GetFrameTime();
    iTime_ += iTimeDelta_;
    ++iFrame_;

    auto mpos = GetMousePosition();
    mpos.y = screenHeight_ - mpos.y;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        iMouse_[2] = mpos.x;
        iMouse_[3] = mpos.y;
    } else {
        iMouse_[3] = -1 * std::abs(iMouse_[3]);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        iMouse_[0] = mpos.x;
        iMouse_[1] = mpos.y;
        iMouse_[2] = std::abs(iMouse_[2]);
    } else {
        iMouse_[2] = -1 * std::abs(iMouse_[2]);
    }

    time_t now = std::time(nullptr);
    tm gt{0};
#ifdef __linux__
    localtime_r(&now, &gt);
#elif _WIN32
    localtime_s(&gt, &now);
#endif
    iDate_[0] = gt.tm_year;
    iDate_[1] = gt.tm_mon;
    iDate_[2] = gt.tm_mday;
    iDate_[3] = gt.tm_hour * 3600 + gt.tm_min * 60 + gt.tm_sec;
}

void PixelShader::bindShaderUniform(Shader shader)
{
    SetShaderValue(shader, shaderLoc_[shader.id]["iResolution"], iResolution_, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, shaderLoc_[shader.id]["iTime"], &iTime_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, shaderLoc_[shader.id]["iTimeDelta"], &iTimeDelta_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, shaderLoc_[shader.id]["iFrame"], &iFrame_, SHADER_UNIFORM_INT);
    SetShaderValue(shader, shaderLoc_[shader.id]["iMouse"], iMouse_, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, shaderLoc_[shader.id]["iDate"], iDate_, SHADER_UNIFORM_VEC4);

    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        std::string s = fmt::format("iChannel{}", i);
        switch (iChannel_[i].type) {
            case CHANNEL_UNUSED:
                break;
            case CHANNEL_TEXTURE: {
                if (iChannel_[i].textType == "2d") {
                    SetShaderValueTexture(shader, shaderLoc_[shader.id][s], iChannel_[i].text);
                } else if (iChannel_[i].textType == "cube") {
                    int sampler = 0;
                    SetShaderValue(shader, shaderLoc_[shader.id][s], &sampler, SHADER_UNIFORM_INT);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, iChannel_[i].text.id);
                }
                break;
            }
            case CHANNEL_SHADER:
                SetShaderValueTexture(shader, shaderLoc_[shader.id][s],
                                      iChannel_[i].bufferPrev.texture);
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
    iResolution_[0] = screenWidth_;
    iResolution_[1] = screenHeight_;
    iResolution_[2] = 1.0;
    iTime_ = 0;
    iTimeDelta_ = 0;
    iFrame_ = 0;
    std::memset(iDate_, 0, sizeof(iDate_));
    std::memset(iMouse_, 0, sizeof(iMouse_));
    for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
        std::string k = fmt::format("channel{}", i);
        if (!j.contains(k)) {
            iChannel_[i].type = CHANNEL_UNUSED;
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
    mainShader_ = loadShader(j["vertexShader"], j["fragmentShader"]);
    getLocation(mainShader_);
    int fps = j["targetFPS"];
    if (fps > 0) {
        SetTargetFPS(fps);
    }
}

void PixelShader::render(nlohmann::json const& j)
{
    BaseShader::loadManifest(j);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth_, screenHeight_, "Pixel Shader");

    PixelShader::loadManifest(j);

    while (!WindowShouldClose()) {
        updateUniform();
        for (int i = CHANNEL_0; i < CHANNEL_MAX; ++i) {
            if (iChannel_[i].type == CHANNEL_SHADER) {
                BeginTextureMode(iChannel_[i].buffer);
                glDisable(GL_BLEND);
                drawRect(iChannel_[i].shader);
                glEnable(GL_BLEND);
                EndTextureMode();
                glCopyImageSubData(iChannel_[i].buffer.texture.id, GL_TEXTURE_2D, 0, 0, 0, 0,
                                   iChannel_[i].bufferPrev.texture.id, GL_TEXTURE_2D, 0, 0, 0, 0,
                                   screenWidth_, screenHeight_, 1);
                if (iChannel_[i].filter == "mipmap") {
                    glBindTexture(GL_TEXTURE_2D, iChannel_[i].bufferPrev.texture.id);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawRect(mainShader_);
        DrawFPS(10, 10);
        DrawText(TextFormat("%8i", iFrame_), screenWidth_ - 65, screenHeight_ - 20, 18, GRAY);
        EndDrawing();
    }

    CloseWindow();
}
