#include "inventory.h"

Inventory::Inventory()
    : selectedBlockPtr(0), max_blocks(64), blocksOnHandSize(9), blocksInInventorySize(blocksOnHandSize+32)
{
    initBlocks();
}

void Inventory::initBlocks() {
    blocksInInventory.clear();

    // init blocks in inventory
    for (int j = 0; j < blocksInInventorySize; ++j) {
        blocksInInventory.push_back(std::make_pair(EMPTY, 0));
    }
}

void Inventory::getItemInfo(std::vector<std::pair<BlockType, int>>* itemsInfo) {
    itemsInfo = &blocksInInventory;
}

void Inventory::setBlocks(std::vector<BlockType>& blockTypes, int count) {

    count = fmin(count, max_blocks);
    int inventory_count = fmin(blockTypes.size(), blocksInInventorySize);

    for (int j = 0; j < inventory_count; ++j) {
        blocksInInventory[j] = (std::make_pair(blockTypes[j], count));
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

int Inventory::getBlocksOnHandSize() {
    return blocksOnHandSize;
}

int Inventory::getBlocksInInventorySize() {
    return blocksInInventorySize;
}

bool Inventory::storeBlock(BlockType blockType) {    

    int firstEmptyIdxInInventory = -1;

    for (int i = 0; i < blocksInInventorySize; ++i) {
        if (blocksInInventory[i].first == blockType && blocksInInventory[i].second < max_blocks) {
            blocksInInventory[i].second += 1;
            return true;
        }

        if (firstEmptyIdxInInventory < 0 && Block::isEmpty(blocksInInventory[i].first)) {
            firstEmptyIdxInInventory = i;
        }
    }

    if (firstEmptyIdxInInventory >= 0) {
        blocksInInventory[firstEmptyIdxInInventory].first = blockType;
        blocksInInventory[firstEmptyIdxInInventory].second = 1;
        return true;
    }
    return false;
}

BlockType Inventory::placeBlock() {
    if (Block::isEmpty(blocksInInventory[selectedBlockPtr].first)) {
        return EMPTY;
    }

    blocksInInventory[selectedBlockPtr].second -= 1;
    BlockType targetBlockType = blocksInInventory[selectedBlockPtr].first;

    if (blocksInInventory[selectedBlockPtr].second == 0) {
        blocksInInventory[selectedBlockPtr].first = EMPTY;
    }

    return targetBlockType;
}

BlockType Inventory::changeSelectedBlock(int newBlockIdx) {
    if (newBlockIdx < 0 || newBlockIdx >= blocksOnHandSize) {
        return EMPTY;
    }

    selectedBlockPtr = newBlockIdx;

    return blocksInInventory[selectedBlockPtr].first;
}

BlockType Inventory::changeSelectedBlock() {
    int targetIdx = selectedBlockPtr + 1;

    if (targetIdx >= blocksOnHandSize) {
        targetIdx = 0;
    }

    return changeSelectedBlock(targetIdx);
}

bool Inventory::switchBlocks(int blockIdxFrom) {
    // TODO: fill the pos with same blockType first
    // if no available blocktype and no empty pos: return false

    return true;
}
