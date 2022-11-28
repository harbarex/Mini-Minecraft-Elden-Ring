#include "widget.h"
#include "inventory.h"

#ifndef BLOCKINWIDGET_H
#define BLOCKINWIDGET_H

class BlockInWidget : public Widget
{
public:
    BlockInWidget(OpenGLContext* context);
    // we draw the item with uv provided by player, usually for block drawing
    void addItemFromInventory(int overallShiftIdx, Inventory& inventory);
    virtual void createVBOdata();
};

#endif // BLOCKINWIDGET_H
