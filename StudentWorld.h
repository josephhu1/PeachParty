/*
Author: Joseph Hu
*/

#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Board.h"
#include "Actor.h"
#include <vector>
#include <string>
using namespace std;

class GameObject;
class Player;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    bool isEmpty(int x, int y);
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    Actor* get_square_at_location(double x, double y);
    int get_bank_coins() const { return m_bank_coins; }
    void deposit_bank_coins(int coins) { m_bank_coins += coins; }
    void reset_bank_coins() { m_bank_coins = 0; }
    Actor* get_random_square(double x, double y);
    void exchangeSquare(double x, double y);
    Peach* getPeach() const;
    Yoshi* getYoshi() const;
    bool isOverlap(int coordX1, int coordY1, int coordX2, int coordY2);
private:
    Board bd;
    Peach* m_peach;
    Yoshi* m_yoshi;
    vector<Actor*> m_actors;
    int m_bank_coins;
};

#endif // STUDENTWORLD_H_

