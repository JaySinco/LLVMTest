#include "utils/base.h"
#include <raylib.h>
#include <argparse/argparse.hpp>

int main(int argc, char** argv)
{
    argparse::ArgumentParser prog("pixel-shader");
    prog.add_argument("-f", "--fragment-shader")
        .default_value(std::string("sphere"))
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

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
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
