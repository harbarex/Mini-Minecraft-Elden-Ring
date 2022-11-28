#include "widget.h"
#include "inventory.h"

#ifndef BLOCKINWIDGET_H
#define BLOCKINWIDGET_H

class BlockInWidget : public Widget
{
protected:

public:
    BlockInWidget(OpenGLContext* context);
    // we draw the item with uv provided by player, usually for block drawing
    bool addItemFromInventory(int overallShiftIdx, Inventory& inventory);
    virtual void createVBOdata();
};

#endif // BLOCKINWIDGET_H
