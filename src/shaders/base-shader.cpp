#include "base-shader.h"
#include <rlgl.h>
#include <raymath.h>

BaseShader::~BaseShader() {}

void BaseShader::load_manifest(nlohmann::json const& j)
{
    if (j.contains("screenWidth")) {
        screenWidth = j["screenWidth"];
    }
    if (j.contains("screenHeight")) {
        screenHeight = j["screenHeight"];
    }
}

Shader BaseShader::load_shader(std::string const& vertex, std::string const& fragment)
{
    return LoadShader((__DIRNAME__ / vertex).string().c_str(),
                      (__DIRNAME__ / fragment).string().c_str());
}

// Generate cubemap texture from HDR texture
static TextureCubemap gen_texture_cubemap(Shader shader, Texture2D panorama, int size, int format)
{
    TextureCubemap cubemap = {0};

    rlDisableBackfaceCulling();  // Disable backface culling to render inside the cube

    // STEP 1: Setup framebuffer
    //------------------------------------------------------------------------------------------
    unsigned int rbo = rlLoadTextureDepth(size, size, true);
    cubemap.id = rlLoadTextureCubemap(0, size, format);

    unsigned int fbo = rlLoadFramebuffer(size, size);
    rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0,
                        RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

    // Check if framebuffer is complete with attachments (valid)
    if (rlFramebufferComplete(fbo))
        TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
    //------------------------------------------------------------------------------------------

    // STEP 2: Draw to framebuffer
    //------------------------------------------------------------------------------------------
    // NOTE: Shader is used to convert HDR equirectangular environment map to cubemap equivalent (6
    // faces)
    rlEnableShader(shader.id);

    // Define projection matrix and send it to shader
    Matrix matFboProjection =
        MatrixPerspective(90.0 * DEG2RAD, 1.0, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

    // Define view matrix for every side of the cubemap
    Matrix fboViews[6] = {MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{1.0f, 0.0f, 0.0f},
                                       Vector3{0.0f, -1.0f, 0.0f}),
                          MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{-1.0f, 0.0f, 0.0f},
                                       Vector3{0.0f, -1.0f, 0.0f}),
                          MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 1.0f, 0.0f},
                                       Vector3{0.0f, 0.0f, 1.0f}),
                          MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, -1.0f, 0.0f},
                                       Vector3{0.0f, 0.0f, -1.0f}),
                          MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 0.0f, 1.0f},
                                       Vector3{0.0f, -1.0f, 0.0f}),
                          MatrixLookAt(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 0.0f, -1.0f},
                                       Vector3{0.0f, -1.0f, 0.0f})};

    rlViewport(0, 0, size, size);  // Set viewport to current fbo dimensions

    // Activate and enable texture for drawing to cubemap faces
    rlActiveTextureSlot(0);
    rlEnableTexture(panorama.id);

    for (int i = 0; i < 6; i++) {
        // Set the view matrix for the current cube face
        rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);

        // Select the current cubemap face attachment for the fbo
        // WARNING: This function by default enables->attach->disables fbo!!!
        rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0,
                            RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
        rlEnableFramebuffer(fbo);

        // Load and draw a cube, it uses the current enabled texture
        rlClearScreenBuffers();
        rlLoadDrawCube();

        // ALTERNATIVE: Try to use internal batch system to draw the cube instead of rlLoadDrawCube
        // for some reason this method does not work, maybe due to cube triangles definition?
        // normals pointing out?
        // TODO: Investigate this issue...
        // rlSetTexture(panorama.id); // WARNING: It must be called after enabling current
        // framebuffer if using internal batch system! rlClearScreenBuffers();
        // DrawCubeV(Vector3Zero(), Vector3One(), WHITE);
        // rlDrawRenderBatchActive();
    }
    //------------------------------------------------------------------------------------------

    // STEP 3: Unload framebuffer and reset state
    //------------------------------------------------------------------------------------------
    rlDisableShader();       // Unbind shader
    rlDisableTexture();      // Unbind texture
    rlDisableFramebuffer();  // Unbind framebuffer
    rlUnloadFramebuffer(
        fbo);  // Unload framebuffer (and automatically attached depth texture/renderbuffer)

    // Reset viewport dimensions to default
    rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
    rlEnableBackfaceCulling();
    //------------------------------------------------------------------------------------------

    cubemap.width = size;
    cubemap.height = size;
    cubemap.mipmaps = 1;
    cubemap.format = format;

    return cubemap;
}

Texture BaseShader::load_texture(std::string const& texture, bool cube)
{
    auto abspath = (utils::source_repo / fmt::format("models/{}", texture)).string();
    if (!cube) {
        return LoadTexture(abspath.c_str());
    } else {
        // Shader shdrCubemap = load_shader("cubemap.vs", "cubemap.fs");
        // Texture panorama = LoadTexture(abspath.c_str());
        // SetShaderValueTexture(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"),
        //                       panorama);

        // // Generate cubemap (texture with 6 quads-cube-mapping) from panorama HDR texture
        // // NOTE 1: New texture is generated rendering to texture, shader calculates the
        // sphere->cube
        // // coordinates mapping NOTE 2: It seems on some Android devices WebGL, fbo does not
        // properly
        // // support a FLOAT-based attachment, despite texture can be successfully created.. so
        // using
        // // PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 instead of PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
        // Texture text =
        //     gen_texture_cubemap(shdrCubemap, panorama, 1024,
        //     PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
        // UnloadTexture(panorama);
        // UnloadShader(shdrCubemap);
        // return text;
        Image a = LoadImage(abspath.c_str());
        Texture text = LoadTextureCubemap(a, CUBEMAP_LAYOUT_AUTO_DETECT);
        UnloadImage(a);
        return text;
    }
}

Model BaseShader::load_model(std::string const& model)
{
    return LoadModel((utils::source_repo / fmt::format("models/{}", model)).string().c_str());
}