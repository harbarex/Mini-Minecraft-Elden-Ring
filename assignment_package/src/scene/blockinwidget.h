#include "widget.h"

#ifndef BLOCKINWIDGET_H
#define BLOCKINWIDGET_H

class BlockInWidget : public Widget
{
protected:

    std::vector<glm::vec2> firstItemScreen;
public:
    BlockInWidget(OpenGLContext* context);

    virtual void createVBOdata();
};

#endif // BLOCKINWIDGET_H
