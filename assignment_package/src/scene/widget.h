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
protected:
    // top-left and bottom-right uv coordinate of widget
    // 0 <= x, y <= 1 (origin at bottom-left)
    std::vector<glm::vec2> widgetUVCoord;

    // top-left and bottom-right screen coordinate of widget
    // -1 <= x, y <= 1 (origin at center)
    std::vector<glm::vec2> widgetScreenCoord;

    // top-left and bottom-right uv coordinate of widget (optional)
    // 0 <= x, y <= 1 (origin at bottom-left)
    std::vector<glm::vec2> selectedFrameUVCoord;

    // top-left and bottom-right screen coordinate of selected frame (optional)
    // locate at the first frame (top-left)
    // -1 <= x, y <= 1 (origin at center)
    std::vector<glm::vec2> selectedFrameScreenCoord;

    // first vec: shift dist of selected frame (optional)
    // -1 <= x, y <= 1
    // second vec: size of the widget (unit: frame) (col and row) (optional)
    // 0 < x, y and are expected as integer
    std::vector<glm::vec2> frameShiftInfo;

    // current shift info of selected frame
    glm::vec2 currShift;

    // used for loading coordinates from text file
    std::unordered_map<std::string, std::pair<std::vector<glm::vec2>, int>> widgetInfoMap;

    void pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec);
    void pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec);


public:
    Widget(OpenGLContext* context);
//    void setupWidgetCoord(float uv_x, float uv_y, float screen_x, float screen_y);
//    void setupSelectedFrameCoord(float uv_x, float uv_y, float screen_x, float screen_y);
//    void setupSelectedFrameShiftInfo(float dist_x, float dist_y, int colLimit, int rowLimit);
    void loadCoordFromText(const char* text_path);
    void insertNewInfos(std::string infoType, std::vector<glm::vec2> infos);
    bool setCurrShift(int x, int y);

    virtual void createVBOdata();
};

#endif // WIDGET_H
