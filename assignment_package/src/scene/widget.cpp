#include "widget.h"

Widget::Widget(OpenGLContext *context) : Drawable(context)
{}

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
    GLuint idx[6]{0, 1, 2, 0, 2, 3};
    glm::vec4 vert_pos[4] {glm::vec4(0.f, 0.f, 0.999999f, 1.f),
                           glm::vec4(1.f, 0.f, 0.999999f, 1.f),
                           glm::vec4(1.f, 1.f, 0.999999f, 1.f),
                           glm::vec4(0.f, 1.f, 0.999999f, 1.f)};

    glm::vec2 vert_UV[4] {glm::vec2(0.5f, 0.5f),
                          glm::vec2(1.f, 0.5f),
                          glm::vec2(1.f, 1.f),
                          glm::vec2(0.5f, 1.f)};

    m_count = 6;

    generateIdx();
    bindIdx();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    generatePos();
    bindPos();
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), vert_pos, GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), vert_UV, GL_STATIC_DRAW);
}
