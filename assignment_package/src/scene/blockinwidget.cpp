#include "blockinwidget.h"

BlockInWidget::BlockInWidget(OpenGLContext *context)
    : Widget(context)
{
    widgetInfoMap = {
        {"regionInfo", std::make_pair(regionInfo, 4)}
    };
}

void BlockInWidget::addItemFromInventory(int overallShiftIdx, Inventory& inventory) {

}

void BlockInWidget::createVBOdata() {

    std::vector<GLuint> indices;
    std::vector<GLuint> faceIndices = {0, 1, 2, 0, 2, 3};
    std::vector<float> buffer_pos;
    std::vector<float> buffer_uv;
    int nVert = 0;

    for (auto& drawItem : drawItems) {
        pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[0], 0.999999f, 1.f));
        pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[1], 0.999999f, 1.f));
        pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[2], 0.999999f, 1.f));
        pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[3], 0.999999f, 1.f));

        pushVec2ToBuffer(buffer_uv, drawItem[4]);
        pushVec2ToBuffer(buffer_uv, drawItem[5]);
        pushVec2ToBuffer(buffer_uv, drawItem[6]);
        pushVec2ToBuffer(buffer_uv, drawItem[7]);

        // add indices for each face (4 vertices)
        for (int index : faceIndices) {
            indices.push_back(nVert + index);
        }
        // move the offset for indices
        nVert += 4;
    }

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


    drawItems.clear();
}
