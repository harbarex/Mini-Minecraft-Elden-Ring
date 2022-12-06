#include "inventory.h"

Inventory::Inventory()
    : selectedBlockPtr(0), max_blocks(1), blocksOnHandSize(9), blocksInInventorySize(blocksOnHandSize+27)
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

/**
 * @brief Inventory::getItemInfo
 *  get the item type and count given the index
 * @param overallIdx : int, the index of the item located in the inventory
 * @return itemType : BlockType, the type of the item
 * @return count : int, the count of the item
 */
void Inventory::getItemInfo(float overallIdx, BlockType* itemType, int* count) {
    *itemType = blocksInInventory[overallIdx].first;
    *count = blocksInInventory[overallIdx].second;
}

void Inventory::getItemInfo(std::vector<std::pair<BlockType, int>>* itemsInfo) {
    *itemsInfo = blocksInInventory;
}

/**
 * @brief Inventory::setBlock
 *  set the block at the specific location
 * @param overallIdx : int, the index of the item located in the inventory
 * @param blockType : BlockType, type of the item
 * @param count : int, count of the item
 */
void Inventory::setBlock(int overallIdx, BlockType& blockType, int count) {
    blocksInInventory[overallIdx] = (std::make_pair(blockType, count));
    return;
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

bool Inventory::switchItems(int overallIdxFrom, int overallIdxTo) {
    // TODO: switch the items in the same inventory

    return switchItems(overallIdxFrom, overallIdxTo, blocksInInventory[overallIdxFrom].first, blocksInInventory[overallIdxFrom].second);
}

bool Inventory::switchItems(int overallIdxFrom, int overallIdxTo, BlockType fromItemType, int fromItemCount) {

    if (fromItemType == blocksInInventory[overallIdxTo].first) {
        if (fromItemCount + blocksInInventory[overallIdxTo].second > max_blocks) {
            blocksInInventory[overallIdxFrom].second = fromItemCount + blocksInInventory[overallIdxTo].second - max_blocks;
            blocksInInventory[overallIdxTo].second = max_blocks;
        } else {
            blocksInInventory[overallIdxTo].second += blocksInInventory[overallIdxFrom].second;
            blocksInInventory[overallIdxFrom].first = EMPTY;
            blocksInInventory[overallIdxFrom].second = 0;
        }
        return true;
    }

    BlockType tempFrom = fromItemType;
    int tempFromCount = fromItemCount;
    blocksInInventory[overallIdxFrom].first = blocksInInventory[overallIdxTo].first;
    blocksInInventory[overallIdxFrom].second = blocksInInventory[overallIdxTo].second;
    blocksInInventory[overallIdxTo].first = tempFrom;
    blocksInInventory[overallIdxTo].second = tempFromCount;
    return true;
}

void Inventory::clearItem(int overallIdx) {
    blocksInInventory[overallIdx].first = EMPTY;
    blocksInInventory[overallIdx].second = 0;
}
