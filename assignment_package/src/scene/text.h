#pragma once
#include "drawable.h"
#include <unordered_map>
#ifndef TEXT_H
#define TEXT_H

// helper function for storing all vbo related data for text
struct TextData
{
    // each TextData contains a vertex position and a UV coord
    glm::vec2 pos;
    // later, the uv coordinate is manually set
    glm::vec2 uv;

    TextData(glm::vec2 p, glm::vec2 u)
        : pos(p), uv(u) {}
    TextData()
        : pos(), uv() {}

};

class Text : public Drawable
{
protected:
    // all texts drawn on the screen
    std::vector<std::array<TextData, 4>> texts;

public:
    Text(OpenGLContext* context);

    // a collection of all the uvs of all types of texts
    // order of uv: bottom-left, bottom-right, top-right, top-left
    std::unordered_map<std::string, std::array<glm::vec2, 4>> TextCollection;

    // load uv coordinate of all texts from text file
    bool loadUVCoordFromText(const char* text_path);

    // draw the text given text info and the top-left coordinate, and the height of the text
    // -1 <= pos.x, pos.y < 1
    // 0 < height < 2, associated with the height of the screen
    bool addText(std::string text, glm::vec2 pos, float height);

    virtual void createVBOdata();
};

#endif // TEXT_H
