
# Feature Implementation

## Game Engine Tick Function and Player Physics

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
