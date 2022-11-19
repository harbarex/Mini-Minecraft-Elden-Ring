#include "chunk.h"
#include <iostream>



Chunk::Chunk(OpenGLContext *context)
    : Drawable(context),
      m_blocks(),
      m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
      vboLoaded(false)
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


/**
 * @brief Chunk::getNeighborBlock
 *  Retrieve the neighboring block (along the dirVec)
 *  of the current block location at (x, y, z).
 * @param x
 * @param y
 * @param z
 * @param dirVec
 * @return
 */
BlockType Chunk::getNeighborBlock(int x, int y, int z, glm::vec4 dirVec) const
{
    // init to empty
    BlockType block = EMPTY;

    // get neighboring block coordiante
    int nx = x + (int) dirVec[0];
    int ny = y + (int) dirVec[1];
    int nz = z + (int) dirVec[2];

    // check boundary at first
    // y = -1 or 256 => ignore, treat as empty (OUT OF THE WORLD)
    // either x = (-1 or 16) or z = (-1 or 16) => check neighboring chunk
    if (ny == -1 || ny == 256) {
        // treat it as EMPTY
        return block;
    }

    // <neighbor x coord, Direction, neighbor's x block at>
    std::tuple<int, Direction, int> xBoundaryInfo[2] = {std::make_tuple(-1, XNEG, 15),
                                                        std::make_tuple(16, XPOS, 0)};
    for (int i = 0; i < 2; i++) {
        if (nx == std::get<0>(xBoundaryInfo[i])) {
            Direction dir = std::get<1>(xBoundaryInfo[i]);
            // note: each dir is already instantiated in m_neighbors (to nullptr)
            Chunk *neighborChunk = m_neighbors.at(dir);
            return neighborChunk != nullptr ? neighborChunk->getBlockAt(std::get<2>(xBoundaryInfo[i]), y, z)
                                            : block;
        }
    }

    // <neighbor z coord, Direction, neighbor's z block at>
    std::tuple<int, Direction, int> zBoundaryInfo[2] = {std::make_tuple(-1, ZNEG, 15),
                                                        std::make_tuple(16, ZPOS, 0)};
    for (int i = 0; i < 2; i++) {
        if (nz == std::get<0>(zBoundaryInfo[i])) {
            Direction dir = std::get<1>(zBoundaryInfo[i]);
            // note: each dir is already instantiated in m_neighbors (to nullptr)
            Chunk *neighborChunk = m_neighbors.at(dir);
            return neighborChunk != nullptr ? neighborChunk->getBlockAt(x, y, std::get<2>(zBoundaryInfo[i]))
                                            : block;
        }
    }

    // within the range => check the chunk itself
    return getBlockAt(nx, ny, nz);
}

/**
 * @brief Chunk::getNeighbors
 * @return
 */
std::unordered_map<Direction, Chunk*, EnumHash> Chunk::getNeighbors() const
{
    return m_neighbors;
}

/**
 * @brief pushVec4ToBuffer
 *  The helper func to push 4 elements into buffer array
 * @param buf
 * @param vec
 */
void pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec)
{
    for (int i = 0; i < 4; i++) {
        buf.push_back(vec[i]);
    }
}


/**
 * @brief pushVec2ToBuffer
 *   *  The helper func to push 2 elements into buffer array
 * @param buf
 * @param vec
 */
void pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec)
{
    for (int i = 0; i < 2; i++) {
        buf.push_back(vec[i]);
    }
}

/**
 * @brief Chunk::generateVBOdata
 *  This method generates the needed vertex buffer & index data for this chunk.
 *  Note: the order of the vertex buffer is (pos, normal, uv, animatable flag).
 *  All of them are put in a vector<float>.
 * @return
 */
ChunkVBOdata Chunk::generateVBOdata()
{
    // init
    ChunkVBOdata vbo = ChunkVBOdata((Chunk*)(this));

    generateVBOdataDrawType(vbo, TerrainDrawType::opaque);
    generateVBOdataDrawType(vbo, TerrainDrawType::transparent);

    return vbo;
}

/**
 * @brief Chunk::generateVBOdata
 *  This method generates the needed vertex buffer & index data for this chunk.
 *  Note: the order of the vertex buffer is (pos, normal, uv, animatable flag).
 *  All of them are put in a vector<float>.
 * @param vbo, ChunkVBOdata
 * @param drawType, TerrainDrawType
 * @return
 */
void Chunk::generateVBOdataDrawType(ChunkVBOdata &vbo, TerrainDrawType drawType) {

    // basically, iterate through all the blocks contained in a chunk
    // each chunk : 16 x 256 x 16
    // each chunk contains the whole y axis [0-256)
    int nVert = 0;

    // Used as the indices for triangulation
    std::vector<GLuint> faceIndices = {0, 1, 2, 0, 2, 3};
    glm::vec4 out = glm::vec4();
    std::vector<float>* processingBuffer;
    std::vector<GLuint>* processingIndices;

    switch (drawType) {
    case (TerrainDrawType::opaque):
        processingBuffer = &vbo.buffer;
        processingIndices = &vbo.indices;
        break;
    case (TerrainDrawType::transparent):
        processingBuffer = &vbo.transparentBuffer;
        processingIndices = &vbo.transparentIndices;
        break;
    }

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {

                // get each block at (x, y, z) in this chunk
                // remember, there are 6 faces for a block
                BlockType blockType = getBlockAt(x, y, z);

                if (!checkBlockDrawing(drawType, blockType)) {
                    continue;
                }

                // iterate through each face and see if it has an opague neighbor
                // Block::BlockCollection contains the faces of various kinds of blocks
                for (const BlockFace &face : Block::BlockCollection[blockType]) {

                    // the neighboring block might be in the neighboring chunk
                    BlockType neighborBlockType = getNeighborBlock(x, y, z, face.normal);

                    if (!checkBlockFaceDrawing(drawType, neighborBlockType)) {
                        continue;
                    }

                    // add this face
                    for (const VertexData &vert : face.vertices) {
                        // buffer: pos0nor0col0uv0
                        pushVec4ToBuffer(*processingBuffer, vert.pos + glm::vec4(x, y, z, 0));
                        pushVec4ToBuffer(*processingBuffer, face.normal);
                        pushVec2ToBuffer(*processingBuffer, vert.uv);
                        pushVec2ToBuffer(*processingBuffer, Block::getAnimatableFlag(blockType));
                    }
                    // add indices for each face (4 vertices)
                    for (int index : faceIndices) {
                        (*processingIndices).push_back(nVert + index);
                    }
                    // move the offset for indices
                    nVert += 4;
                }
            }
        }
    }
}

/**
 * @brief Chunk::checkBlockDrawing
 *  check if current block needs to be drawn
 * @param drawType : TerrainDrawType
 * @param blockType : BlockType
 * @return boolean indicating if we need to draw current block or not
 */
bool Chunk::checkBlockDrawing(TerrainDrawType drawType, BlockType blockType) const {

    // skip empty block
    if (Block::isEmpty(blockType)) {
        return false;
    }

    // skip transparent block in opaque drawing
    if (drawType == TerrainDrawType::opaque && Block::isTransparent(blockType)) {
        return false;
    }

    // skip opaque block in transparent drawing
    if (drawType == TerrainDrawType::transparent && Block::isOpaque(blockType)) {
        return false;
    }

    return true;
}

//
/**
 * @brief Chunk::checkBlockFaceDrawing
 *  check if current block face needs to be drawn
 * @param drawType : TerrainDrawType
 * @param blockType : BlockType, adjacent block
 * @return boolean indicating if we need to draw current block or not
 */
bool Chunk::checkBlockFaceDrawing(TerrainDrawType drawType, BlockType neighborBlockType) const {

    // skip face adjacent to opaque block
    if (Block::isOpaque(neighborBlockType)) {
        return false;
    }

    // skip face adjacent to non-empty block in transparent drawing
    if (drawType == TerrainDrawType::transparent && !Block::isEmpty(neighborBlockType)) {
        return false;
    }

    return true;
}


/**
 * @brief Chunk::createVBOdata
 * @param vbo : ChunkVBOdata, contains the loaded interleaved vertex data and index data
 */
void Chunk::createVBOdata(ChunkVBOdata &vbo)
{
    // remember to set m_count
    m_count = vbo.indices.size();
    m_transparentCount = vbo.transparentIndices.size();

    int bufferSize = vbo.buffer.size();
    int transparentBufferSize = vbo.transparentBuffer.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), vbo.indices.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize * sizeof(float), vbo.buffer.data(), GL_STATIC_DRAW);

    generateTransparentIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufTransparentIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_transparentCount * sizeof(GLuint), vbo.transparentIndices.data(), GL_STATIC_DRAW);

    generateTransparentData();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufTransparentData);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, transparentBufferSize * sizeof(float), vbo.transparentBuffer.data(), GL_STATIC_DRAW);

    // set to vboLoaded to true
    vboLoaded = true;

    return;

}

/**
 * @brief Chunk::createVBOdata
 * Generate the buffer for chunk rendering.
 * Note: each block & each block face are already created in block.h
 * Note: this is used only for MS1. In MS2 & MS3, vbo of each chunk
 * is hold by the terrain. Won't be generated on the fly here.
 */
void Chunk::createVBOdata()
{
    ChunkVBOdata vbo = generateVBOdata();
    createVBOdata(vbo);
}


/**
 * @brief Chunk::isVBOLoaded
 * @return
 */
bool Chunk::isVBOLoaded() const
{
    return vboLoaded;
}


/**
 * @brief Chunk::destroyVBOdata
 */
void Chunk::destroyVBOdata()
{
    Drawable::destroyVBOdata();
    vboLoaded = false;
}



Chunk::~Chunk(){}

