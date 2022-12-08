#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(-1), m_transparentCount(-1), m_bufIdx(), m_bufPos(), m_bufNor(), m_bufCol(), m_bufUV(), m_bufTransparentData(), m_bufTransparentIdx(),
      m_idxGenerated(false), m_posGenerated(false), m_norGenerated(false), m_colGenerated(false), m_uvGenerated(false),
      m_transparentDataGenerated(false), m_transparentIdxGenerated(false), mp_context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroyVBOdata()
{
    mp_context->glDeleteBuffers(1, &m_bufIdx);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);
    mp_context->glDeleteBuffers(1, &m_bufUV);
    mp_context->glDeleteBuffers(1, &m_bufTransparentData);
    mp_context->glDeleteBuffers(1, &m_bufTransparentIdx);
    m_idxGenerated = m_posGenerated = m_norGenerated = m_colGenerated = m_uvGenerated = false;
    m_transparentDataGenerated = m_transparentIdxGenerated = false;
    m_count = -1;
    m_transparentCount = -1;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_count;
}

int Drawable::transparentElemCount()
{
    return m_transparentCount;
}

void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

void Drawable::generateTransparentIdx()
{
    m_transparentIdxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufTransparentIdx
    mp_context->glGenBuffers(1, &m_bufTransparentIdx);
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

// for transparent vbo
void Drawable::generateTransparentData()
{
    m_transparentDataGenerated = true;
    // Create a VBO on our GPU and store transparent data in bufTransparentData
    mp_context->glGenBuffers(1, &m_bufTransparentData);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generateUV()
{
    m_uvGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufUV);
}

bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxGenerated;
}

bool Drawable::bindTransparentIdx()
{
    if(m_transparentIdxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufTransparentIdx);
    }
    return m_transparentIdxGenerated;
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindTransparentData()
{
    if(m_transparentDataGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufTransparentData);
    }
    return m_transparentDataGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindUV()
{
    if(m_uvGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    }
    return m_uvGenerated;
}

InstancedDrawable::InstancedDrawable(OpenGLContext *context)
    : Drawable(context), m_numInstances(0), m_bufPosOffset(-1), m_offsetGenerated(false)
{}

InstancedDrawable::~InstancedDrawable(){}

int InstancedDrawable::instanceCount() const {
    return m_numInstances;
}

void InstancedDrawable::generateOffsetBuf() {
    m_offsetGenerated = true;
    mp_context->glGenBuffers(1, &m_bufPosOffset);
}

bool InstancedDrawable::bindOffsetBuf() {
    if(m_offsetGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosOffset);
    }
    return m_offsetGenerated;
}


void InstancedDrawable::clearOffsetBuf() {
    if(m_offsetGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufPosOffset);
        m_offsetGenerated = false;
    }
}
void InstancedDrawable::clearColorBuf() {
    if(m_colGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufCol);
        m_colGenerated = false;
    }
}

/**
 * @brief pushVec4ToBuffer
 *  The helper func to push 4 elements into buffer array
 *  same as the code in chunk.cpp
 * @param buf
 * @param vec
 */
void Drawable::pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec)
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
void Drawable::pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec)
{
    for (int i = 0; i < 2; i++) {
        buf.push_back(vec[i]);
    }
}
