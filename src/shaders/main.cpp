#include "utils/base.h"
#include "raylib.h"

int main(int argc, char** argv)
{
    int const screenWidth = 800;
    int const screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - model shader");

    Camera camera = {0};
    camera.position = Vector3{4.0f, 4.0f, 4.0f};
    camera.target = Vector3{0.0f, 1.0f, -1.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Model model = LoadModel((utils::source_repo / "models/watermill.obj").string().c_str());
    Texture2D texture =
        LoadTexture((utils::source_repo / "models/watermill_diffuse.png").string().c_str());

    Shader shader = LoadShader(0, (__DIRNAME__ / "grayscale.fs").string().c_str());

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
        DrawText("(c) Watermill 3D model by Alberto Cano", screenWidth - 210, screenHeight - 20, 10,
                 GRAY);
        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadShader(shader);
    UnloadTexture(texture);
    UnloadModel(model);
    CloseWindow();

    return 0;
}
