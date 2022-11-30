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
Node::Node(NPCBlock *block)
    : block(block)
{
    // init
    children = std::vector<uPtr<Node>>();
}

/**
 * @brief Node::Node
 */
Node::Node()
    : Node(nullptr)
{}

/**
 * @brief Node::Node
 * @param node
 */
Node::Node(const Node &node)
    : Node(node.block)
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
 */
TranslateNode::TranslateNode(NPCBlock *block,
                             glm::vec3 translate)
    : Node(block), translate(translate)
{}

/**
 * @brief TranslateNode::TranslateNode
 */
TranslateNode::TranslateNode()
    : Node(nullptr),
      translate(glm::vec3(0.))
{}

/**
 * @brief TranslateNode::TranslateNode
 * @param node
 */
TranslateNode::TranslateNode(const TranslateNode &node)
    : Node(node), translate(node.getTranslate())
{}

/**
 * @brief TranslateNode::operator =
 * @param node
 * @return Translate&
 */
TranslateNode& TranslateNode::operator=(const TranslateNode &node)
{

    Node::operator=(node);
    translate = node.translate;

    return *this;
}

/**
 * @brief TranslateNode::setTranslate
 * @param translate
 */
void TranslateNode::setTranslate(glm::vec3 t)
{
    translate = t;
}


/**
 * @brief TranslateNode::getTranslate
 * @return
 */
glm::vec3 TranslateNode::getTranslate() const
{
    return translate;
}

/**
 * @brief TranslateNode::computeTransform
 * @param transform
 * @return mat4
 */
glm::mat4 TranslateNode::computeTransform(glm::mat4 transform)
{
    // apply translation on the input transform
    // transform * translation
    return glm::translate(transform, translate);
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
RotateNode::RotateNode(NPCBlock *block,
                       glm::vec3 rotAxis,
                       float degree)
    : Node(block), rotAxis(rotAxis), deg(degree)
{}

/**
 * @brief RotateNode::RotateNode
 */
RotateNode::RotateNode()
    : Node(nullptr),
      rotAxis(0.f, 0.f, 1.f),
      deg(0.0)
{}

/**
 * @brief RotateNode::RotateNode
 * @param node
 */
RotateNode::RotateNode(const RotateNode &node)
    : Node(node), rotAxis(node.rotAxis), deg(node.getDeg())
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
    rotAxis = node.rotAxis;
    deg = node.deg;
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


void RotateNode::setAxis(glm::vec3 axis)
{
    rotAxis = axis;
}

glm::vec3 RotateNode::getAxis() const
{
    return rotAxis;
}

/**
 * @brief RotateNode::computeTransform
 * @param transform
 * @return mat3
 */
glm::mat4 RotateNode::computeTransform(glm::mat4 transform)
{
    // convert to radian
    float radian = deg * (PI / 180.f);

    // apply on transform => transform * rotateMatrix
    // return glm::rotate(transform, radian);
    return glm::rotate(transform, radian, rotAxis);
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
ScaleNode::ScaleNode(NPCBlock *block,
                     glm::vec3 scale)
    : Node(block), scale(scale)
{}

/**
 * @brief ScaleNode::ScaleNode
 */
ScaleNode::ScaleNode()
    : Node(nullptr),
      scale(glm::vec3(1.))
{}

/**
 * @brief ScaleNode::ScaleNode
 * @param node
 */
ScaleNode::ScaleNode(const ScaleNode &node)
    : Node(node),
      scale(node.scale)
{}

/**
 * @brief ScaleNode::operator =
 * @param node
 * @return
 */
ScaleNode& ScaleNode::operator=(const ScaleNode &node)
{
    Node::operator=(node);
    scale = node.scale;
    return *this;
}


/**
 * @brief ScaleNode::computeTransform
 * @param transform
 * @return mat3
 */
glm::mat4 ScaleNode::computeTransform(glm::mat4 transform)
{
    return glm::scale(transform, scale);
}

/**
 * @brief ScaleNode::~ScaleNode
 */
ScaleNode::~ScaleNode()
{}

