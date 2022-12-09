#include "lsystems.h"
#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>
#include <iostream>

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// LSYSTEM START

LSystem::LSystem(glm::vec2 pos, glm::vec2 heading, float fDistance) :
    path(""), activeTurtle(Turtle(pos, heading, fDistance)), turtleStack(), ruleSet(), charToDrawingOperation(), branchProb(1.0f) {}

LSystem::~LSystem(){}

void LSystem::addRule(QChar chr, QString str){
    ruleSet[chr] = str;
}

void LSystem::generatePath(int n, QString seed){
    // Add Generative Rules
    //addRule('F', "F[-F]F[+F][F]");
    addRule('F', "F[+F+F][-F-F+F]F[-F-F]F[+F+F]");

    addRule('X', "[-FX]+FX");

    // String Generation
    QString s = seed;
    for(int i = 0; i < n; i++){
        QHashIterator<QChar, QString> j(ruleSet);
        while (j.hasNext()) {
            j.next();
            s.replace(QRegularExpression(QString(j.key())), j.value());
        }
    }
    path = s;
}

void LSystem::populateOps(){
    Rule leftBracketPtr     = &LSystem::leftBracket;
    Rule rightBracketPtr    = &LSystem::rightBracket;
    Rule FPtr               = &LSystem::F;
    Rule minusSignPtr       = &LSystem::minusSign;
    Rule plusSignPtr        = &LSystem::plusSign;
    Rule XPtr               = &LSystem::X;

    charToDrawingOperation['['] = leftBracketPtr;
    charToDrawingOperation[']'] = rightBracketPtr;
    charToDrawingOperation['F'] = FPtr;
    charToDrawingOperation['-'] = minusSignPtr;
    charToDrawingOperation['+'] = plusSignPtr;
    charToDrawingOperation['X'] = XPtr;
}

void LSystem::leftBracket(){
    activeTurtle.increaseDepth();
    Turtle turtleCopy = Turtle(activeTurtle);
    turtleStack.push(turtleCopy);
}

void LSystem::rightBracket(){
    activeTurtle = turtleStack.pop();
}

void LSystem::F(){
    activeTurtle.moveForward();
}

void LSystem::minusSign(){
    activeTurtle.turnLeft();
}

void LSystem::plusSign(){
    activeTurtle.turnRight();
}

void LSystem::X(){}

void LSystem::printPath(){
    std::cout << path.toUtf8().constData() << std::endl;
}

float LSystem::rand01(){
    float n = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    return n;
}

// LSYSTEM END
// **********************************************************************************************
// TURTLE START

Turtle::Turtle() :
    position(glm::vec2(0,0)), orientation(glm::vec2(0,1)), depth(1), fDistance(1.0f) {}

Turtle::Turtle(glm::vec2 pos, glm::vec2 heading, float fDistance) :
    position(pos), orientation(heading), depth(1), fDistance(fDistance)
{}

Turtle::Turtle(const Turtle& t) :
    position(t.position), orientation(t.orientation), depth(t.depth), fDistance(t.fDistance)
{}

Turtle::~Turtle(){}

void Turtle::turnLeft(){
    float angle = PI_4 - randAngle();
    orientation = glm::vec2(orientation[0] * cosf(angle) - orientation[1] * sinf(angle),
                            orientation[0] * sinf(angle) + orientation[1] * cosf(angle));
}

void Turtle::turnRight(){
    float angle = -PI_4 + randAngle();
    orientation = glm::vec2(orientation[0] * cosf(angle) - orientation[1] * sinf(angle),
                            orientation[0] * sinf(angle) + orientation[1] * cosf(angle));
}

void Turtle::moveForward(){
    glm::vec2 change = orientation * fDistance;
    position += change;
}

void Turtle::increaseDepth(){
    depth++;
}

void Turtle::decreaseDepth(){
    depth--;
}

void Turtle::printCoordinates() const{
    std::cout << "(" << position[0] << ", " << position[1] << ")" << std::endl;
}

void Turtle::printOrientation() const{
    std::cout << "(" << orientation[0] << ", " << orientation[1] << ")" << std::endl;
}

float Turtle::randAngle() const{
    float n = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    return n * PI_4;
}

// TURTLE END
// **********************************************************************************************
// TREE START

Tree::Tree(glm::vec2 pos, float fDistance) :
    LSystem(pos, glm::vec2(0.0f, 1.0f), fDistance){
    this->branchProb = 0.7f;
}

Tree::~Tree(){}
