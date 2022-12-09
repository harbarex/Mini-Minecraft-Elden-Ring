#pragma once

#include <la.h>
#include <QString>
#include <QChar>
#include <QHash>
#include <QStack>
#include <QRegularExpression>
#include <time.h>

using namespace glm;
using namespace std;

const float PI = 3.141592653589793238463;
const float PI_4 = 0.78539816;
const float PI_6 = 0.52359877;
const float PI_8 = 0.39269908;

class LSystem;
typedef void (LSystem::*Rule)(void);

class Turtle
{
public:
    glm::vec2   position;       // starting position of turtle
    glm::vec2   orientation;    // staring orientation of turtle
    int         depth;          // recursion depth
    float       fDistance;      // distance turtle moves on 'F'

    Turtle();
    Turtle(glm::vec2 pos, glm::vec2 heading, float fDistance);
    Turtle(const Turtle& t);
    ~Turtle();

    void turnLeft();
    void turnRight();
    void moveForward();
    void increaseDepth();
    void decreaseDepth();

    void printCoordinates() const;  // prints turtle's position
    void printOrientation() const;  // prints turtle's orientation vector
private:
    float randAngle() const;        // returns an angle between 0 and PI/4 radians
};

class LSystem
{
public:
    QString path;
    Turtle activeTurtle;
    QStack<Turtle> turtleStack;
    QHash<QChar, QString> ruleSet;              // char -> string map replacement rules for generating turtle path instructions
    QHash<QChar, Rule> charToDrawingOperation;  // maps characters to LSystem functions controlling this turtle
    float branchProb;                           // probability of branch generation

    LSystem(glm::vec2 pos, glm::vec2 heading, float fDistance);
    virtual ~LSystem();

    virtual void generatePath(int n, QString seed); // generates path to be traversed by turtle (n branching events)
    virtual void populateOps();                     // populates charToDrawingOperation hash
    void printPath();                               // prints path string
    float rand01();                                 // returns a random number between 0 and 1
protected:
    void addRule(QChar chr, QString str);           // add a replacement rule for path string generation

    virtual void leftBracket();
    virtual void rightBracket();
    virtual void F();
    virtual void minusSign();
    virtual void plusSign();
    virtual void X();
};


class Tree : public LSystem{                        // An LSystem Tree
public:
    Tree(glm::vec2 pos, float fDistance);
    virtual ~Tree();
};

