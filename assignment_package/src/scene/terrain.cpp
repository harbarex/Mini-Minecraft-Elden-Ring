#include "terrain.h"
#include "noise.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), createdChunks(),
      m_chunksWithBlocks(), m_chunksWithBlocksLock(),
      m_chunksWithVBOs(), m_chunksWithVBOsLock(),
      m_generatedTerrain(), mp_context(context)
{}

Terrain::~Terrain() {}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}


/**
 * @brief setChunkMinMaxXZ
 *  The logic to find the grid around a player.
 *  Basically, the grid is a square of size ((1 + 2 * halfGridSize) chunks) , centered at player's chunk.
 * @param playerX
 * @param playerZ
 * @param minX
 * @param maxX
 * @param minZ
 * @param maxZ
 */
void setChunkMinMaxXZ(float playerX,
                      float playerZ,
                      int halfGridSize,
                      int &minX,
                      int &maxX,
                      int &minZ,
                      int &maxZ)
{
    int xFloor = static_cast<int>(glm::floor(playerX / 16.f));
    int zFloor = static_cast<int>(glm::floor(playerZ / 16.f));

    // Note: the xFloor, zFloor stand for the i-th chunk (not chunk origin coordinate)
    // Remember to multiply with 16 to get back to the chunk origin
    // Also, the chunk directly at maxX & maxZ won't be included.
    minX = (xFloor - halfGridSize) * 16;
    maxX = (xFloor + (halfGridSize + 1)) * 16;
    minZ = (zFloor - halfGridSize) * 16;
    maxZ = (zFloor + (halfGridSize + 1)) * 16;

}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    // each instantiated chunk is a drawable item
    uPtr<Chunk> chunk = mkU<Chunk>(this->mp_context);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

/**
 * @brief Terrain::draw
 * @param playerX
 * @param playerZ
 * @param halfGridSize
 * @param shaderProgram
 */
void Terrain::draw(float playerX, float playerZ, int halfGridSize, ShaderProgram *shaderProgram)
{
    // get the grid of minX, maxX, minZ, maxZ by (playerX, playerZ)
    int minX, maxX, minZ, maxZ;

    setChunkMinMaxXZ(playerX,
                     playerZ,
                     halfGridSize,
                     minX,
                     maxX,
                     minZ,
                     maxZ);

    // use the origianl terrain::draw
    draw(minX, maxX, minZ, maxZ, shaderProgram);

}

// When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
// Note: minX, maxX, minZ, maxZ should already be the origins of each chunk
// USse Terrain::draw(float playerX, float playerZ, ShaderProgram*) in MyGL
// to ensure the region around the player is drawn.
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {

    // - Iterate through each chunk
    // - Set the model matrix based on new X, Z
    // - Let each chunk draw itself
    for (int x = minX; x < maxX; x += 16) {
        for (int z = minZ; z < maxZ; z += 16) {

            // check has a chunk or not
            if (!hasChunkAt(x, z)) {
                // do nothing if no chunk at (x, z)
                continue;
            }

            // get the pointer of the chunk at (x, z)
            const uPtr<Chunk> &chunk = getChunkAt(x, z);

            // TODO: only draw the chunk with vbo loaded
            // skip if not loaded yet
            if (!chunk->isVBOLoaded()) {
                continue;
            }

            // set model matrix
            glm::mat4 translation = glm::mat4(1.f);
            translation[3] = glm::vec4(x, 0, z, 1);

            shaderProgram->setModelMatrix(translation);
            // use drawInterleaved to draw the interleaved buffer data
            shaderProgram->drawInterleaved(*chunk);

        }
    }
}


/**
 * @brief Terrain::checkThreadResults
 */
void Terrain::checkThreadResults()
{
    m_chunksWithBlocksLock.lock();
    spawnVBOWorkers(m_chunksWithBlocks);
    m_chunksWithBlocks.clear();
    m_chunksWithBlocksLock.unlock();

    // send to gpu
    m_chunksWithVBOsLock.lock();
    for (ChunkVBOdata &vbo : m_chunksWithVBOs) {
        vbo.mp_chunk->createVBOdata(vbo);
    }
    m_chunksWithVBOs.clear();
    m_chunksWithVBOsLock.unlock();
}

/**
 * @brief Terrain::expand
 *  For MS1: 3 x 3 chunk
 *  For MS2: 5 x 5 zones
 * @param playerX
 * @param playerZ
 * @param halfGridSize
 */
void Terrain::expand(float playerX, float playerZ, int halfGridSize)
{
    // TODO: change to use zone
    // m_generatedTerrain.insert(toKey(0, 0));

    // set the min max X & Z
    // note the chunk at (maxX, maxZ) is not included
    int minX, maxX, minZ, maxZ;
    setChunkMinMaxXZ(playerX,
                     playerZ,
                     halfGridSize,
                     minX,
                     maxX,
                     minZ,
                     maxZ);

    // TODO: destroy VBOs
    // instantnate all needed chunks at first
    for (int x = minX; x < maxX; x += 16) {

        for (int z = minZ; z < maxZ; z += 16) {

            int64_t key = toKey(x, z);

            if (createdChunks.find(key) != createdChunks.end()) {
                // this chunk is already instantiated
                // all the blocks are already filled as well
                // TODO: spawn VBO worker for this chunk
                // TODO: include in zone loop later?
                Chunk *chunk = getChunkAt(x, z).get();
                // spawnVBOWorker(chunk);
            }

            else // if (createdChunks.find(key) == createdChunks.end())
            {
                // otherwise, spawn the thread to instantiate chunk & fill its blocks
                // TODO: convert to zone later
                std::cout << "Expand: " << x << " " << z << std::endl;
                createdChunks.insert(key);
                spawnFillBlocksWorker(x, z);
            }

        }
    }

}



/**
 * @brief Terrain::instantiateChunkAndFillBlocks
 *  This helper is used to fill the blocks in the chunk
 * @param chunkX : int, the chunk's origin X
 * @param chunkZ : int, the chunk's origin Z
 */
void Terrain::instantiateChunkAndfillBlocks(int chunkX, int chunkZ)
{
    // instantiate the chunk
    instantiateChunkAt(chunkX, chunkZ);

    Noise terrainHeightMap;

    for (int x = chunkX; x < chunkX + 16; x++) {

        for (int z = chunkZ; z < chunkZ + 16; z++) {

            // TODO: wrap up the logics later
            double y = terrainHeightMap.getHeight(x,z);

            if( y < 136){
                setBlockAt(x, y, z, WATER);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    setBlockAt(x, y_dirt, z, WATER);
                }
            }
            else if( y < 142){
                setBlockAt(x, y, z, GRASS);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    setBlockAt(x, y_dirt, z, DIRT);
                }
            }
            else if (y > 190){
                setBlockAt(x, y, z, SNOW);
                for(int y_stone=128; y_stone<y; y_stone++){
                    setBlockAt(x, y_stone, z, STONE);
                }
            }
            else{
                setBlockAt(x, y, z, STONE);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    setBlockAt(x, y_dirt, z, DIRT);
                }
            }

            // Set DIRT from 0-128 height values
            for(int y_underground=0; y_underground<128;y_underground++){
                setBlockAt(x, y_underground, z, STONE);
            }

        }

    }
}


/**
 * @brief Terrain::putBlockAt
 *  Set the block at (x, y, z) as a block t,
 *  then destroy the chunk's VBO containing this block (to reload).
 * @param x
 * @param y
 * @param z
 * @param t
 */
void Terrain::placeBlockAt(int x, int y, int z, BlockType t)
{
    // set to block type t
    setBlockAt(x, y, z, t);

    // find the corresponding chunk origin with block's (x, z)
    int chunkX = static_cast<int>(glm::floor(x / 16.f)) * 16;
    int chunkZ = static_cast<int>(glm::floor(z / 16.f)) * 16;

    if (!hasChunkAt(chunkX, chunkZ)) {
        // no chunk at this place
        // not allow to place any block where there's no chunk.
        return;
    }

    // create the VBO again
    const uPtr<Chunk> &chunk = getChunkAt(chunkX, chunkZ);
    ChunkVBOdata vbo = chunk->generateVBOdata();
    // update the one in the map
    vbo.mp_chunk->createVBOdata(vbo);
    // m_chunkVBOs[chunk.get()] = vbo;
    // chunk->createVBOdata(vbo);

}


void Terrain::CreateTestScene()
{

    int size = 64;
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < size; x += 16) {
        for(int z = 0; z < size; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));

    // Create the basic terrain floor
    for(int x = 0; x < size; ++x) {
        for(int z = 0; z < size; ++z) {
            if((x + z) % 2 == 0) {
                setBlockAt(x, 128, z, STONE);
            }
            else {
                setBlockAt(x, 128, z, DIRT);
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < size; ++x) {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    // Add a central column
    for(int y = 129; y < 140; ++y) {
        setBlockAt(32, y, 32, GRASS);
    }
}

void Terrain::CreateTestGrassScene()
{
    Noise terrainHeightMap;

    int xMax = 128;
    int zMax = 128;

    // Create the Chunks that will store the blocks for our initial world space
    for(int x = 0; x < xMax; x += 16) {
        for(int z = 0; z < zMax; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that the "generated terrain zone" at (0,0) now exists.
    m_generatedTerrain.insert(toKey(0, 0));

    // Create the basic terrain floor
    for(int x = 0; x < xMax; ++x) {
        for(int z = 0; z < zMax; ++z) {

            double y = terrainHeightMap.getHeight(x,z);
            y = std::max(128.0, y);

            if( y < 136){
                setBlockAt(x, y, z, WATER);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    setBlockAt(x, y_dirt, z, WATER);
                }
            }

            else if( y < 142){
                setBlockAt(x, y, z, GRASS);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    setBlockAt(x, y_dirt, z, DIRT);
                }
            }
            else if (y > 190){
                setBlockAt(x, y, z, SNOW);
                for(int y_stone=128; y_stone<y; y_stone++){
                    setBlockAt(x, y_stone, z, STONE);
                }
            }
            else{
                setBlockAt(x, y, z, STONE);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    setBlockAt(x, y_dirt, z, DIRT);
                }
            }

            // Set DIRT from 0-128 height values
            for(int y_underground=0; y_underground<128;y_underground++){
                setBlockAt(x, y_underground, z, DIRT);
            }

        }
    }
}


/**
 * @brief Terrain::spawnFillBlocksWorker
 * @param x
 * @param z
 */
void Terrain::spawnFillBlocksWorker(int x, int z)
{
    Chunk *chunk = instantiateChunkAt(x, z);
    FillBlocksWorker *worker = new FillBlocksWorker(x, z,
                                                    chunk,
                                                    &m_chunksWithBlocks,
                                                    &m_chunksWithBlocksLock);
    QThreadPool::globalInstance()->start(worker);
}


/**
 * @brief Terrain::spawnVBOWorker
 * @param mp_chunk
 */
void Terrain::spawnVBOWorker(Chunk* mp_chunk)
{
    VBOWorker *worker = new VBOWorker(mp_chunk,
                                      &m_chunksWithVBOs,
                                      &m_chunksWithVBOsLock);
    QThreadPool::globalInstance()->start(worker);
}


/**
 * @brief Terrain::spawnVBOWorkers
 * @param completedChunksWithBlocks
 */
void Terrain::spawnVBOWorkers(const std::unordered_set<Chunk*> &completedChunksWithBlocks)
{
    for (Chunk *ck : completedChunksWithBlocks) {
        spawnVBOWorker(ck);
    }
}

//--------------------------
// Thread Workers
//--------------------------
FillBlocksWorker::FillBlocksWorker(int x,
                                   int z,
                                   Chunk *chunk,
                                   std::unordered_set<Chunk*> *completedChunks,
                                   QMutex *completedChunksLock)
    : xCorner(x), zCorner(z),
      chunk(chunk),
      completedChunks(completedChunks), completedChunksLock(completedChunksLock)
{}


/**
 * @brief FillBlocksWorker::run
 *  The run() to fill the blocks of a given region
 */
void FillBlocksWorker::run()
{
    // TODO: iterate through each chunks in the zone
    // TODO: iterate through (x, z) in a chunk
    // assign the block

    Noise terrainHeightMap;

    for (int x = 0; x < 16; x++) {

        for (int z = 0; z < 16; z++) {

            // TODO: wrap up the logics later
            double y = terrainHeightMap.getHeight(xCorner + x ,zCorner + z);

            // x, z need to be the offset (not the global x, z)

            if( y < 136){
                chunk->setBlockAt(x, y, z, WATER);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    chunk->setBlockAt(x, y_dirt, z, WATER);
                }
            }
            else if( y < 142){
                chunk->setBlockAt(x, y, z, GRASS);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    chunk->setBlockAt(x, y_dirt, z, DIRT);
                }
            }
            else if (y > 190){
                chunk->setBlockAt(x, y, z, SNOW);
                for(int y_stone=128; y_stone<y; y_stone++){
                    chunk->setBlockAt(x, y_stone, z, STONE);
                }
            }
            else{
                chunk->setBlockAt(x, y, z, STONE);
                for(int y_dirt=128; y_dirt<y; y_dirt++){
                    chunk->setBlockAt(x, y_dirt, z, DIRT);
                }
            }

            // Set DIRT from 0-128 height values
            for(int y_underground=0; y_underground<128;y_underground++){
                chunk->setBlockAt(x, y_underground, z, STONE);
            }

        }

    }

    // already filled the blocks
    // TODO: push to the collection of completed Chunks
    completedChunksLock->lock();
    completedChunks->insert(chunk);
    // neighbors to re-create vbo
    for (const std::pair<Direction, Chunk*> &p : chunk->getNeighbors()) {
        if (p.second != nullptr) {
            completedChunks->insert(p.second);
        }
    }
    completedChunksLock->unlock();
}


/**
 * @brief VBOWorker::VBOWorker
 * @param chunkWithoutVBO
 * @param completedChunkVBOs
 * @param completedChunkVBOsLock
 */
VBOWorker::VBOWorker(Chunk *chunkWithoutVBO,
                     std::vector<ChunkVBOdata> *completedChunkVBOs,
                     QMutex *completedChunkVBOsLock)
    : chunkWithoutVBO(chunkWithoutVBO),
      completedChunkVBOs(completedChunkVBOs),
      completedChunkVBOsLock(completedChunkVBOsLock)
{}


/**
 * @brief VBOWorker::run
 */
void VBOWorker::run()
{
    // create vbo
    ChunkVBOdata vbo = chunkWithoutVBO->generateVBOdata();
    completedChunkVBOsLock->lock();
    completedChunkVBOs->push_back(vbo);
    completedChunkVBOsLock->unlock();
}
