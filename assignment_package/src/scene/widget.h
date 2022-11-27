#pragma once
#include <unordered_map>
#include "drawable.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QApplication>
#include <QFile>
#include <iostream>

#ifndef WIDGET_H
#define WIDGET_H

class Widget : public Drawable
{
private:
    // top-left and bottom-right uv coordinate of widget
    // 0 <= x, y <= 1 (origin at bottom-left)
    std::array<glm::vec2, 2> widgetUVCoord;

    // top-left and bottom-right screen coordinate of widget
    // -1 <= x, y <= 1 (origin at center)
    std::array<glm::vec2, 2> widgetScreenCoord;

    // top-left and bottom-right uv coordinate of widget (optional)
    // 0 <= x, y <= 1 (origin at bottom-left)
    std::array<glm::vec2, 2> selectedFrameUVCoord;

    // top-left and bottom-right screen coordinate of selected frame (optional)
    // locate at the first frame (top-left)
    // -1 <= x, y <= 1 (origin at center)
    std::array<glm::vec2, 2> selectedFrameScreenCoord;

    // first vec: shift dist of selected frame (optional)
    // -1 <= x, y <= 1
    // second vec: size of the widget (unit: frame) (col and row) (optional)
    // 0 < x, y and are expected as integer
    std::array<glm::vec2, 2> frameShiftInfo;

    // used for loading coordinates from text file
    std::unordered_map<std::string, std::array<glm::vec2, 2>> widgetInfoMap = {
        {"widgetUV", widgetUVCoord},
        {"widgetScreen", widgetScreenCoord},
        {"selectedFrameUV", selectedFrameUVCoord},
        {"selectedFrameScreen", selectedFrameScreenCoord},
        {"frameShiftInfo", frameShiftInfo}
    };

    void pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec);
    void pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec);


public:
    Widget(OpenGLContext* context);
    void setupWidgetCoord(float uv_x, float uv_y, float screen_x, float screen_y);
    void setupSelectedFrameCoord(float uv_x, float uv_y, float screen_x, float screen_y);
    void setupSelectedFrameShiftInfo(float dist_x, float dist_y, int colLimit, int rowLimit);
    void loadCoordFromText(const char* text_path);
    void insertNewInfos(std::string infoType, std::array<glm::vec2, 2> infos);

    virtual void createVBOdata();
};

#endif // WIDGET_H
