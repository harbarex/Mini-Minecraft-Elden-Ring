
# Feature Implementation

## Game Engine Tick Function and Player Physics (Meng-Chuan Chang)

### Movement

While pressing the key associated with movement (WASDQE), we assign the acceleration to the corresponding direction. Also, we set the limit of velocity. The velocity would be fixed if the length of vector combined with the acceleration exceed the limit.

### Collision

If the player collides with the block, we would disable the velocity at the corresponding direction. To make the game run smoothly, we check the collision separately, and therefore the player can slide on the wall.

We would not disable the velocity that would make the player detach the collision surface

#### X and Z direction

Each block has 8 corners, and player occupies 2 blocks. Therefore, the overall number of corner would be 12 (8 times 2 and then subtracted by 4 repetitive corners).

For each corner, we use ray tracing to check if the corner hits the block or not. The direction of the ray starts from center to location of the corner

#### Y direction

We start from just two positions, the center of camera block and the bottom of player block. Then apply ray tracing only on y axis.

### Distroy / Create the block

We use ray tracing (gridMarch) to find out if there is any non-empty block given the player position and orientation.

For distroying block, we need to find out the location of hit block, set the block as EMPTY, and then reload VBO.

For creating block, we need to find out the location of hit block and its adjacent empty block on the face of hit block we point to with the revised version of gridMarch func.

#### wait time

We set a minimal waiting time (0.5 second) between every two creation and every two deletion.

### Mouse movement

#### Rotation

We treat the displacement of cursor on x and y axis on the screen as the angle change on world location. Also, we set a scalar to the conversion of displacement to angle to make the rotation more natural.

#### Issue in MacOS

In MacOS, there is an issue in QCursor::setPos. It only works on Mac at the first time. Therefore, we need to manually delete MiniMinecraft.app in accessibility everytime before running the program.


## Efficient Terrain Rendering and Chunking (Chun-Fu Yeh)

### Add block.h & block.cpp

To organize the needed information to render all the blocks in a given chunk, block.h and block.cpp are used to define the structs and the classes to store the information about a block, including block types, block faces, vertices, normals, colors, uvs. The rules to define a block as opaque or transparent are also kept here.

### Chunk as a drawable

The main task is to collect the vbo data of a chunk and keep vbo data in memory for rendering. During the collection of vbo data, if it is an opaque block, only the faces contacting with the air would be rendering, which means the vertices, normals, colors and uvs of that face are added to the vbo data. For the blocks on the edge of a chunk, the neighboring chunk will be retrieved to check the neighboring block of the blocks on the edge.

### Interleaved VBO

Since there's only one buffer array in addition to the index buffer array, only generatePos() & m_bufPos is needed. Then, bind the buffer data with the m_bufPos. In the shaderprogram.cpp, a drawInterleaved(Drawable &d) method is added to draw the buffer. Inside the method, the addresses of the start of each data (pos, normal, color, uv) are specified along with the stride needed to retrieve each info correctly.

### Terrain expansion

For expansion, every tick, the program checks whether the 3 x 3 chunks surrounding the player are instantiated and the vbo data of these chunks are created or not. If not, the program instantiate the chunk and create the vbo data for it. Here, there is a member variable, called m_ChunkVBOs, storing all the vbos of the loaded chunks. The terrain.draw(...) iterates through the grid and draws the chunk if its vbo data is available.


## Procedural Terrain Generation (Ankit Billa)

In order to get the procedurally generated heightmap for a particular `x` and `z` coordinate, variations of Perlin noise were used, wrapped in the `Noise` class. Subsequently, `noise.h` and `noise.cpp` contain private helper functions to generate height values for different biomes, in this case Grasslands, Mountains and Water Bodies. 

Grasslands: 
The grasslands terrain is generated using Worley Noise coupled with 2D Fractal Brownian Motion (FBM2D) for Perlin noise as suggested in the handout. 

Mountainous Rock:
The mountainous rock terrain is generated using the absolute value of FBM Perlin Noise.

Water Bodies:
Water bodies are added to the procedurally generated terrain, and the height value is obtained using averaged FBM Perlin Noise.

In order to have smooth transitions between the biomes, we utilize `glm::smoothstep` applied on averaged FBM Perlin Noise. This hastens the transitions from one biome to another.
 
The method used to get the procedurally generated height has the following signature:
```
int getHeight(int x, int z);
```

### 
