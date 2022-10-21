#include "utils/base.h"
#include <raylib.h>
#include <argparse/argparse.hpp>

int main(int argc, char** argv)
{
    argparse::ArgumentParser prog("uber-shader");
    prog.add_argument("-t", "--vertex-shader")
        .default_value(std::string("mvp"))
        .required()
        .help("vertex shader name");
    prog.add_argument("-f", "--fragment-shader")
        .default_value(std::string("diffuse"))
        .required()
        .help("fragment shader name");
    prog.add_argument("-m", "--model")
        .default_value(std::string("watermill"))
        .required()
        .help("model name");

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
    InitWindow(screenWidth, screenHeight, "uber shader");

    Camera camera = {0};
    camera.position = Vector3{4.0f, 4.0f, 4.0f};
    camera.target = Vector3{0.0f, 1.0f, -1.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    auto modelName = prog.get<std::string>("--model");
    auto modelPath = utils::source_repo / fmt::format("models/{}.obj", modelName);
    Model model = LoadModel(modelPath.string().c_str());
    auto texturePath = utils::source_repo / fmt::format("models/{}_diffuse.png", modelName);
    Texture2D texture = LoadTexture(texturePath.string().c_str());

    auto vertexShaderPath =
        __DIRNAME__ / fmt::format("{}.vs", prog.get<std::string>("--vertex-shader"));
    auto fragShaderPath =
        __DIRNAME__ / fmt::format("{}.fs", prog.get<std::string>("--fragment-shader"));
    Shader shader = LoadShader(vertexShaderPath.string().c_str(), fragShaderPath.string().c_str());

    model.materials[0].shader = shader;
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    Vector3 position = {0.0f, 0.0f, 0.0f};

    SetCameraMode(camera, CAMERA_FREE);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        DrawModel(model, position, 0.2f, WHITE);
        DrawGrid(10, 1.0f);
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadModel(model);
    CloseWindow();

    return 0;
}
