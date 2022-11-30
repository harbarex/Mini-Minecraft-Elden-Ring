#ifndef TEXTURE_H
#define TEXTURE_H

#pragma once

#include <openglcontext.h>
#include <la.h>
#include <memory>

/**
 * @brief The NPCTexture enum
 *  An enum to hold the IDs of each npc texture, used in MyGL.
 */
enum NPCTexture : unsigned char
{
   SHEEP, STEVE
};

class Texture
{
public:

    Texture();
    Texture(OpenGLContext* context);
    ~Texture();

    Texture(const Texture &texture);

    void create(const char *texturePath);
    void load(int texSlot);
    void bind(int texSlot);

    // get slot
    int getSlot() const;

private:
    OpenGLContext* context;
    GLuint m_textureHandle;
    std::shared_ptr<QImage> m_textureImage;
    // the textSlot being loaded
    int slot;
};


#endif // TEXTURE_H
