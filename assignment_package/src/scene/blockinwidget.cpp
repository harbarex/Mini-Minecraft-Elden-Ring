#include "blockinwidget.h"

BlockInWidget::BlockInWidget(OpenGLContext *context)
    : Widget(context)
{
    widgetInfoMap = {
        {"regionInfo", std::make_pair(regionInfo, 4)}
    };
}

void BlockInWidget::createVBOdata() {

}
