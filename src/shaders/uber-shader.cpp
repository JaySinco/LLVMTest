#include "./uber-shader.h"

UberShader::UberShader() = default;

UberShader::~UberShader()
{
    UnloadShader(shader_);
    UnloadTexture(texture_);
    UnloadModel(model_);
}

void UberShader::loadManifest(nlohmann::json const& j)
{
    shader_ = loadShader(j["vertexShader"], j["fragmentShader"]);
    auto& text = j["texture"];
    texture_ = loadTexture(text["file"], text["type"], text["filter"], text["wrap"]);
    model_ = loadModel(j["model"]);
    model_.materials[0].shader = shader_;
    model_.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_;
}

void UberShader::render(nlohmann::json const& j)
{
    BaseShader::loadManifest(j);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth_, screenHeight_, "Uber Shader");

    Camera camera = {0};
    camera.position = Vector3{4.0f, 4.0f, 4.0f};
    camera.target = Vector3{0.0f, 1.0f, -1.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector3 position = {0.0f, 0.0f, 0.0f};

    SetCameraMode(camera, CAMERA_FREE);
    SetTargetFPS(60);
    UberShader::loadManifest(j);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        DrawModel(model_, position, 0.2f, WHITE);
        DrawGrid(10, 1.0f);
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
}
