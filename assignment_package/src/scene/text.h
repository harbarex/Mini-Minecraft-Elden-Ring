#pragma once
#include "drawable.h"
#include <unordered_map>
#include <QApplication>
#include <QFile>
#include <iostream>
#ifndef TEXT_H
#define TEXT_H

// helper function for storing all vbo related data for text
struct TextData
{
    // each TextData contains a vertex position and a UV coord
    glm::vec4 pos;
    // later, the uv coordinate is manually set
    glm::vec2 uv;

    TextData(glm::vec2 p, glm::vec2 u)
        : pos(glm::vec4(p, 0.999999f, 1.f)), uv(u) {}
    TextData()
        : pos(), uv() {}

};

class Text : public Drawable
{
protected:
    // for computing width pos based on the height
    float width_height_screen_ratio;
    // determine the width relative to height loaded from text file
    glm::vec2 width_height_len;

    // all texts drawn on the screen
    std::vector<std::array<TextData, 4>> texts;

    // compute the uv in 4 coordinates based on uv on top-left and width_height_text_ratio
    void insertNewInfo(std::string text, glm::vec2 uv_topleft);

public:
    Text(OpenGLContext* context, float width, float height);
    ~Text();

    // a collection of all the uvs of all types of texts
    // order of uv: bottom-left, bottom-right, top-right, top-left
    std::unordered_map<char, std::array<glm::vec2, 4>> TextCollection;

    // load uv coordinate of all texts from text file
    bool loadUVCoordFromText(const char* text_path);

    // draw the text given text info and the top-left coordinate, and the height of the text
    // -1 <= pos.x, pos.y < 1
    // 0 < height < 2, associated with the height of the screen
    bool addText(std::string text, glm::vec2 pos, float height);

    void resizeDimension(float width, float height);

    virtual void createVBOdata();
};

#endif // TEXT_H
