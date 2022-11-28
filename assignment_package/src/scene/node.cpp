#include <QTreeWidgetItem>
#include "smartpointerhelp.h"
#include "node.h"


#define PI 3.14159265

///----------------------------------------------------------------------
/// Node
///----------------------------------------------------------------------

/**
 * @brief Node::Node
 * @param color
 * @param name
 * @param block
 */
Node::Node(glm::vec3 color, QString name, Block *block)
    : color(color), name(name), block(block)
{
    // init
    children = std::vector<uPtr<Node>>();

    // set name, shown in GUI
    setText(0, name);

}

/**
 * @brief Node::Node
 */
Node::Node()
    : Node(glm::vec3(0, 0, 0),
           "dummy",
           nullptr)
{}

/**
 * @brief Node::Node
 * @param node
 */
Node::Node(const Node &node)
    : Node(node.getColor(), node.getName(), node.block)
{

    // TODO: deep copy of children
    for (const uPtr<Node> &child : node.children) {
        TranslateNode *tChild = dynamic_cast<TranslateNode *>(child.get());
        if (tChild != nullptr) {
            // it's a TranslateNode
            addChild(mkU<TranslateNode>(*tChild));
            continue;
        }
        RotateNode *rChild = dynamic_cast<RotateNode *>(child.get());
        if (rChild != nullptr) {
            // it's a RotateNode
            addChild(mkU<RotateNode>(*rChild));
            continue;
        }
        ScaleNode *sChild = dynamic_cast<ScaleNode *>(child.get());
        if (sChild != nullptr) {
            // it's a ScaleNode
            addChild(mkU<ScaleNode>(*sChild));
            continue;
        }
    }

}

/**
 * @brief Node::operator =
 * @param node
 * @return
 */
Node& Node::operator=(const Node &node)
{

    color = node.getColor();
    name = node.getName();
    block = node.block;

    // copy children
    for (const uPtr<Node> &child : node.children) {
        TranslateNode *tChild = dynamic_cast<TranslateNode *>(child.get());
        if (tChild != nullptr) {
            // it's a TranslateNode
            addChild(mkU<TranslateNode>(*tChild));
            continue;
        }
        RotateNode *rChild = dynamic_cast<RotateNode *>(child.get());
        if (rChild != nullptr) {
            // it's a RotateNode
            addChild(mkU<RotateNode>(*rChild));
            continue;
        }
        ScaleNode *sChild = dynamic_cast<ScaleNode *>(child.get());
        if (sChild != nullptr) {
            // it's a ScaleNode
            addChild(mkU<ScaleNode>(*sChild));
            continue;
        }
    }

    return *this;
}

/**
 * @brief Node::getName
 * @return QString
 */
QString Node::getName() const
{
    return name;
}

/**
 * @brief Node::setColor
 * @param newColor
 */
void Node::setColor(glm::vec3 newColor)
{
    color = newColor;
}

/**
 * @brief Node::getColor
 * @return glm::vec3
 */
glm::vec3 Node::getColor() const
{
    return color;
}

/**
 * @brief Node::addChild
 * @param node
 * @return Node& : the reference of child node
 */
Node& Node::addChild(uPtr<Node> node)
{
    // get the reference of the node on heap
    Node &ref = *node;

    // move the unique ptr to its parent
    this->children.push_back(std::move(node));

    // invoke QTreeWidgetItem
    QTreeWidgetItem::addChild(&ref);

    // return the reference of the node
    return ref;

}

/**
 * @brief Node::getChildren
 * @return
 */
const std::vector<uPtr<Node>>& Node::getChildren()
{
    return children;
}

Node::~Node()
{
}

///----------------------------------------------------------------------
/// TranslateNode
///----------------------------------------------------------------------

/**
 * @brief TranslateNode::TranslateNode
 * @param color
 * @param name
 * @param block
 * @param translateX
 * @param translateY
 */
TranslateNode::TranslateNode(glm::vec3 color,
                             QString name,
                             Block *block,
                             float translateX,
                             float translateY)
    : Node(color, name, block), tx(translateX), ty(translateY)
{}

/**
 * @brief TranslateNode::TranslateNode
 */
TranslateNode::TranslateNode()
    : Node(glm::vec3(0.0, 0.0, 0.0),
                     "translate",
                     nullptr),
      tx(0.0),
      ty(0.0)
{}

/**
 * @brief TranslateNode::TranslateNode
 * @param node
 */
TranslateNode::TranslateNode(const TranslateNode &node)
    : Node(node), tx(node.getTx()), ty(node.getTy())
{
}

/**
 * @brief TranslateNode::operator =
 * @param node
 * @return Translate&
 */
TranslateNode& TranslateNode::operator=(const TranslateNode &node)
{

    Node::operator=(node);
    tx = node.getTx();
    ty = node.getTy();

    return *this;
}

/**
 * @brief TranslateNode::setTx
 * @param translateX
 */
void TranslateNode::setTx(float translateX)
{
    tx = translateX;
}

/**
 * @brief TranslateNode::setTy
 * @param translateY
 */
void TranslateNode::setTy(float translateY)
{
    ty = translateY;
}

/**
 * @brief TranslateNode::getTx
 * @return
 */
float TranslateNode::getTx() const
{
    return tx;
}

/**
 * @brief TranslateNode::getTy
 * @return
 */
float TranslateNode::getTy() const
{
    return ty;
}

/**
 * @brief TranslateNode::computeTransform
 * @param transform
 * @return mat3
 */
glm::mat3 TranslateNode::computeTransform(glm::mat3 transform)
{
    // apply translation on the input transform
    // transform * translation
    return glm::translate(transform, glm::vec2(tx, ty));

}

TranslateNode::~TranslateNode()
{
}

///----------------------------------------------------------------------
/// RotateNode
///----------------------------------------------------------------------

/**
 * @brief RotateNode::RotateNode
 * @param color
 * @param name
 * @param block
 * @param degree
 */
RotateNode::RotateNode(glm::vec3 color,
                       QString name,
                       Block *block,
                       float degree)
    : Node(color, name, block), deg(degree)
{}

/**
 * @brief RotateNode::RotateNode
 */
RotateNode::RotateNode()
    : Node(glm::vec3(0.0, 0.0, 0.0),
           "rotate",
           nullptr),
      deg(0.0)
{}

/**
 * @brief RotateNode::RotateNode
 * @param node
 */
RotateNode::RotateNode(const RotateNode &node)
    : Node(node), deg(node.getDeg())
{
}

/**
 * @brief RotateNode::operator =
 * @param node
 * @return RotateNode&
 */
RotateNode& RotateNode::operator=(const RotateNode &node)
{
    Node::operator=(node);
    deg = node.getDeg();
    return *this;
}

/**
 * @brief RotateNode::setDeg
 * @param degree
 */
void RotateNode::setDeg(float degree)
{
    deg = degree;
}

/**
 * @brief getDeg
 * @return
 */
float RotateNode::getDeg() const
{
    return deg;
}

/**
 * @brief RotateNode::computeTransform
 * @param transform
 * @return mat3
 */
glm::mat3 RotateNode::computeTransform(glm::mat3 transform)
{
    // convert to radian
    float radian = deg * (PI / 180.f);

    // apply on transform => transform * rotateMatrix
    return glm::rotate(transform, radian);

}

/**
 * @brief RotateNode::~RotateNode
 */
RotateNode::~RotateNode()
{}

///----------------------------------------------------------------------
/// ScaleNode
///----------------------------------------------------------------------

/**
 * @brief ScaleNode::ScaleNode
 * @param color
 * @param name
 * @param block
 * @param scaleX
 * @param scaleY
 */
ScaleNode::ScaleNode(glm::vec3 color,
                     QString name,
                     Block *block,
                     float scaleX,
                     float scaleY)
    : Node(color, name, block), sx(scaleX), sy(scaleY)
{}

/**
 * @brief ScaleNode::ScaleNode
 */
ScaleNode::ScaleNode()
    : Node(glm::vec3(0.0, 0.0, 0.0),
           "scale",
           nullptr),
      sx(0.0),
      sy(0.0)
{}

/**
 * @brief ScaleNode::ScaleNode
 * @param node
 */
ScaleNode::ScaleNode(const ScaleNode &node)
    : Node(node),
      sx(node.getSx()),
      sy(node.getSy())
{}

/**
 * @brief ScaleNode::operator =
 * @param node
 * @return
 */
ScaleNode& ScaleNode::operator=(const ScaleNode &node)
{
    Node::operator=(node);
    sx = node.getSx();
    sy = node.getSy();
    return *this;
}

/**
 * @brief ScaleNode::setSx
 * @param scaleX
 */
void ScaleNode::setSx(float scaleX)
{
    sx = scaleX;
}

/**
 * @brief ScaleNode::setSy
 * @param scaleY
 */
void ScaleNode::setSy(float scaleY)
{
    sy = scaleY;
}

/**
 * @brief getSx
 * @return
 */
float ScaleNode::getSx() const
{
    return sx;
}

/**
 * @brief ScaleNode::getSy
 * @return
 */
float ScaleNode::getSy() const
{
    return sy;
}

/**
 * @brief ScaleNode::computeTransform
 * @param transform
 * @return mat3
 */
glm::mat3 ScaleNode::computeTransform(glm::mat3 transform)
{
    return glm::scale(transform, glm::vec2(sx, sy));
}

/**
 * @brief ScaleNode::~ScaleNode
 */
ScaleNode::~ScaleNode()
{}

