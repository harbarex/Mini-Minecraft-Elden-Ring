#include "blockinwidget.h"

BlockInWidget::BlockInWidget(OpenGLContext *context)
    : Widget(context)
{
    widgetInfoMap = {
        {"firstItemScreen", std::make_pair(firstItemScreen, 2)},
        {"frameShiftInfo", std::make_pair(frameShiftInfo, 2)}
    };
}

void BlockInWidget::createVBOdata() {

}
