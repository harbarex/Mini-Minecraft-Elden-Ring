#include "text.h"

Text::Text(OpenGLContext *context)
    : Drawable(context)
{}

/**
 * @brief Text::loadUVCoordFromTex
 *  load uv coordinate of all texts from text file
 * @param text_path : local path of text file (need to add the path in qrc file)
 * @return
 */
bool Text::loadUVCoordFromText(const char* text_path) {
    return true;
}

/**
 * @brief Text::addText
 *  store text info in text object for further drawing
 * @param text : string, the text itself
 * @param
 * @return
 */
// draw the text given text info and the top-left coordinate, and the height of the text
// -1 <= pos.x, pos.y < 1
// 0 < height < 2, associated with the height of the screen
bool Text::addText(std::string text, glm::vec2 pos, float height) {
    return true;
}

void Text::createVBOdata() {
    return;
}
