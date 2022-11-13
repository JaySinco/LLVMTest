#include "base-shader.h"
#include <rlgl.h>
#include <glad.h>

static unsigned int generateCubemapTexture(void const* data, int size, int format)
{
    unsigned int id = 0;
    unsigned int data_size = GetPixelDataSize(size, size, format);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    unsigned int gl_internal_format, gl_format, gl_type;
    rlGetGlTextureFormats(format, &gl_internal_format, &gl_format, &gl_type);

    if (gl_internal_format != -1) {
        if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) {
            GLint swizzle_mask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);
        } else if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) {
            GLint swizzle_mask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
            glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);
        }

        for (unsigned int i = 0; i < 6; i++) {
            if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gl_internal_format, size, size,
                             0, gl_format, gl_type,
                             static_cast<unsigned char const*>(data) + i * data_size);
            } else {
                glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gl_internal_format,
                                       size, size, 0, data_size,
                                       static_cast<unsigned char const*>(data) + i * data_size);
            }
        }
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    if (id > 0) {
        spdlog::info("cubemap texture[{}] loaded successfully ({}x{})", id, size, size);
    } else {
        spdlog::error("failed to load cubemap texture");
    }

    return id;
}

static TextureCubemap loadTextureCubemap(Image image, int layout)
{
    TextureCubemap cubemap = {0};

    if (layout == CUBEMAP_LAYOUT_AUTO_DETECT) {
        if (image.width > image.height) {
            if ((image.width / 6) == image.height) {
                layout = CUBEMAP_LAYOUT_LINE_HORIZONTAL;

            } else if ((image.width / 4) == (image.height / 3)) {
                layout = CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE;

            } else if (image.width >= static_cast<int>(image.height * 1.85f)) {
                layout = CUBEMAP_LAYOUT_PANORAMA;
            }
        } else if (image.height > image.width) {
            if ((image.height / 6) == image.width) {
                layout = CUBEMAP_LAYOUT_LINE_VERTICAL;

            } else if ((image.width / 3) == (image.height / 4)) {
                layout = CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR;
            }
        }
    }

    if (layout == CUBEMAP_LAYOUT_AUTO_DETECT) {
        spdlog::error("failed to detect cubemap image layout");
        return cubemap;
    }

    if (layout == CUBEMAP_LAYOUT_LINE_HORIZONTAL) {
        cubemap.width = image.width / 6;
    } else if (layout == CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE) {
        cubemap.width = image.width / 4;
    } else if (layout == CUBEMAP_LAYOUT_PANORAMA) {
        cubemap.width = image.width / 4;
    } else if (layout == CUBEMAP_LAYOUT_LINE_VERTICAL) {
        cubemap.width = image.height / 6;
    } else if (layout == CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR) {
        cubemap.width = image.width / 3;
    }

    cubemap.height = cubemap.width;

    int size = cubemap.width;

    Image faces = {nullptr};
    Rectangle face_recs[6] = {0};
    for (auto& face_rec: face_recs) {
        face_rec = Rectangle{0, 0, static_cast<float>(size), static_cast<float>(size)};
    }

    if (layout == CUBEMAP_LAYOUT_LINE_VERTICAL) {
        faces = ImageCopy(image);
    } else if (layout == CUBEMAP_LAYOUT_PANORAMA) {
        // TODO: Convert panorama image to square faces...
        // Ref: https://github.com/denivip/panorama/blob/master/panorama.cpp
    } else {
        if (layout == CUBEMAP_LAYOUT_LINE_HORIZONTAL) {
            for (int i = 0; i < 6; i++) {
                face_recs[i].x = size * i;
            }
        } else if (layout == CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR) {
            face_recs[0].x = size;
            face_recs[0].y = size;
            face_recs[1].x = size;
            face_recs[1].y = size * 3;
            face_recs[2].x = size;
            face_recs[2].y = 0;
            face_recs[3].x = size;
            face_recs[3].y = size * 2;
            face_recs[4].x = 0;
            face_recs[4].y = size;
            face_recs[5].x = size * 2;
            face_recs[5].y = size;
        } else if (layout == CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE) {
            face_recs[0].x = size * 2;
            face_recs[0].y = size;
            face_recs[1].x = 0;
            face_recs[1].y = size;
            face_recs[2].x = size;
            face_recs[2].y = 0;
            face_recs[3].x = size;
            face_recs[3].y = size * 2;
            face_recs[4].x = size;
            face_recs[4].y = size;
            face_recs[5].x = size * 3;
            face_recs[5].y = size;
        }

        faces = GenImageColor(size, size * 6, MAGENTA);
        ImageFormat(&faces, image.format);

        for (int i = 0; i < 6; i++) {
            ImageDraw(&faces, image, face_recs[i],
                      Rectangle{0, static_cast<float>(size * i), static_cast<float>(size),
                                static_cast<float>(size)},
                      WHITE);
        }
    }

    cubemap.id = generateCubemapTexture(faces.data, size, faces.format);
    if (cubemap.id == 0) {
        spdlog::error("failed to load cubemap image");
    }

    UnloadImage(faces);

    return cubemap;
}

static void setupTextureParam(std::string const& type, Texture text, std::string const& filter,
                              std::string const& wrap)
{
    GLint gl_filter = 0;
    if (filter == "nearest") {
        gl_filter = GL_NEAREST;
    } else if (filter == "linear") {
        gl_filter = GL_LINEAR;
    } else if (filter == "mipmap") {
        gl_filter = GL_LINEAR_MIPMAP_LINEAR;
    } else {
        THROW_(fmt::format("texture filter not support: {}", filter));
    }

    GLint gl_wrap = 0;
    if (wrap == "clamp") {
        gl_wrap = GL_CLAMP_TO_EDGE;
    } else if (wrap == "repeat") {
        gl_wrap = GL_REPEAT;
    } else {
        THROW_(fmt::format("texture filter not support: {}", filter));
    }

    GLenum gl_target = 0;
    if (type == "2d") {
        gl_target = GL_TEXTURE_2D;
    } else if (type == "cube") {
        gl_target = GL_TEXTURE_CUBE_MAP;
    } else {
        THROW_(fmt::format("texture type not support: {}", type));
    }

    glBindTexture(gl_target, text.id);
    glTexParameteri(gl_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(gl_target, GL_TEXTURE_MIN_FILTER, gl_filter);
    glTexParameteri(gl_target, GL_TEXTURE_WRAP_S, gl_wrap);
    glTexParameteri(gl_target, GL_TEXTURE_WRAP_T, gl_wrap);
    glTexParameteri(gl_target, GL_TEXTURE_WRAP_R, gl_wrap);
    if (filter == "mipmap") {
        glGenerateMipmap(gl_target);
    }
    glBindTexture(gl_target, 0);
}

BaseShader::~BaseShader() = default;

void BaseShader::loadManifest(nlohmann::json const& j)
{
    if (j.contains("screenWidth")) {
        screen_width = j["screenWidth"];
    }
    if (j.contains("screenHeight")) {
        screen_height = j["screenHeight"];
    }
}

Shader BaseShader::loadShader(std::string const& vertex, std::string const& fragment)
{
    return LoadShader((__RESDIR__ / vertex).string().c_str(),
                      (__RESDIR__ / fragment).string().c_str());
}

RenderTexture2D BaseShader::loadBufferTexture(int width, int height, std::string const& filter,
                                              std::string const& wrap)
{
    RenderTexture2D target = {0};

    target.id = rlLoadFramebuffer(width, height);

    if (target.id > 0) {
        rlEnableFramebuffer(target.id);

        target.texture.id =
            rlLoadTexture(nullptr, width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        target.texture.mipmaps = 1;

        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;  // DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0,
                            RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH,
                            RL_ATTACHMENT_RENDERBUFFER, 0);

        if (rlFramebufferComplete(target.id)) {
            spdlog::info("FBO: [ID {}] Framebuffer object created successfully", target.id);
        }
        rlDisableFramebuffer();
    } else {
        spdlog::error("FBO: Framebuffer object can not be created");
    }

    setupTextureParam("2d", target.texture, filter, wrap);
    return target;
}

Texture BaseShader::loadTexture(std::string const& texture, std::string const& type,
                                std::string const& filter, std::string const& wrap)
{
    auto abspath = (utils::kSourceRepo / fmt::format("res/{}", texture)).string();
    Texture text;
    if (type == "2d") {
        text = LoadTexture(abspath.c_str());
    } else if (type == "cube") {
        Image image = LoadImage(abspath.c_str());
        text = loadTextureCubemap(image, CUBEMAP_LAYOUT_AUTO_DETECT);
    } else {
        THROW_(fmt::format("texture type not support: {}", type));
    }
    setupTextureParam(type, text, filter, wrap);
    return text;
}

Model BaseShader::loadModel(std::string const& model)
{
    return LoadModel((utils::kSourceRepo / fmt::format("res/{}", model)).string().c_str());
}