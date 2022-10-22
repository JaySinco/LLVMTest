#include "utils/base.h"
#include <raylib.h>
#include <argparse/argparse.hpp>
#include <ctime>

int main(int argc, char** argv)
{
    argparse::ArgumentParser prog("pixel-shader");
    prog.add_argument("-f", "--fragment-shader")
        .default_value(std::string("testing"))
        .required()
        .help("fragment shader name");

    try {
        prog.parse_args(argc, argv);
    } catch (std::exception const& err) {
        spdlog::error("{}\n", err.what());
        std::cerr << prog;
        std::exit(1);
    }

    int const screenWidth = 800;
    int const screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "pixel shader");

    auto vertexShaderPath = __DIRNAME__ / "identity.vs";
    auto fragShaderPath =
        __DIRNAME__ / fmt::format("{}.fs", prog.get<std::string>("--fragment-shader"));
    Shader shader = LoadShader(vertexShaderPath.string().c_str(), fragShaderPath.string().c_str());

    int iResolutionLoc = GetShaderLocation(shader, "iResolution");
    int iTimeLoc = GetShaderLocation(shader, "iTime");
    int iTimeDeltaLoc = GetShaderLocation(shader, "iTimeDelta");
    int iFrameLoc = GetShaderLocation(shader, "iFrame");
    int iMouseLoc = GetShaderLocation(shader, "iMouse");
    int iDateLoc = GetShaderLocation(shader, "iDate");

    int frameCount = 0;
    float runTime = 0.0f;
    float resolution[3] = {screenWidth, screenHeight, 1.0};
    float mousePos[4] = {0};

    SetShaderValue(shader, iResolutionLoc, resolution, SHADER_UNIFORM_VEC3);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        runTime += deltaTime;
        ++frameCount;

        auto mpos = GetMousePosition();
        mpos.y = screenHeight - mpos.y;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mousePos[2] = mpos.x;
            mousePos[3] = mpos.y;
        } else {
            mousePos[3] = -1 * std::abs(mousePos[3]);
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            mousePos[0] = mpos.x;
            mousePos[1] = mpos.y;
            mousePos[2] = std::abs(mousePos[2]);
        } else {
            mousePos[2] = -1 * std::abs(mousePos[2]);
        }

        time_t now = std::time(0);
        tm gt{0};
        localtime_s(&gt, &now);
        int sec = gt.tm_hour * 3600 + gt.tm_min * 60 + gt.tm_sec;
        float dateTime[4] = {(float)gt.tm_year, (float)gt.tm_mon, (float)gt.tm_mday, (float)sec};

        SetShaderValue(shader, iTimeLoc, &runTime, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, iTimeDeltaLoc, &deltaTime, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, iFrameLoc, &frameCount, SHADER_UNIFORM_INT);
        SetShaderValue(shader, iMouseLoc, mousePos, SHADER_UNIFORM_VEC4);
        SetShaderValue(shader, iDateLoc, dateTime, SHADER_UNIFORM_VEC4);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginShaderMode(shader);
        DrawTriangle3D(Vector3{-1, -1, 1}, Vector3{1, 1, 1}, Vector3{-1, 1, 1}, WHITE);
        DrawTriangle3D(Vector3{1, 1, 1}, Vector3{-1, -1, 1}, Vector3{1, -1, 1}, WHITE);
        EndShaderMode();
        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadShader(shader);
    CloseWindow();

    return 0;
}
