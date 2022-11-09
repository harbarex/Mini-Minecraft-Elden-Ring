#pragma once
#include "glm_includes.h"
#include <unordered_map>
#include <array>


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};


/**
 * This serves as the Chunk's helper
 * to create Blocks, BlockFaces, VertexData.
 * Moreover, various kind of blocks are defined here as well.
 **/
struct VertexData
{
    // each VertexData contains a vertex position and a UV coord
    glm::vec4 pos;
    // later, the uv coordinate is manually set
    glm::vec2 uv;

    VertexData(glm::vec4 p, glm::vec2 u)
        : pos(p), uv(u) {}
    VertexData()
        : pos(), uv() {}

};

struct BlockFace
{

    // represents a face of a cube
    Direction dir;
    glm::vec4 normal;
    std::array<VertexData, 4> vertices;

    BlockFace(Direction dir, glm::vec4 nor, const VertexData &v1, const VertexData &v2, const VertexData &v3, const VertexData &v4)
        : dir(dir), normal(nor), vertices({v1, v2, v3, v4}) {}
    BlockFace(): dir(), normal(), vertices() {}
};


/**
 * @brief The Block class
 *  Define a class that handles all blocks
 */
class Block
{

private:

    // create the 6 faces of a block with manually defined uv offsets of each face
    static std::array<BlockFace, 6> createBlockFaces(std::array<glm::vec2, 6> uvOffsets);
    // default func to create the 6 faces of a given block (uv offset is set to (0, 0))
    static std::array<BlockFace, 6> createBlockFaces();

public:

    // a collection of all the pos, nor, col, uvs of all types of blocks
    static std::unordered_map<BlockType, std::array<BlockFace, 6>> BlockCollection;

    // the rule to determine whether a given block is opaque or not
    static bool isOpaque(BlockType type);

    // the rule to determine whether a given block is transparent or not
    static bool isTransparent(BlockType type);

    // the function that defines the color of each block type
    static glm::vec4 getColors(BlockType type);

};


