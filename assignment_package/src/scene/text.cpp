#include "text.h"

Text::Text(OpenGLContext *context, float width, float height)
    : Drawable(context), width_height_len(glm::vec2(12.f/256.f, 16.f/256.f)), width_height_screen_ratio(width/height)
{}

void Text::insertNewInfo(std::string currText, glm::vec2 currCoord) {
    if (currText == "textWidthHeight") {
        width_height_len = currCoord;
        return;
    }

    // order of uv: bottom-left, bottom-right, top-right, top-left
    TextCollection[currText][0] = glm::vec2(currCoord[0] * width_height_len[0], currCoord[1] * width_height_len[1]);
    TextCollection[currText][1] = TextCollection[currText][0] + glm::vec2(width_height_len[0], 0.f);
    TextCollection[currText][2] = TextCollection[currText][0] + width_height_len;
    TextCollection[currText][3] = TextCollection[currText][0] + glm::vec2(0, width_height_len[1]);
    return;
}

/**
 * @brief Text::loadUVCoordFromTex
 *  load uv coordinate of all texts from text file
 * @param text_path : local path of text file (need to add the path in qrc file)
 * @return
 */
bool Text::loadUVCoordFromText(const char* text_path) {
    // read text file
    QFile f(text_path);
    f.open(QIODevice::ReadOnly);
    QTextStream s(&f);
    QString line;
    bool readCoord = false;
    int coordCount = 0;
    int maxCoordCount = 1;
    glm::vec2 currCoord;
    std::string currText;

    while (!s.atEnd()) {
        line = s.readLine();
        if (line.size() == 0 || line.startsWith("#")) {
            // skip empty line and line starts with #
            continue;
        }

        if (readCoord) {
            // store uv coordinate into TextCollection
            currCoord = glm::vec2(line.split(" ")[0].toDouble(), line.split(" ")[1].toDouble());
            coordCount += 1;
        } else {
            // identify which text is now processing
            currText = line.toStdString();
            readCoord = true;
        }

        if (coordCount == maxCoordCount) {
            coordCount = 0;
            insertNewInfo(currText, currCoord);
            readCoord = false;
        }
    }

    return true;
}

void Text::resizeDimension(float width, float height) {
    width_height_screen_ratio = width/height;
    return;
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
