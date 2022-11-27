#include "widget.h"

Widget::Widget(OpenGLContext *context) : Drawable(context)
{}

/**
 * @brief pushVec4ToBuffer
 *  The helper func to push 4 elements into buffer array
 *  same as the code in chunk.cpp
 * @param buf
 * @param vec
 */
void Widget::pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec)
{
    for (int i = 0; i < 4; i++) {
        buf.push_back(vec[i]);
    }
}


/**
 * @brief pushVec2ToBuffer
 *   The helper func to push 2 elements into buffer array
 *   same as the code in chunk.cpp
 * @param buf
 * @param vec
 */
void Widget::pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec)
{
    for (int i = 0; i < 2; i++) {
        buf.push_back(vec[i]);
    }
}

void Widget::insertNewInfos(std::string infoType, std::array<glm::vec2, 2> infos) {
    for (int i = 0; i < (int)infos.size(); ++i) {
        widgetInfoMap[infoType][i] = infos[i];
    }
    return;
}

void Widget::loadCoordFromText(const char* text_path) {
    // read text file
    QFile f(text_path);
    f.open(QIODevice::ReadOnly);
    QTextStream s(&f);
    QString line;
    bool isReadInfo = false;
    int dataReadCount = 0;
    int dataReadCountMax = 2;
    std::array<glm::vec2, 2> infos;
    std::string currInfoType;

    while (!s.atEnd()) {
        line = s.readLine();
        if (line.size() == 0 || line.startsWith("#")) {
            // skip empty line and line starts with #
            continue;
        }

        if (isReadInfo) {
            // store uv coordinate into temp array
            infos[dataReadCount] = glm::vec2(line.split(" ")[0].toDouble(), line.split(" ")[1].toDouble());
            dataReadCount += 1;
        } else if (widgetInfoMap.find(line.toStdString()) != widgetInfoMap.end()) {
            // identify which block
            currInfoType = line.toStdString();
            isReadInfo = true;
        }

        if (dataReadCount == dataReadCountMax) {
            // store 2 uv coordinates into Widget object
            insertNewInfos(currInfoType, infos);
            infos.empty();
            dataReadCount = 0;
            isReadInfo = false;
        }
    }
}

void Widget::createVBOdata(){

    int loadWidgetCount = 2;

    std::vector<GLuint> indices;
    std::vector<GLuint> faceIndices = {0, 1, 2, 0, 2, 3};
    std::vector<float> buffer_pos;
    std::vector<float> buffer_uv;
    int nVert = 0;

    for (int i = 0; i < loadWidgetCount; ++i) {
        // add indices for each face (4 vertices)
        for (int index : faceIndices) {
            indices.push_back(nVert + index);
        }
        // move the offset for indices
        nVert += 4;
    }

    // position of widget
    glm::vec2 topLeftPos = widgetInfoMap["widgetScreen"][0];
    glm::vec2 bottomRightPos = widgetInfoMap["widgetScreen"][1];
    glm::vec2 topRightPos(widgetInfoMap["widgetScreen"][1][0], widgetInfoMap["widgetScreen"][0][1]);
    glm::vec2 bottomLeftPos(widgetInfoMap["widgetScreen"][0][0], widgetInfoMap["widgetScreen"][1][1]);

    pushVec4ToBuffer(buffer_pos, glm::vec4(bottomLeftPos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(bottomRightPos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(topRightPos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(topLeftPos, 0.999999f, 1.f));

    // uv of widget
    glm::vec2 topLeftUV = widgetInfoMap["widgetUV"][0];
    glm::vec2 bottomRightUV = widgetInfoMap["widgetUV"][1];
    glm::vec2 topRightUV(widgetInfoMap["widgetUV"][1][0], widgetInfoMap["widgetUV"][0][1]);
    glm::vec2 bottomLeftUV(widgetInfoMap["widgetUV"][0][0], widgetInfoMap["widgetUV"][1][1]);

    pushVec2ToBuffer(buffer_uv, bottomLeftUV);
    pushVec2ToBuffer(buffer_uv, bottomRightUV);
    pushVec2ToBuffer(buffer_uv, topRightUV);
    pushVec2ToBuffer(buffer_uv, topLeftUV);

    // position of selected frame
    glm::vec2 topLeftFramePos = widgetInfoMap["selectedFrameScreen"][0];
    glm::vec2 bottomRightFramePos = widgetInfoMap["selectedFrameScreen"][1];
    glm::vec2 topRightFramePos(widgetInfoMap["selectedFrameScreen"][1][0], widgetInfoMap["selectedFrameScreen"][0][1]);
    glm::vec2 bottomLeftFramePos(widgetInfoMap["selectedFrameScreen"][0][0], widgetInfoMap["selectedFrameScreen"][1][1]);

    pushVec4ToBuffer(buffer_pos, glm::vec4(bottomLeftFramePos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(bottomRightFramePos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(topRightFramePos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(topLeftFramePos, 0.999999f, 1.f));

    // uv of selected frame
    glm::vec2 topLeftFrameUV = widgetInfoMap["selectedFrameUV"][0];
    glm::vec2 bottomRightFrameUV = widgetInfoMap["selectedFrameUV"][1];
    glm::vec2 topRightFrameUV(widgetInfoMap["selectedFrameUV"][1][0], widgetInfoMap["selectedFrameUV"][0][1]);
    glm::vec2 bottomLeftFrameUV(widgetInfoMap["selectedFrameUV"][0][0], widgetInfoMap["selectedFrameUV"][1][1]);

    pushVec2ToBuffer(buffer_uv, bottomLeftFrameUV);
    pushVec2ToBuffer(buffer_uv, bottomRightFrameUV);
    pushVec2ToBuffer(buffer_uv, topRightFrameUV);
    pushVec2ToBuffer(buffer_uv, topLeftFrameUV);

    m_count = indices.size();

    generateIdx();
    bindIdx();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    generatePos();
    bindPos();
    mp_context->glBufferData(GL_ARRAY_BUFFER, buffer_pos.size() * sizeof(float), buffer_pos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, buffer_uv.size() * sizeof(float), buffer_uv.data(), GL_STATIC_DRAW);

}
