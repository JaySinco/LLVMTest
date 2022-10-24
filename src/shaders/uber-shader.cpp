#include "./uber-shader.h"

UberShader::UberShader() {}

UberShader::~UberShader()
{
    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadModel(model);
}

void UberShader::load_manifest(nlohmann::json const& j)
{
    shader = load_shader(j["vertexShader"], j["fragmentShader"]);
    texture = load_texture(j["texture"]["file"], j["texture"]["cube"]);
    model = load_model(j["model"]);
    model.materials[0].shader = shader;
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
}

void UberShader::render(nlohmann::json const& j)
{
    BaseShader::load_manifest(j);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Uber Shader");

    Camera camera = {0};
    camera.position = Vector3{4.0f, 4.0f, 4.0f};
    camera.target = Vector3{0.0f, 1.0f, -1.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector3 position = {0.0f, 0.0f, 0.0f};

    SetCameraMode(camera, CAMERA_FREE);
    SetTargetFPS(60);
    UberShader::load_manifest(j);

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

    CloseWindow();
}
