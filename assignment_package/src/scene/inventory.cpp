#include "inventory.h"

Inventory::Inventory()
    : selectedBlockPtr(0), max_blocks(64), blocksOnHandSize(10), blocksInInventorySize(64)
{
    initBlocks();
}

void Inventory::initBlocks() {
    // init blocks on hand
    blocksOnHand.clear();
    blocksInInventory.clear();

    for (int i = 0; i < blocksOnHandSize; ++i) {
        blocksOnHand.push_back(std::make_pair(EMPTY, 0));
    }
    // init blocks in inventory
    for (int j = 0; j < blocksInInventorySize; ++j) {
        blocksInInventory.push_back(std::make_pair(EMPTY, 0));
    }
}

void Inventory::setBlocks(std::vector<BlockType> blockTypes, int count) {

    count = fmin(count, max_blocks);

    if ((int)blockTypes.size() < blocksOnHandSize) {
        for (int i = 0; i < (int)blockTypes.size(); ++i) {
            blocksOnHand[i] = std::make_pair(blockTypes[i], count);
        }
        return;
    }

    for (int i = 0; i < blocksOnHandSize; ++i) {
        blocksOnHand[i] = std::make_pair(blockTypes[i], count);
    }

    int inventory_count = fmin(blockTypes.size() - blocksOnHandSize, blocksInInventorySize);

    for (int j = 0; j < inventory_count; ++j) {
        blocksInInventory[j] = (std::make_pair(blockTypes[j+blocksOnHandSize], count));
    }

    return;
}

void Inventory::setBlocks() {
    std::vector<BlockType> blockTypes;
    blockTypes.push_back(STONE);
    blockTypes.push_back(GRASS);
    blockTypes.push_back(ICE);
    blockTypes.push_back(WOOD);
    blockTypes.push_back(LEAF);
    blockTypes.push_back(SNOW);
    blockTypes.push_back(DIRT);

    setBlocks(blockTypes, max_blocks);
}

bool Inventory::checkIdx(int idx, BlockSelectedState state) {
    if (idx < 0) {
        return false;
    }

    switch (state) {
    case (BlockSelectedState::inventory):
        if (idx >= blocksInInventorySize) {
            return false;
        }
        break;
    case (BlockSelectedState::hand):
        if (idx >= blocksOnHandSize) {
            return false;
        }
        break;
    }

    return true;
}

int Inventory::getBlocksOnHandSize() {
    return blocksOnHandSize;
}

int Inventory::getBlocksInInventorySize() {
    return blocksInInventorySize;
}

bool Inventory::storeBlock(BlockType blockType) {
    return true;
}

BlockType Inventory::putBlock() {
    return EMPTY;
}

BlockType Inventory::changeSelectedBlock(int newBlockIdx) {
    if (!checkIdx(newBlockIdx, BlockSelectedState::hand)) {
        return EMPTY;
    }

    selectedBlockPtr = newBlockIdx;

    return blocksOnHand[selectedBlockPtr].first;
}

bool Inventory::switchBlocks(int blockIdxFrom, BlockSelectedState state) {
    if (!checkIdx(blockIdxFrom, state)) {
        return false;
    }

    return true;
}
