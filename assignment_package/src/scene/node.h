#pragma once
#include "smartpointerhelp.h"
#include "block.h"
#include <QTreeWidgetItem>
#include <glm/gtx/matrix_transform_2d.hpp>


class Node : public QTreeWidgetItem
{
protected:

    // color for the Polygon2D
    glm::vec3 color;

    // children
    std::vector<uPtr<Node>> children;

    // name of the node (for GUI)
    QString name;

    Block *block;

public:

    // constructors
    Node();

    Node(glm::vec3 color, QString name, Block *block);

    Node(const Node &node);


    // assign operator
    Node& operator=(const Node &node);

    /**
     * Setters & Getters
     **/
    // for name
    QString getName() const;

    // for color
    void setColor(glm::vec3 newColor);
    glm::vec3 getColor() const;


    // add child (also returns the reference of added child)
    virtual Node& addChild(uPtr<Node> node);

    const std::vector<uPtr<Node>>& getChildren();

    // compute & return transformation matrix (3 x 3)
    virtual glm::mat3 computeTransform(glm::mat3 transform) = 0;

    // virtual destructor
    virtual ~Node();

};


class TranslateNode : public Node
{

private:

    // translation in the X direction
    float tx;

    // translation in the Y direction
    float ty;

public:

    // constructors
    TranslateNode();

    TranslateNode(glm::vec3 color,
                  QString name,
                  Block *block,
                  float translateX,
                  float translateY);

    TranslateNode(const TranslateNode &node);

    // assign
    TranslateNode& operator=(const TranslateNode &node);

    /**
     * Setters & Getters
     **/
    void setTx(float translateX);

    void setTy(float translateY);

    float getTx() const;

    float getTy() const;

    // compute & return transformation matrix (3 x 3)
    glm::mat3 computeTransform(glm::mat3 transform) override;

    // virtual destructor
    virtual ~TranslateNode();
};

class RotateNode : public Node
{

private:

    // magnitude of rotation in degree
    float deg;

public:

    // constructors
    RotateNode();

    RotateNode(glm::vec3 color, QString name, Block *block, float degree);

    RotateNode(const RotateNode &node);

    // assign
    RotateNode& operator=(const RotateNode &node);

    /**
     * Setters & Getters
     **/
    void setDeg(float degree);

    float getDeg() const;

    // compute & return transformation matrix (3 x 3)
    glm::mat3 computeTransform(glm::mat3 transform) override;

    // virtual destructor
    virtual ~RotateNode();
};

class ScaleNode : public Node
{

private:

    // scale in the X direction
    float sx;

    // scale in the Y direction
    float sy;

public:

    // constructors
    ScaleNode();

    ScaleNode(glm::vec3 color,
              QString name,
              Block *block,
              float scaleX,
              float scaleY);

    ScaleNode(const ScaleNode &node);

    // assign
    ScaleNode& operator=(const ScaleNode &node);

    /**
     * Setters & Getters
     **/
    void setSx(float scaleX);

    void setSy(float scaleY);

    float getSx() const;

    float getSy() const;

    // compute & return transformation matrix (3 x 3)
    glm::mat3 computeTransform(glm::mat3 transform) override;

    // virtual destructor
    virtual ~ScaleNode();

};
