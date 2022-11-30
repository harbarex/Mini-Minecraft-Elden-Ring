#pragma once
#include "smartpointerhelp.h"
#include "scene/npc.h"
#include <glm/gtx/matrix_transform_2d.hpp>

class NPCBlock;

class Node
{
protected:

    // children
    std::vector<uPtr<Node>> children;

public:

    // constructors
    Node();

    Node(NPCBlock *block);

    Node(const Node &node);


    // assign operator
    Node& operator=(const Node &node);

    /**
     * Setters & Getters
     **/
    // add child (also returns the reference of added child)
    virtual Node& addChild(uPtr<Node> node);

    const std::vector<uPtr<Node>>& getChildren();

    // compute & return transformation matrix (4 x 4)
    virtual glm::mat4 computeTransform(glm::mat4 transform) = 0;

    NPCBlock *block;

    // virtual destructor
    virtual ~Node();

};


class TranslateNode : public Node
{

private:

    glm::vec3 translate;

public:

    // constructors
    TranslateNode();

    TranslateNode(NPCBlock *block, glm::vec3 translate);

    TranslateNode(const TranslateNode &node);

    // assign
    TranslateNode& operator=(const TranslateNode &node);

    /**
     * Setters & Getters
     **/
    void setTranslate(glm::vec3 translate);
    glm::vec3 getTranslate() const;

    // compute & return transformation matrix (3 x 3)
    glm::mat4 computeTransform(glm::mat4 transform) override;

    // virtual destructor
    virtual ~TranslateNode();
};

class RotateNode : public Node
{

private:

    glm::vec3 rotAxis;

    // magnitude of rotation in degree
    float deg;

public:

    // constructors
    RotateNode();

    RotateNode(NPCBlock *block, glm::vec3 rotAxis, float degree);

    RotateNode(const RotateNode &node);

    // assign
    RotateNode& operator=(const RotateNode &node);

    /**
     * Setters & Getters
     **/
    void setDeg(float degree);

    float getDeg() const;

    void setAxis(glm::vec3 axis);

    glm::vec3 getAxis() const;

    // compute & return transformation matrix (3 x 3)
    glm::mat4 computeTransform(glm::mat4 transform) override;

    // virtual destructor
    virtual ~RotateNode();
};

class ScaleNode : public Node
{

private:

    glm::vec3 scale;

public:

    // constructors
    ScaleNode();

    ScaleNode(NPCBlock *block,
              glm::vec3 scale);

    ScaleNode(const ScaleNode &node);

    // assign
    ScaleNode& operator=(const ScaleNode &node);

    /**
     * Setters & Getters
     **/
    // compute & return transformation matrix (3 x 3)
    glm::mat4 computeTransform(glm::mat4 transform) override;

    // virtual destructor
    virtual ~ScaleNode();

};
