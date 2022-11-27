#pragma once
#ifndef WIDGET_H
#define WIDGET_H


#include "drawable.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Widget : public Drawable
{
private:
    // top-left and bottom-right uv coordinate of widget
    // 0 <= x, y <= 1 (origin at bottom-left)
    glm::vec2 topLeftWidgetUVCoord;
    glm::vec2 bottomRightWidgetUVCoord;

    // top-left and bottom-right screen coordinate of widget
    // -1 <= x, y <= 1 (origin at center)
    glm::vec2 topLeftWidgetScreenCoord;
    glm::vec2 bottomRightWidgetScreenCoord;

    // top-left and bottom-right uv coordinate of widget (optional)
    // 0 <= x, y <= 1 (origin at bottom-left)
    glm::vec2 topLeftFrameSelectedUVCoord;
    glm::vec2 bottomRightFrameSelectedUVCoord;

    // top-left and bottom-right screen coordinate of selected frame (optional)
    // locate at the first frame (top-left)
    // -1 <= x, y <= 1 (origin at center)
    glm::vec2 topLeftFrameSelectedScreenCoord;
    glm::vec2 bottomRightFrameSelectedScreenCoord;

    // shift dist of selected frame (optional)
    // -1 <= x, y <= 1
    glm::vec2 boxShiftDist;

    // size of the widget (unit: frame) (optional)
    int colLimit;
    int rowLimit;

public:
    Widget(OpenGLContext* context);
    void setupWidgetCoord(float uv_x, float uv_y, float screen_x, float screen_y);
    void setupSelectedFrameCoord(float uv_x, float uv_y, float screen_x, float screen_y);
    void setupSelectedFrameShiftInfo(float dist_x, float dist_y, int colLimit, int rowLimit);
    void loadUVCoordFromText();

    virtual void createVBOdata();
};

#endif // WIDGET_H
