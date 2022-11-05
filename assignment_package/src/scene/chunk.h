#pragma once
#include "drawable.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "block.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include <openglcontext.h>

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// have Chunk inherit from Drawable
class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    // The helpers to get VBO data
    void fillBuffer(std::vector<GLuint> &indices, std::vector<glm::vec4> &buffer, std::vector<glm::vec2> &uvs);
    BlockType getNeighborBlock(int x, int y, int z, glm::vec4 dirVec) const;

public:
    // constructor as a subclass of Drawable
    Chunk(OpenGLContext *context);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);

    // createVBOData needs to be implemented as a subclass of Drawable
    // since chunk's drawMode is still GL_TRIANGLES, no need to implement drawMode() here.
    virtual void createVBOdata() override;

    virtual ~Chunk();
};




