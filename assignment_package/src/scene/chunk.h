#pragma once
#include "drawable.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "block.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include <openglcontext.h>

class Chunk;

// this struct is used to hold the VBO data of a given chunk
// identified by (x, z) coord
struct ChunkVBOdata
{
    // keep a pointer of the chunk
    Chunk* mp_chunk;

    // MS1 - opaque
    std::vector<GLuint> indices;
    // order: pos (vec4) + normal (vec4) + color (vec4) + uv (vec2)
    std::vector<float> buffer;

    // MS2: add transparent part
    std::vector<GLuint> transparentIndices;
    // order: pos (vec4) + normal (vec4) + color (vec4) + uv (vec2)
    std::vector<float> transparentBuffer;

    // constructors
    ChunkVBOdata(Chunk* chunk)
        : mp_chunk(chunk), indices(), buffer(),
          transparentIndices(), transparentBuffer() {}

};

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

    // get neighboring block
    BlockType getNeighborBlock(int x, int y, int z, glm::vec4 dirVec) const;

    // TODO: a member variable to mark vboLoaded
    bool vboLoaded;

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

    // this generates the vbo data for further rendering
    ChunkVBOdata generateVBOdata() const;

    // this takes ChunkVBOdata in and buffers them into this Chunk (Drawable)
    void createVBOdata(ChunkVBOdata &vbo);

    int getNNeighbors() const;

    std::unordered_map<Direction, Chunk*, EnumHash> getNeighbors() const;

    bool isVBOLoaded() const;

    // helper method to destroy vbo and set isVBOLoaded to false
    void destroyVBOdata();

    virtual ~Chunk();
};


