#pragma once

#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "chunk.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "shaderprogram.h"
#include "cube.h"
#include "utils.h"
#include <QMutex>
#include <QThreadPool>
#include "lsystems.h"


//using namespace std;

// Helper functions to convert (x, z) to and from hash map key
int64_t toKey(int x, int z);
glm::ivec2 toCoords(int64_t k);

// The container class for all of the Chunks in the game.
// Ultimately, while Terrain will always store all Chunks,
// not all Chunks will be drawn at any given time as the world
// expands.
class Terrain {
private:
    // Stores every Chunk according to the location of its lower-left corner
    // in world space.
    // We combine the X and Z coordinates of the Chunk's corner into one 64-bit int
    // so that we can use them as a key for the map, as objects like std::pairs or
    // glm::ivec2s are not hashable by default, so they cannot be used as keys.
    std::unordered_map<int64_t, uPtr<Chunk>> m_chunks;

    // Keep a collection of the to-do tasks for creating the vbos of the chunks
    // Note: the chunks in this collection must be filled with the blocks
    std::unordered_set<Chunk*> m_chunksWithBlocks;
    // the lock for the read / write to the m_chunksWithBlocks
    QMutex m_chunksWithBlocksLock;

    // Keep a collection of the to-do tasks for sending vbos to gpu
    std::vector<ChunkVBOdata> m_chunksWithVBOs;
    // the lock for the read / write to the m_chunksWithVBOs
    QMutex m_chunksWithVBOsLock;

    // private helpers for workers
    // Note: (x, z) is zone's (xCorner, zCorner)
    void spawnFillBlocksWorker(int x, int z);
    void spawnVBOWorker(Chunk* mp_chunk);
    void spawnVBOWorkers(const std::unordered_set<Chunk*> &completedChunksWithBlocks);

    // We will designate every 64 x 64 area of the world's x-z plane
    // as one "terrain generation zone". Every time the player moves
    // near a portion of the world that has not yet been generated
    // (i.e. its lower-left coordinates are not in this set), a new
    // 4 x 4 collection of Chunks is created to represent that area
    // of the world.
    // The world that exists when the base code is run consists of exactly
    // one 64 x 64 area with its lower-left corner at (0, 0).
    // When milestone 1 has been implemented, the Player can move around the
    // world to add more "terrain generation zone" IDs to this set.
    // While only the 3 x 3 collection of terrain generation zones
    // surrounding the Player should be rendered, the Chunks
    // in the Terrain will never be deleted until the program is terminated.
    std::unordered_set<int64_t> m_generatedTerrain;

    // this set represents the currently loaded 5 x 5 zones
    std::unordered_set<int64_t> m_prevBorderZones;

    void destroyZoneVBOs(int xCorner, int zCorner);

    OpenGLContext* mp_context;

public:
    Terrain(OpenGLContext *context);
    ~Terrain();

    // Instantiates a new Chunk and stores it in
    // our chunk map at the given coordinates.
    // Returns a pointer to the created Chunk.
    Chunk* instantiateChunkAt(int x, int z);
    // Do these world-space coordinates lie within
    // a Chunk that exists?
    bool hasChunkAt(int x, int z) const;
    // Assuming a Chunk exists at these coords,
    // return a mutable reference to it
    uPtr<Chunk>& getChunkAt(int x, int z);
    // Assuming a Chunk exists at these coords,
    // return a const reference to it
    const uPtr<Chunk>& getChunkAt(int x, int z) const;
    // Given a world-space coordinate (which may have negative
    // values) return the block stored at that point in space.
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAt(glm::vec3 p) const;
    // Given a world-space coordinate (which may have negative
    // values) set the block at that point in space to the
    // given type.
    void setBlockAt(int x, int y, int z, BlockType t);

    // add additional helper to check whether the block exist or not
    bool hasBlockAt(glm::vec3 p) const;

    // Draws every Chunk that falls within the bounding box
    // described by the min and max coords, using the provided
    // ShaderProgram
    void draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram, TerrainDrawType drawType);
    // custom draw to
    // draw the chunks around the player at (playerX, playerZ)
    // with a defined halfGridSize
    // the side of the grid is (1 + 2 * halfGridSize) chunks
    void draw(float playerX, float playerZ, int halfGridSize, ShaderProgram *shaderProgram, TerrainDrawType drawType);

    // Initializes the Chunks that store the 64 x 256 x 64 block scene you
    // see when the base code is run.
    void CreateTestScene();
    void CreateTestGrassScene();

    void drawErdtree(const glm::ivec2);

    // Terrain expansion that instantiate the Chunks (including the blocks inside)
    // around the player.
    void instantiateChunkAndfillBlocks(int chunkX, int chunkZ);
    void expand(float playerX, float playerZ, int halfGridSize);

    // for initial terrain
    bool m_initialTerrainLoaded;
    void loadInitialTerrain(float playerX, float playerZ, int halfGridSize);

    // check thread result
    // send the result from FillBlocksWorkers to VBOWorkers
    void checkThreadResults();



    // for player to destroy & add blocks
    void placeBlockAt(int x, int y, int z, BlockType t);
};


// Worker to fill the blocks
class FillBlocksWorker : public QRunnable
{
private:
    // TODO: other biome attrubites can be added here
    // TODO: zone attributes
    // TODO: wrap the height mapping logic in setBlocks
    int xCorner;
    int zCorner;
    std::unordered_map<int64_t, Chunk*> chunks;
    std::unordered_set<Chunk*> *completedChunks;
    QMutex *completedChunksLock;

    // helper to set the blocks of each chunk
    void setSurfaceTerrain(Chunk *chunk, int chunkCornerX, int x, int chunkCornerZ, int z, int height);
    void setFloatingTerrain(Chunk *chunk, int chunkCornerX, int x, int chunkCornerZ, int z, int height);
    void setBlocks(Chunk *chunk, int chunkXCorner, int chunkZCorner);

    void drawTree(Chunk* chunk, const glm::ivec2);

public:
    // constructor
    // Note: completedChunks == m_chunksWithBlocks (in terrain)
    FillBlocksWorker(int x,
                     int z,
                     std::unordered_map<int64_t, Chunk*> chunks,
                     std::unordered_set<Chunk*> *completedChunks,
                     QMutex *completedChunksLock);

    // run()
    void run() override;
};


// Worker to create vbo
class VBOWorker : public QRunnable
{
private:
    Chunk *chunkWithoutVBO;
    std::vector<ChunkVBOdata> *completedChunkVBOs;
    QMutex *completedChunkVBOsLock;

public:
    // constructor
    // Note: completedChunksVBOs == m_chunksWithVBOs (in terrain);
    VBOWorker(Chunk *chunkWithoutVBO,
              std::vector<ChunkVBOdata> *completedChunkVBOs,
              QMutex *completedChunkVBOsLock);

    // run()
    void run() override;
};
