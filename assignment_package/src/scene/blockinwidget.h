#include "widget.h"
#include <unordered_set>

#ifndef BLOCKINWIDGET_H
#define BLOCKINWIDGET_H

class BlockInWidget : public Widget
{
public:
    BlockInWidget(OpenGLContext* context);
    ~BlockInWidget();
    // we draw the item with uv provided by player, usually for block drawing
    void addItem(int overallShiftIdx, std::array<glm::vec2, 4>& uvCoords);
    void addItem(glm::vec2 pos, glm::vec2 len, std::array<glm::vec2, 4>& uvCoords);
    // draw 3D blocks
    void addItem(int overallShiftIdx, std::array<std::array<glm::vec2, 4>, 3>& uvCoords);
    void addItem(glm::vec2 pos, glm::vec2 len, std::array<std::array<glm::vec2, 4>, 3>& uvCoords);
    void storeItemIntoDrawVector(RecRegion* currRegion, int shiftX, int shiftY, std::array<glm::vec2, 4>& uvCoords);
    // store 3D blocks
    void storeItemIntoDrawVector(RecRegion* currRegion, int shiftX, int shiftY, std::array<std::array<glm::vec2, 4>, 3>& uvCoords);
    void setWidgetInfo();
    virtual void createVBOdata();
};

#endif // BLOCKINWIDGET_H
