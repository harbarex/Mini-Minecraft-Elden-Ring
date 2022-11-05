#include "chunk.h"
#include <iostream>


Chunk::Chunk(OpenGLContext *context)
    : Drawable(context),
      m_blocks(),
      m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}


// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}


struct Cube
{
    std::vector<glm::vec4> pos = {
        // XPOS
        glm::vec4(1, 0, 1, 1),
        glm::vec4(1, 0, 0, 1),
        glm::vec4(1, 1, 0, 1),
        glm::vec4(1, 1, 1, 1),
        // XNEG
        glm::vec4(0, 0, 0, 1),
        glm::vec4(0, 0, 1, 1),
        glm::vec4(0, 1, 1, 1),
        glm::vec4(0, 1, 0, 1),
        // YPOS
        glm::vec4(0, 1, 1, 1),
        glm::vec4(1, 1, 1, 1),
        glm::vec4(1, 1, 0, 1),
        glm::vec4(0, 1, 0, 1),
        // YNEG
        glm::vec4(0, 0, 0, 1),
        glm::vec4(1, 0, 0, 1),
        glm::vec4(1, 0, 1, 1),
        glm::vec4(0, 0, 1, 1),
        // ZPOS
        glm::vec4(0, 0, 1, 1),
        glm::vec4(1, 0, 1, 1),
        glm::vec4(1, 1, 1, 1),
        glm::vec4(0, 1, 1, 1),
        // ZNEG
        glm::vec4(1, 0, 0, 1),
        glm::vec4(0, 0, 0, 1),
        glm::vec4(0, 1, 0, 1),
        glm::vec4(1, 1, 0, 1)
    };

    std::vector<glm::vec4> normals = {
        // XPOS
        glm::vec4(1, 0, 0, 0),
        glm::vec4(1, 0, 0, 0),
        glm::vec4(1, 0, 0, 0),
        glm::vec4(1, 0, 0, 0),
        // XNEG
        glm::vec4(-1, 0, 0, 0),
        glm::vec4(-1, 0, 0, 0),
        glm::vec4(-1, 0, 0, 0),
        glm::vec4(-1, 0, 0, 0),
        // YPOS
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 1, 0, 0),
        // YNEG
        glm::vec4(0, -1, 0, 0),
        glm::vec4(0, -1, 0, 0),
        glm::vec4(0, -1, 0, 0),
        glm::vec4(0, -1, 0, 0),
        // ZPOS
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 0, 1, 0),
        // ZNEG
        glm::vec4(0, 0, -1, 0),
        glm::vec4(0, 0, -1, 0),
        glm::vec4(0, 0, -1, 0),
        glm::vec4(0, 0, -1, 0),
    };

    std::vector<GLuint> indices = {
        // XPOS
        0, 1, 2, 0, 2, 3,
        // XNEG
        4, 5, 6, 4, 6, 7,
        // YPOS
        8, 9, 10, 8, 10, 11,
        // YNEG
        12, 13, 14, 12, 14, 15,
        // ZPOS
        16, 17, 18, 16, 18, 19,
        // ZNEG
        20, 21, 22, 20, 22, 23
    };

};

/**
 * @brief Chunk::createVBOdata
 * TODO: Generate the buffer for chunk rendering.
 */
void Chunk::createVBOdata()
{

    std::vector<GLuint> indices = std::vector<GLuint>();
    std::vector<glm::vec4> pos, normals, colors = std::vector<glm::vec4>();

    Cube cube = Cube();
    // simple example for a cube (vertex positions, normals)
    // each chunk : 16 x 256 x 16
    // each chunk contains the whole y axis [0-256)
    // basically, iterate through all the blocks contained in a chunk
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                // get each block at (x, y, z) in this chunk
                // remember, there are 6 faces at most for a block
                BlockType blockType = getBlockAt(x, y, z);
                // let's draw all blocks at first
                if (blockType != EMPTY) {
                    // add all info of a block into the buffer
                    for (int i = 0; i < cube.pos.size(); i++) {
                        pos.push_back(cube.pos[i] + glm::vec4(x, y, z, 0));
                        normals.push_back(cube.normals[i]);
                        switch (blockType)
                        {
                        case GRASS:
                            colors.push_back(glm::vec4(95.f, 159.f, 53.f, 0.f) / 255.f);
                            break;
                        case DIRT:
                            colors.push_back(glm::vec4(121.f, 85.f, 58.f, 0.f) / 255.f);
                            break;
                        case STONE:
                            colors.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 0.f));
                            break;
                        case WATER:
                            colors.push_back(glm::vec4(0.f, 0.f, 0.75f, 0.f));
                            break;
                        default:
                            // Other block types are not yet handled, so we default to debug purple
                            colors.push_back(glm::vec4(1.f, 0.f, 1.f, 0.f));
                            break;
                        }
                    }
                    // add indices
                    int offset = indices.size();
                    for (int idx : cube.indices) {
                        indices.push_back(offset + idx);
                    }
                }
                // TODO: only the face (check opague -> add faces)
            }
        }

    }

    std::cout << "n indices: " << indices.size() << std::endl;
    // bind buffer & pass to gpu
    m_count = indices.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufNor);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals.size() * sizeof(glm::vec4), normals.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);

    // (TODO: single VBO buffer)
    // TODO: Iterate through each block inside a chunk
    // TODO: update m_count
    // TODO: only blBindBuffer & glBufferData once
    // TODO: check handles (in ohther cpp file)
    // make sure they read this buffer data correctly
    return;
}

Chunk::~Chunk(){}
