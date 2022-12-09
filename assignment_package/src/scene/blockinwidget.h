#include "widget.h"

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
    void storeItemIntoDrawVector(RecRegion* currRegion, int shiftX, int shiftY, std::array<glm::vec2, 4>& uvCoords);
    void setWidgetInfo();
    virtual void createVBOdata();
};

#endif // BLOCKINWIDGET_H
