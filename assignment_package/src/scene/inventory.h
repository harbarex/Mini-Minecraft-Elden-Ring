#pragma once
#include <vector>
#include "block.h"

#ifndef INVENTORY_H
#define INVENTORY_H

enum class BlockSelectedState { inventory, hand };

/**
 * @brief The Inventory class
 *  Define a class that handles the inventory of blocks that player has
 */

class Inventory {

private:
    // which block that the player currently holds
    int selectedBlockPtr;

    // the maximum number of blocks in one element of inventory
    int max_blocks;

    // the size
    int blocksOnHandSize;
    int blocksInInventorySize;

    // the player can immediately grab the blocks
    std::vector<std::pair<BlockType, int>> blocksOnHand;

    // the player needs to access it via pressing i
    std::vector<std::pair<BlockType, int>> blocksInInventory;

    // check if the index is out of range
    bool checkIdx(int idx, BlockSelectedState state);

    // getter function of size
    int getBlocksOnHandSize();
    int getBlocksInInventorySize();

public:

    Inventory();

    // initialize the blocks on hand and those in inventory
    void initBlocks();

    // Assign blocks to the player
    // mainly used for debug or creative mode
    void setBlocks();
    void setBlocks(std::vector<BlockType> blockTypes, int count);

    // add destroyed block to inventory
    // Add to onhand first. Add to the inventory if onHand is full
    bool storeBlock(BlockType blockType);

    // return the selected block and subtract the block by 1
    BlockType putBlock();

    // change the selected block to given index and return the blocktype of new selected block
    // EMPTY is allowed
    BlockType changeSelectedBlock(int newBlockIdx);

    // the function would be activated if the user clicks the block either in inventory or on hand
    // by mouse when the inventory shows up
    // switch the block position between inventory and hand
    // return true if the action succeeds, and return false if the target is full
    bool switchBlocks(int blockIdxFrom, BlockSelectedState state);
};


#endif // INVENTORY_H
