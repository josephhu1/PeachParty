/*
Author: Joseph Hu
*/

#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;
class Enemy;

//Player Number
const int PLAYER_1 = 1;
const int PLAYER_2 = 2;

//Player State
const int WAITING_TO_ROLE = 0;
const int WALKING = 1;

//Coin State
const int NOT_ALIVE = 0;
const int ALIVE = 1;

//WALKING DIRECTION
const int WALK_RIGHT = 'r';
const int WALK_LEFT = 'l';
const int WALK_UP = 'u';
const int WALK_DOWN = 'd';

//BOWSER/BOO STATE
const int PAUSED_STATE = 0;
const int WALKING_STATE = 1;

class Actor: public GraphObject
{
public:
    Actor(StudentWorld* world, int imageID, double startX, double startY, int startDirection, int depth, double size);
    virtual void do_something() = 0;
    virtual bool isAlive() const { return true; }
    StudentWorld* getWorld() { return m_world; }
    virtual bool is_a_square() const = 0;
    void forkPositions(bool& upValid, bool& downValid, bool& rightValid, bool& leftValid, int& counter);
    void setWalkingDirection(int update) { m_walkingDirection = update; }
    int getWalkingDirection() { return m_walkingDirection; }
    int newWalkingDirection();
    void checkSpot();
    void move();
    bool canMove(int x, int y);

private:
    StudentWorld* m_world;
    int m_imageID;
    int m_walkingDirection;

};

class Player: public Actor
{
public:
    Player(StudentWorld* world, int playerNum, int imageID, double startX, double startY);
    void do_something();
    int get_stars() const { return m_stars; }
    int get_coins() const { return m_coins; }
    int get_dice() const { return m_diceStatus; }
    void adjust_stars(const int this_much) { m_stars += this_much; }
    void adjust_coins(const int this_much) { m_coins += this_much; }
    void reset_coins() { m_coins = 0; }
    void reset_stars() { m_stars = 0; }
    bool getLandedOnSquare() { return m_landedOnSquare; }
    void setLandedOnSquare(bool update) { m_landedOnSquare = update; }
    void setState(int update) { m_state = update; }
    int getState() { return m_state; }
    int getTicksToMove() { return m_ticks_to_move; }
    void setTicksToMove(int update) { m_ticks_to_move = update; }
    virtual bool is_a_square() const { return false; }
    void atFork();
private:
    int getPlayerNum() { return m_playerNum; }
    int m_playerNum;
    int m_ticks_to_move;
    int m_state;
    int m_coins;
    int m_stars;
    bool m_landedOnSquare;
    int m_diceStatus;
};

class Peach: public Player
{
public:
    Peach(StudentWorld* world, double startX, double startY);
};

class Yoshi: public Player
{
public:
    Yoshi(StudentWorld* world, double startX, double startY);
};

class ActivatingObject : public Actor {
public:
    ActivatingObject(StudentWorld *sw, int imageID, int startX, int startY, int dir, double size, int depth);
    virtual void do_something() = 0;
};

class ActivateOnPlayer : public ActivatingObject {
public:
    ActivateOnPlayer(StudentWorld *sw, int imageID, int startX, int startY, int dir, double size, int depth, bool activate_when_go_lands);
    virtual bool isAlive() { return m_activate_when_go_lands; }
    virtual void setAlive(bool update) { m_activate_when_go_lands = update; }
    virtual void do_something() = 0;
private:
    bool m_activate_when_go_lands;
};

class StarSquare : public ActivateOnPlayer {
public:
    StarSquare(StudentWorld *sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const { return true; }
    virtual void do_something();
private:
    void giveStar(Player* p);
};

class CoinSquare : public ActivateOnPlayer {
public:
    CoinSquare(StudentWorld *sw, int imageID, int startX, int startY, int adjust_coins_by);
    virtual bool is_a_square() const { return true; }
    virtual void do_something();
    int distributeCoins() { return m_adjust_coins_by; }
private:
    int m_adjust_coins_by;
    void giveCoins(Player* p);
};

class DirectionalSquare : public ActivateOnPlayer {
public:
    DirectionalSquare(StudentWorld *sw, int imageID, int startX, int startY, int dir, int angle);
    virtual bool is_a_square() const { return true; }
    virtual void do_something();
private:
    int m_angle;
    int getAngle() { return m_angle; }
    void changeDirection(Player* p);
};

class BankSquare : public ActivateOnPlayer {
public:
    BankSquare(StudentWorld *sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const { return true; }
    virtual void do_something();
private:
    void takeCoins(Player* p);
    void giveCoins(Player* p);
};

class EventSquare : public ActivateOnPlayer {
public:
  EventSquare(StudentWorld *sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const { return true; }
    virtual void do_something();
private:
    void teleportPlayer(Player* p);
    void swapPlayer(Player* p1, Player* p2);
};

class Enemy : public ActivateOnPlayer {
public:
  Enemy(StudentWorld *sw, int imageID, int startX, int startY, int dir, double size, int depth, bool activate_when_go_lands, int num_sq_to_move, int number_of_ticks_to_pause, int walkState);
    virtual void do_something() = 0;
    virtual bool is_a_square() const { return false; };
    int getWalkState() { return m_walkState; }
    void setWalkState(int update) { m_walkState = update; }
    int getPauseCounter() { return m_number_of_ticks_to_pause; }
    void setPauseCounter(int update) { m_number_of_ticks_to_pause = update; }
    int getSquaresToMove() { return m_squares_to_move; }
    void setSquaresToMove(int update) { m_squares_to_move = update; }
    int getTicksToMove() { return m_ticks_to_move; }
    void setTicksToMove(int update) { m_ticks_to_move = update; }
    int randomDir();
    bool atFork();
    bool affectPeach() const { return m_peach_status; }
    void affectPeachStatus(bool actionOnPeach) { m_peach_status = actionOnPeach; }
    bool affectYoshi() const { return m_yoshi_status; }
    void affectYoshiStatus(bool actionOnYoshi) { m_yoshi_status = actionOnYoshi; }
    bool attackPeach();
    bool attackYoshi();
    void endMove(int steps);
    void startMove();
private:
    int m_num_sq_to_move;
    int m_number_of_ticks_to_pause;
    int m_walkState;
    int m_squares_to_move;
    int m_ticks_to_move;
    bool m_peach_status;
    bool m_yoshi_status;
};

class DroppingSquare : public ActivateOnPlayer {
public:
    DroppingSquare(StudentWorld *sw, int imageID, int startX, int startY);
    virtual void do_something();
    virtual bool is_a_square() const { return true; }
private:
    void executeDroppingSquare(Player* p);
};

class Bowser : public Enemy {
public:
    Bowser(StudentWorld *sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const { return false; }
    virtual void do_something();
private:
    void executeBowser(Player* p);
};

class Boo : public Enemy {
public:
    Boo(StudentWorld *sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const { return false; }
    virtual void do_something();
private:
    void executeBoo(Player* p1, Player* p2);
};

#endif // ACTOR_H_
