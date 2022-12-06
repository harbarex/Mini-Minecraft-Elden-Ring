#pragma once
#include <vector>
#include "block.h"

#ifndef INVENTORY_H
#define INVENTORY_H

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
    // only on hand
    int blocksOnHandSize;
    // include on hand and in box
    int blocksInInventorySize;

    // the blocks that the player currently holds (including in box)
    std::vector<std::pair<BlockType, int>> blocksInInventory;

public:

    Inventory();

    // initialize the blocks on hand and those in inventory
    void initBlocks();

    void getItemInfo(float overallIdx, BlockType* itemType, int* count);
    void getItemInfo(std::vector<std::pair<BlockType, int>>* itemsInfo);

    // Assign blocks to the player
    // mainly used for debug or creative mode
    void setBlocks();
    void setBlock(int overallIdx, BlockType& blockType, int count);
    void setBlocks(std::vector<BlockType>& blockTypes, int count);

    // add destroyed block to inventory
    // Add to onhand first. Add to the inventory if onHand is full
    bool storeBlock(BlockType blockType);

    // return the selected block and subtract the block by 1
    BlockType placeBlock();

    // clear the block at specific index
    void clearItem(int overallIdx);

    // change the selected block to given index and return the blocktype of new selected block
    // EMPTY is allowed
    // default: move to the next block
    BlockType changeSelectedBlock();
    BlockType changeSelectedBlock(int newBlockIdx);

    // switch the items in the same inventory
    bool switchItems(int overallIdxFrom, int overallIdxTo);
    bool switchItems(int overallIdxFrom, int overallIdxTo, BlockType fromItemType, int fromItemCount);

    // getter function of size
    int getBlocksOnHandSize();
    int getBlocksInInventorySize();
};


#endif // INVENTORY_H
