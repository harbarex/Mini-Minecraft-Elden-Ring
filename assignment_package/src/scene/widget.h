#pragma once
#ifndef WIDGET_H
#define WIDGET_H


#include "drawable.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Widget : public Drawable
{
public:
    Widget(OpenGLContext* context);
    virtual void createVBOdata();
};

#endif // WIDGET_H
