#include "base-shader.h"
#include <rlgl.h>
#include <glad.h>

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

static unsigned int rl_load_texture_cubemap(void const* data, int size, int format)
{
    unsigned int id = 0;
    unsigned int dataSize = GetPixelDataSize(size, size, format);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if (glInternalFormat != -1) {
        if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) {
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        } else if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) {
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
            glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }

        // Load cubemap faces
        for (unsigned int i = 0; i < 6; i++) {
            if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0,
                             glFormat, glType, (unsigned char*)data + i * dataSize);
            else
                glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat,
                                       size, size, 0, dataSize,
                                       (unsigned char*)data + i * dataSize);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    if (id > 0)
        TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Cubemap texture loaded successfully (%ix%i)", id,
                 size, size);
    else
        TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load cubemap texture");

    return id;
}

static TextureCubemap load_texture_cubemap(Image image, int layout)
{
    TextureCubemap cubemap = {0};

    if (layout == CUBEMAP_LAYOUT_AUTO_DETECT)  // Try to automatically guess layout type
    {
        // Check image width/height to determine the type of cubemap provided
        if (image.width > image.height) {
            if ((image.width / 6) == image.height) {
                layout = CUBEMAP_LAYOUT_LINE_HORIZONTAL;
                cubemap.width = image.width / 6;
            } else if ((image.width / 4) == (image.height / 3)) {
                layout = CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE;
                cubemap.width = image.width / 4;
            } else if (image.width >= (int)((float)image.height * 1.85f)) {
                layout = CUBEMAP_LAYOUT_PANORAMA;
                cubemap.width = image.width / 4;
            }
        } else if (image.height > image.width) {
            if ((image.height / 6) == image.width) {
                layout = CUBEMAP_LAYOUT_LINE_VERTICAL;
                cubemap.width = image.height / 6;
            } else if ((image.width / 3) == (image.height / 4)) {
                layout = CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR;
                cubemap.width = image.width / 3;
            }
        }

        cubemap.height = cubemap.width;
    }

    // Layout provided or already auto-detected
    if (layout != CUBEMAP_LAYOUT_AUTO_DETECT) {
        int size = cubemap.width;

        Image faces = {0};            // Vertical column image
        Rectangle faceRecs[6] = {0};  // Face source rectangles
        for (int i = 0; i < 6; i++) faceRecs[i] = Rectangle{0, 0, (float)size, (float)size};

        if (layout == CUBEMAP_LAYOUT_LINE_VERTICAL) {
            faces = ImageCopy(image);  // Image data already follows expected convention
        } else if (layout == CUBEMAP_LAYOUT_PANORAMA) {
            // TODO: Convert panorama image to square faces...
            // Ref: https://github.com/denivip/panorama/blob/master/panorama.cpp
        } else {
            if (layout == CUBEMAP_LAYOUT_LINE_HORIZONTAL)
                for (int i = 0; i < 6; i++) faceRecs[i].x = (float)size * i;
            else if (layout == CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR) {
                faceRecs[0].x = (float)size;
                faceRecs[0].y = (float)size;
                faceRecs[1].x = (float)size;
                faceRecs[1].y = (float)size * 3;
                faceRecs[2].x = (float)size;
                faceRecs[2].y = 0;
                faceRecs[3].x = (float)size;
                faceRecs[3].y = (float)size * 2;
                faceRecs[4].x = 0;
                faceRecs[4].y = (float)size;
                faceRecs[5].x = (float)size * 2;
                faceRecs[5].y = (float)size;
            } else if (layout == CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE) {
                faceRecs[0].x = (float)size * 2;
                faceRecs[0].y = (float)size;
                faceRecs[1].x = 0;
                faceRecs[1].y = (float)size;
                faceRecs[2].x = (float)size;
                faceRecs[2].y = 0;
                faceRecs[3].x = (float)size;
                faceRecs[3].y = (float)size * 2;
                faceRecs[4].x = (float)size;
                faceRecs[4].y = (float)size;
                faceRecs[5].x = (float)size * 3;
                faceRecs[5].y = (float)size;
            }

            // Convert image data to 6 faces in a vertical column, that's the optimum layout for
            // loading
            faces = GenImageColor(size, size * 6, MAGENTA);
            ImageFormat(&faces, image.format);

            // NOTE: Image formating does not work with compressed textures

            for (int i = 0; i < 6; i++)
                ImageDraw(&faces, image, faceRecs[i],
                          Rectangle{0, (float)size * i, (float)size, (float)size}, WHITE);
        }

        // NOTE: Cubemap data is expected to be provided as 6 images in a single data array,
        // one after the other (that's a vertical image), following convention: +X, -X, +Y, -Y, +Z,
        // -Z
        cubemap.id = rl_load_texture_cubemap(faces.data, size, faces.format);
        if (cubemap.id == 0) TRACELOG(LOG_WARNING, "IMAGE: Failed to load cubemap image");

        UnloadImage(faces);
    } else
        TRACELOG(LOG_WARNING, "IMAGE: Failed to detect cubemap image layout");

    return cubemap;
}

Texture BaseShader::load_texture(std::string const& texture, std::string const& type)
{
    auto abspath = (utils::source_repo / fmt::format("models/{}", texture)).string();
    if (type == "2d") {
        return LoadTexture(abspath.c_str());
    } else if (type == "cube") {
        Image image = LoadImage(abspath.c_str());
        return load_texture_cubemap(image, CUBEMAP_LAYOUT_AUTO_DETECT);
    } else {
        throw std::runtime_error(fmt::format("texture type not support: {}", type));
    }
}

Model BaseShader::load_model(std::string const& model)
{
    return LoadModel((utils::source_repo / fmt::format("models/{}", model)).string().c_str());
}