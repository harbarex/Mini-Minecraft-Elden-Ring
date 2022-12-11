
# Mini-Minecraft - Elden Ring Edition

**Team: TheGoldenOrder - Ankit Billa, Chun-Fu Yeh, Meng-Chuan Chang**

This Mini-Minecraft project was implemented as a requirement for the final project of CIS 460/560, under Professor Mally, University of Pennsylvania, School of Engineering and Applied Sciences. The overall theme of the world is inspired from [Elden Ring](https://en.bandainamcoent.eu/elden-ring/elden-ring), with player & NPC skins, themed assets & soundtracks added to fit the setting.

## NPC & Path Finding (Chun-Fu Yeh)

### NPC Creation

Basically, the NPC class inherits Entity & Drawable and contains a scene graph. Each scene graph consists of three types of nodes (translation, rotation, scale nodes) and some nodes in the scene graph may point to a NPCBlock, containing the information of a part of the NPC (e.g. head, body, limbs, ... etc).

After the scene graph is built, the distances from the root to the six sides of the NPC must be defined, including rootToGround, rootToTop, rootToLeft, rootToRight, rootToFront, rootToBack. These would be used in determining the collisions against the blocks in the world.

Moreover, the rotation nodes of the limbs would be collected and the rotation degress in those nodes would be updated based on how far the NPCs move in the world. This leads to the walking / flying movements of NPCs.

### NPC Textures

In order to make the NPCs, the corresponding texture maps are downloaded and the UV coordinates for each face (XPOS, XNEG, YPOS, YNEG, ZPOS and ZNEG) are maually retrieved from the maps. These coordinates are hard coded in the textures/npc_uv_coord.txt file, which we load into memory in MyGL::initializedGL().

### NPCs In The World

Currently, we have four types of the NPCs, including Steve, Sheep, Lama and ZombieDragon.

Steve always makes plans to move closer to the player. A group of sheeps explore the world slowly.
As for two lamas, they are being trained to jump among a set of floating stones. At last, the zombie dragon is hovering in the sky and always taking the player as the center.

### NPC Movements

Firstly, the collision models are modified from the models in the player. The ray origins are calculated based on the scene graph root and the distances described above (rootToGround, rootToTop, rootToLeft, rootToRight, rootToFront, rootToBack).

Generally, the NPCs' mvoements are updated per tick.

### Path Finding Algorithm (A* search)

In order to make NPC moving toward some goals, an A* search algorithm is implemented. Here, the Euclidean distance in horizontal direction is used as the heuristics. Generally, this search algorithm starts with the NPC's current position. Each time, a potential path with lowest cost so far is popped out from the priority queue. Then, try to move 1 step further away if possible and push the new explored path to the priority queue. If there are obstacles around, it would try to jump 1 to 3 steps away and see if that is a valid move. 

The size of the search space is manually defined when the NPC are created. Different NPCs might have different search space. If the goal of each NPC is unable to achieve in any of the explored paths, a random path from top 10 lowest cost is sampled as the current path for the NPC.

### NPCs with Path Finder

With the path finding algorithm defined above, each NPC can now make the plans to the goals. Each tick, the NPC would check whether the goal is achieved or not. Then, it will check whether an action in the plan is done or not. If all the actions in a path are done, it will activate the path finder again to get a new path toward the goals.

Basically, there are only two movements, walk and jump, for the NPCs. If any NPC is stuck, the NPCs would be restarted. Some stuck conditions like penetrating the blocks or being stuck in a place for more than 15 seconds.


## Inventory (Meng-Chuan Chang)

### Abstract

The player can store the blocks into the inventory and access the blocks from the inventory.

### Window

There are two windows. 

#### On-Hand Window

The first window is always shown on the bottom of the screen. The player can place the blocks that store in the first window

#### In-Container Window

The player can open and close the second window via pressing key "I", and it can switch the place of stored items.
The bottom row of container window is on hand window. The player can move the blocks from container to hand or vice versa via dragging with the mouse.

### Block Accessibility

#### Place the block

When the player clicks right mouse key button, it would place the block stored in selected frame in on-hand window and subtract the count of the block by one.
If there is no block in selected frame. The player would not place any block.

#### Destroy the block

When the player clicks left mouse key button, the block that the player points to would be destroyed. 
During the destroy process, the destroyed block would be automatically stored to the inventory.
Some of the blocks would be transformed after the player destroys it (e.g. stone would be transformed to cobbleblock)

#### Debug mode

Once the user press key "P", all available blocks would be filled in the inventory.

### Block movement

#### Selected frame and place the block

In on-hand window, the player would place the block in the selected frame.
The player can change the selected frame via pressing number 1 to 9 on the keyboard.

#### Mouse dragging in container windows

While the player opens the container window. The player can move the block via the following steps

1. Move the mouse to the block you want to move
2. Press left mouse key button
3. Drag the mouse to the destination
4. Release the left mouse key button

If the destination does not have the position. The block would be moved back to original position

### Count

The maximum capacity of each position in the widget is 64. Once the block is full, the same type of block would be stored in the new position until there is no empty position in inventory.

## Texturing and Texture Animation (Meng-Chuan Chang)

### Load Texture

Load bottom-left uv coordinate of 6 faces of each block from text file, set uv coordinate for each vertex and pass it to GPU, and load texture 2D image in GPU.

### Split VBO

Split the vbo for transparent blocks and opaque blocks, respectively. Call draw function for two times in order to apply alpha blending to transparent block.

### Texture Animation

#### Animatable Flag

Pass animatable flag (1 or -1) to GPU and apply UV offset if the current processing block is animatable (which is 1)

#### Time

Set the processing number of frame as the time and pass it to GPU (u_Time). Apply u_Time with mod in uv offset for simulating the animation for animatable block (LAVA and WATER)

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

## Multithreaded Terrain Generation

### FillBlocksWorker

If a zone is not created, all the chunks inside this zone would be instantiated and the raw pointers of those chunks would be sent to FillBlocksWorker.

Each worker is responsible for the blocks in a given zone.

### VBOWorker

If the zone is already created and the zone is not part of the previous visted zones, the raw pointers of a zone would be sent to VBOWorker to create VBO data of this zone.

### Lock

Basically, whenever each worker is about to finish its work, it needs to retrieve the lock and push the raw pointers of the chunks or the VBO data into the collections (in shared memory).

### The Issue encountered when destroy VBOs

Each time, when doing the terrain expansion, some of the previously visited zones would be destroyed (their VBOs). If directly iterate through the chunks inside these zones, and then destroy the VBOs of those chunks without

checking whether those chunks have VBOs or not, we found that some errors might occur when the player quickly moved around 4 adjacent zones. 

More specifically, in that case, when the player quickly moved around 4 adjacent zones, the zones on the border are being frequently destroyed and created. 

The program might crash or get glsl operation error if the gpu is reading the vbo while the main thread is destroying those chunks.

Thus, the solution is to check whether those chunks' vbo is loaded or not before destroying it. (Only destroy the VBOs if they were loaded).

## Cave Systems (Ankit Billa)

### Cave Generation

3D Perlin Noise was used to procedurally generate the cave systems, which exist uniformly below the whole surface terrain. If the noise value returned from `getCaveHeight(x,y,z)` is less than `0`, we place `STONE` blocks, otherwise `LAVA` or `EMPTY` blocks are placed according to height.

### Post-Process Overlays

#### Underwater

To simulate being underwater, a simple fluid distortion noise was overlayed on the screen as soon as the player goes below a water block.

#### Under Lava

To simulate being under lava, a fluid distortion using a combination of multi-directional flows and worley noise was overlayed on the screen when a player goes below a lava block.

### Disable collision in liquid (Meng-Chuan Chang)

We would not set the velocity to 0 if the hit block is liquid (WATER, LAVA)

### velocity change (Meng-Chuan Chang)

Slow down the velocity in all direction if the player is in the liquid.
Keep assigning velocity while pressing space and the player is in the liquid.

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

#### Grasslands: 
The grasslands terrain is generated using Worley Noise coupled with 2D Fractal Brownian Motion (FBM2D) for Perlin noise as suggested in the handout. 

#### Mountainous Rock:
The mountainous rock terrain is generated using the absolute value of FBM Perlin Noise.

#### Water Bodies:
Water bodies are added to the procedurally generated terrain, and the height value is obtained using averaged FBM Perlin Noise.

In order to have smooth transitions between the biomes, we utilize `glm::smoothstep` applied on averaged FBM Perlin Noise. This hastens the transitions from one biome to another.
 
The method used to get the procedurally generated height has the following signature:
```
int getHeight(int x, int z);
```

### 
