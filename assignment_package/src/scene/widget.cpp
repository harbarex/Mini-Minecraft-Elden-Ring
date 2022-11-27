#include "widget.h"

Widget::Widget(OpenGLContext *context) : Drawable(context)
{}

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
