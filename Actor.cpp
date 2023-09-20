/*
Author: Joseph Hu
*/

#include "Actor.h"
#include "StudentWorld.h"

// ======================= Objects
Actor::Actor(StudentWorld* world, int imageID, double startX, double startY, int startDirection, int depth, double size)
: GraphObject(imageID, startX, startY, startDirection, depth, size), m_world(world), m_walkingDirection(WALK_RIGHT){}

Player::Player(StudentWorld* world, int playerNum, int imageID, double startX, double startY)
: Actor(world, imageID, startX, startY, 0, 0, 1.0), m_playerNum(playerNum), m_ticks_to_move(0), m_state(WAITING_TO_ROLE), m_coins(0), m_stars(0), m_landedOnSquare(true), m_diceStatus(0){}

Peach::Peach(StudentWorld* world, double startX, double startY)
: Player(world, PLAYER_1, IID_PEACH, startX, startY){}

Yoshi::Yoshi(StudentWorld* world, double startX, double startY)
: Player(world, PLAYER_2, IID_YOSHI, startX, startY){}

ActivatingObject::ActivatingObject(StudentWorld *sw, int imageID, int startX, int startY, int dir, double size, int depth)
  : Actor(sw, imageID, startX, startY, dir, depth, size){}

ActivateOnPlayer::ActivateOnPlayer(StudentWorld *sw, int imageID, int startX, int startY, int dir, double size, int depth, bool activate_when_go_lands)
: ActivatingObject(sw, imageID, startX, startY, dir, size, depth), m_activate_when_go_lands(activate_when_go_lands){}

CoinSquare::CoinSquare(StudentWorld *sw, int imageID, int startX, int startY, int adjust_coins_by)
  : ActivateOnPlayer(sw, imageID, startX, startY, 0, 1.0, 1, ALIVE), m_adjust_coins_by(adjust_coins_by){}

Enemy::Enemy(StudentWorld *sw, int imageID, int startX, int startY,
      int dir, double size, int depth, bool activate_when_go_lands, int num_sq_to_move, int number_of_ticks_to_pause, int walkState)
: ActivateOnPlayer(sw, imageID, startX, startY, dir, size, depth, activate_when_go_lands), m_num_sq_to_move(num_sq_to_move), m_number_of_ticks_to_pause(number_of_ticks_to_pause), m_walkState(walkState), m_squares_to_move(0), m_ticks_to_move(0), m_peach_status(true), m_yoshi_status(true){}

Bowser::Bowser(StudentWorld *sw, int imageID, int startX, int startY)
: Enemy(sw, imageID, startX, startY, 0, 1, 0, ALIVE, 0, 180, PAUSED_STATE){}

StarSquare::StarSquare(StudentWorld *sw, int imageID, int startX, int startY)
: ActivateOnPlayer(sw, imageID, startX, startY, 0, 1.0, 1, ALIVE){}

BankSquare::BankSquare(StudentWorld *sw, int imageID, int startX, int startY)
: ActivateOnPlayer(sw, imageID, startX, startY, 0, 1.0, 1, ALIVE){}

EventSquare::EventSquare(StudentWorld *sw, int imageID, int startX, int startY)
: ActivateOnPlayer(sw, imageID, startX, startY, 0, 1.0, 1, ALIVE){}

DroppingSquare::DroppingSquare(StudentWorld *sw, int imageID, int startX, int startY)
: ActivateOnPlayer(sw, imageID, startX, startY, 0, 1.0, 1, ALIVE){}

DirectionalSquare::DirectionalSquare(StudentWorld *sw, int imageID, int startX, int startY, int dir, int angle)
: ActivateOnPlayer(sw, imageID, startX, startY, dir, 1.0, 1, ALIVE), m_angle(dir){}

Boo::Boo(StudentWorld *sw, int imageID, int startX, int startY)
: Enemy(sw, imageID, startX, startY, 0, 1, 0, ALIVE, 0, 180, PAUSED_STATE){}

/*
    do_something(): 
        Each object in the game world will decide to perform an action or behavior per tick
        This will ensure smooth animation (Takes 8 ticks to move from 1 square to another)
*/

// ======================= 2 Players in game world: Peach and Yoshi
void Player::do_something()
{
    checkSpot();
    if (getState() == WAITING_TO_ROLE)
    {
        if (getWorld()->getAction(getPlayerNum()) == ACTION_ROLL)
        {
            m_diceStatus = randInt(1, 10);
            setTicksToMove(get_dice() * 8);
            setState(WALKING);
        }
    }
    else if (getState() == WALKING)
    {
        atFork();
        move();
        setTicksToMove(getTicksToMove()-1);
        if (getTicksToMove() == 0)
        {
            setLandedOnSquare(true);
            setState(WAITING_TO_ROLE);
        }
    }
}

// ======================= Squares Peach and Yoshi will interact with
void CoinSquare::do_something()
{
    if (!isAlive())
        return;
    giveCoins(getWorld()->getPeach());
    giveCoins(getWorld()->getYoshi());
}

void StarSquare::do_something()
{
    giveStar(getWorld()->getPeach());
    giveStar(getWorld()->getYoshi());
}

void BankSquare::do_something()
{
    giveCoins(getWorld()->getPeach());
    giveCoins(getWorld()->getYoshi());
    takeCoins(getWorld()->getPeach());
    takeCoins(getWorld()->getYoshi());
}

void EventSquare::do_something()
{
    int possibility = randInt(0, 1);
    switch (possibility)
    {
        case 0:
            teleportPlayer(getWorld()->getPeach());
            teleportPlayer(getWorld()->getYoshi());
            break;
        case 1:
            swapPlayer(getWorld()->getPeach(), getWorld()->getYoshi());
            swapPlayer(getWorld()->getYoshi(), getWorld()->getPeach());
            break;
    }
}

void DroppingSquare::do_something()
{
    executeDroppingSquare(getWorld()->getPeach());
    executeDroppingSquare(getWorld()->getYoshi());
}

void DirectionalSquare::do_something()
{
    changeDirection(getWorld()->getPeach());
    changeDirection(getWorld()->getYoshi());
}

// ======================= 2 Enemies: Bowser and Boo

void Bowser::do_something()
{
    if (getWalkState() == PAUSED_STATE)
    {
        if (attackPeach())
            executeBowser(getWorld()->getPeach());
        if (attackYoshi())
            executeBowser(getWorld()->getYoshi());
        setPauseCounter(getPauseCounter() - 1);
        endMove(randInt(1, 10));
    }
    if (getWalkState() == WALKING_STATE)
    {
        startMove();
        if (getTicksToMove() == 0)
        {
            int possibility = randInt(0, 3);
            switch(possibility)
            {
                case 0:
                    getWorld()->exchangeSquare(getX(), getY());
                    getWorld()->playSound(SOUND_DROPPING_SQUARE_CREATED);
                    break;
            }
        }
    }
}

void Boo::do_something()
{
    if (getWalkState() == PAUSED_STATE)
    {
        if (attackPeach())
            executeBoo(getWorld()->getPeach(), getWorld()->getYoshi()); 
        if (attackYoshi())
            executeBoo(getWorld()->getYoshi(), getWorld()->getPeach());
        setPauseCounter(getPauseCounter() - 1);
        endMove(randInt(1, 3));
    }
    if (getWalkState() == WALKING_STATE)
        startMove();
}

// ======================= AUXILIARY FUNCTIONS

/*
    Determine if Actor can move onto next spot
    Force the sprite direction if needed
 */
void Actor::checkSpot()
{
    int newX = getX();
    int newY = getY();
    switch (getWalkingDirection())
    {
    case WALK_RIGHT:
        newX += 16;
        break;
    case WALK_LEFT:
        newX -= 16;
        break;
    case WALK_UP:
        newY += 16;
        break;
    case WALK_DOWN:
        newY -= 16;
        break;
    }
    
    bool validPos = newX % SPRITE_WIDTH == 0 && newY % SPRITE_HEIGHT == 0;
    if (validPos && getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
        setWalkingDirection(newWalkingDirection());
    getWalkingDirection() == WALK_LEFT ? setDirection(180) : setDirection(0);
}

/*
    Determine where Actor should move to
*/
void Actor::move()
{
    switch(getWalkingDirection())
    {
        case WALK_RIGHT:
            moveTo(getX()+2, getY());
            break;
        case WALK_LEFT:
            moveTo(getX()-2, getY());
            break;
        case WALK_UP:
            moveTo(getX(), getY()+2);
            break;
        case WALK_DOWN:
            moveTo(getX(), getY()-2);
            break;
    }
}

/*
    Find new walking direction to face towards
*/
int Actor::newWalkingDirection()
{
    int newX = getX();
    int newY = getY();
    switch(getWalkingDirection())
    {
        case WALK_RIGHT:
        case WALK_LEFT:
            newY = getY() + 16;
            if (!getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
                return WALK_UP;
            newY = getY() - 16;
            if (!getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
                return WALK_DOWN;
        case WALK_UP:
        case WALK_DOWN:
            newX = getX() + 16;
            if (!getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
                return WALK_RIGHT;
            newX = getX() - 16;
            if (!getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
                return WALK_LEFT;
        default:
            exit(1);
    }
}

/*
    Determines if actor can move onto some square
*/
bool Actor::canMove(int x, int y)
{
    bool validPos = x % SPRITE_WIDTH == 0 && y % SPRITE_HEIGHT == 0;
    if (validPos && !getWorld()->isEmpty(x/SPRITE_WIDTH, y/SPRITE_HEIGHT))
        return true;
    else
        return false;
}

/*
    Find directions an actor can move towards in a fork
*/
void Actor::forkPositions(bool& upValid, bool& downValid, bool& rightValid, bool& leftValid, int& counter)
{
    int xnew1, ynew1, xnew2, ynew2, xnew3, ynew3, xnew4, ynew4;  

    getPositionInThisDirection(up, 16, xnew1, ynew1);
    getPositionInThisDirection(down, 16, xnew2, ynew2);
    getPositionInThisDirection(right, 16, xnew3, ynew3);
    getPositionInThisDirection(left, 16, xnew4, ynew4);
    if (canMove(xnew1, ynew1))
    {
        upValid = true;
        counter++;
    }
    if (canMove(xnew2, ynew2))
    {
        downValid = true;
        counter++;
    }
    if (canMove(xnew3, ynew3))
    {
        rightValid = true;
        counter++;
    }
    if (canMove(xnew4, ynew4))
    {
        leftValid = true;
        counter++;
    }
}

/*
    Can use keyboard to choose an action for the Players at a fork
*/
void Player::atFork()
{
    int counter = 0;
    bool upValid = false, downValid = false, rightValid = false, leftValid = false;
    forkPositions(upValid, downValid, rightValid, leftValid, counter);
    
    if (counter <= 2)
        return;
    
    if (getWalkingDirection() == WALK_RIGHT)
        leftValid = false;
    if (getWalkingDirection() == WALK_LEFT)
        rightValid = false;
    if (getWalkingDirection() == WALK_UP)
        downValid = false;
    if (getWalkingDirection() == WALK_DOWN)
        upValid = false;
    
    if (upValid && getWorld()->getAction(getPlayerNum()) == ACTION_UP)
    {
        setWalkingDirection(WALK_UP);
        setDirection(0);
    }
    else if (downValid && getWorld()->getAction(getPlayerNum()) == ACTION_DOWN)
    {
        setWalkingDirection(WALK_DOWN);
        setDirection(0);
    }
    else if (rightValid && getWorld()->getAction(getPlayerNum()) == ACTION_RIGHT)
    {
        setWalkingDirection(WALK_RIGHT);
        setDirection(0);
    }
    else if (leftValid && getWorld()->getAction(getPlayerNum()) == ACTION_LEFT)
    {
        setWalkingDirection(WALK_LEFT);
        setDirection(180);
    }
    else if (getWorld()->getAction(getPlayerNum()) == ACTION_NONE)
        return;
    return;
}

/*
    Give coins to player
*/
void CoinSquare::giveCoins(Player* p)
{
    if (!p->getLandedOnSquare())
        return;
    bool playerOverlap = getWorld()->isOverlap(p->getX(), p->getY(), getX(), getY());
    if (playerOverlap)
    {
        if (distributeCoins() == 3)
        {
            p->adjust_coins(distributeCoins());
            getWorld()->playSound(SOUND_GIVE_COIN);
            p->setLandedOnSquare(false);
        }
        if (distributeCoins() == -3)
        {
            p->get_coins() < 3 ? p->adjust_coins(-(p->get_coins())) : p->adjust_coins(distributeCoins());
            getWorld()->playSound(SOUND_TAKE_COIN);
            p->setLandedOnSquare(false);
        }
    }
}

/*
    Give star to player
*/
void StarSquare::giveStar(Player* p)
{
    if (!p->getLandedOnSquare())
        return;
    bool playerOverlap = getWorld()->isOverlap(p->getX(), p->getY(), getX(), getY());
    if (playerOverlap)
    {
        if (p->get_coins() < 20)
        {
            p->setLandedOnSquare(false);
            return;
        }
        else
        {
            p->adjust_coins(-20);
            p->adjust_stars(1);
            getWorld()->playSound(SOUND_GIVE_STAR);
            p->setLandedOnSquare(false);
        }
    }
}

/*
    Force player to change direction if lands on square
*/
void DirectionalSquare::changeDirection(Player* p)
{
    if (!p->getLandedOnSquare())
        return;
    bool playerOverlap = getWorld()->isOverlap(p->getX(), p->getY(), getX(), getY());
    if (playerOverlap)
    {
        switch(getAngle())
        {
            case 180:
                p->setWalkingDirection(WALK_LEFT);
                p->setDirection(180);
                break;
            case 0:
                p->setWalkingDirection(WALK_RIGHT);
                p->setDirection(0);
                break;
            case 90:
                p->setWalkingDirection(WALK_UP);
                p->setDirection(0);
                break;
            case 270:
                p->setWalkingDirection(WALK_DOWN);
                p->setDirection(0);
                break;
        }
        p->setLandedOnSquare(false);
    }
}

/*
    Take coins from player
*/
void BankSquare::takeCoins(Player* p)
{
    if (p->getTicksToMove() != 8)
        return;
    bool playerOverlap = getWorld()->isOverlap(p->getX(), p->getY(), getX(), getY());
    if (playerOverlap && !p->getLandedOnSquare())
    {
        int numCoins = p->get_coins();
        if (numCoins < 5)
        {
            p->adjust_coins(-numCoins);
            getWorld()->deposit_bank_coins(numCoins);
        }
        else
        {
            p->adjust_coins(-5);
            getWorld()->deposit_bank_coins(5);
        }
        getWorld()->playSound(SOUND_DEPOSIT_BANK);
    }
}

/*
    Give coins to player
*/
void BankSquare::giveCoins(Player* p)
{
    if (!p->getLandedOnSquare())
        return;
    bool playerOverlap = getWorld()->isOverlap(p->getX(), p->getY(), getX(), getY());
    if (playerOverlap)
    {
        p->adjust_coins(getWorld()->get_bank_coins());
        getWorld()->reset_bank_coins();
        getWorld()->playSound(SOUND_WITHDRAW_BANK);
        p->setLandedOnSquare(false);
    }
}

/*
    Teleport player to some random square
*/
void EventSquare::teleportPlayer(Player* p)
{
    if (!p->getLandedOnSquare())
        return;
    bool playerOverlap = getWorld()->isOverlap(p->getX(), p->getY(), getX(), getY());
    if (playerOverlap)
    {
        Actor* square = getWorld()->get_random_square(p->getX(), p->getY());
        p->moveTo(square->getX(), square->getY());
        getWorld()->playSound(SOUND_PLAYER_TELEPORT);
        p->setLandedOnSquare(false);
    }
}

/*
    Swap the 2 players on the game board
*/
void EventSquare::swapPlayer(Player* p1, Player* p2)
{
    if (!p1->getLandedOnSquare())
        return;
    bool playerOverlap = getWorld()->isOverlap(p1->getX(), p1->getY(), getX(), getY());
    if (playerOverlap)
    {
        int tempX = p1->getX();
        int tempY = p1->getY();
        int tempTicks = p1->getTicksToMove();
        int tempWalkingDir = p1->getWalkingDirection();
        int tempSpriteDir = p1->getDirection();
        int tempState = p1->getState();
        
        p1->moveTo(p2->getX(), p2->getY());
        p1->setTicksToMove(p2->getTicksToMove());
        p1->setWalkingDirection(p2->getWalkingDirection());
        p1->setDirection(p2->getDirection());
        p1->setState(p2->getState());
        
        p2->moveTo(tempX, tempY);
        p2->setTicksToMove(tempTicks);
        p2->setWalkingDirection(tempWalkingDir);
        p2->setDirection(tempSpriteDir);
        p2->setState(tempState);
        
        getWorld()->playSound(SOUND_PLAYER_TELEPORT);
        p1->setLandedOnSquare(false);
    }
}

/*
    Square that bowser drops to either take coins or stars
*/
void DroppingSquare::executeDroppingSquare(Player* p)
{
    if (!p->getLandedOnSquare())
        return;
    bool playerOverlap = getWorld()->isOverlap(p->getX(), p->getY(), getX(), getY());
    if (playerOverlap)
    {
        int possibility = randInt(0, 1);
        switch(possibility)
        {
            case 0:
            {
                p->get_coins() < 10 ? p->adjust_coins(-(p->get_coins())) : p->adjust_coins(-10);
                break;
            }
            case 1:
            {
                if (p->get_stars() >= 1)
                    p->adjust_stars(-1);
                break;
            }
        }
        getWorld()->playSound(SOUND_DROPPING_SQUARE_ACTIVATE);
        p->setLandedOnSquare(false);
    }
}

/*
    Allows enemy to move
*/
void Enemy::startMove()
{
    if (canMove(getX(), getY()) && atFork())
        getWalkingDirection() == WALK_LEFT ? setDirection(180) : setDirection(0);
    checkSpot();
    move();
    setTicksToMove(getTicksToMove() - 1);
    if (getTicksToMove() == 0)
    {
        setWalkState(PAUSED_STATE);
        setPauseCounter(180);
    }
}

/*
    Allows enemy to move
*/
void Enemy::endMove(int steps)
{
    if (getPauseCounter() == 0)
    {
        setSquaresToMove(steps);
        setTicksToMove(getSquaresToMove() * 8);
        setWalkingDirection(randomDir());
        getWalkingDirection() == WALK_LEFT ? setDirection(180) : setDirection(0);
        setWalkState(WALKING_STATE);
    }
}

/*
    Pick random direction for enemy to face
*/
int Enemy::randomDir()
{
    for(;;)
    {
        int newX = getX();
        int newY = getY();
        int possibility = randInt(0, 3);
        switch(possibility)
        {
            case 0:
                newY = getY() + 16;
                if (!getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
                    return WALK_UP;
            case 1:
                newY = getY() - 16;
                if (!getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
                    return WALK_DOWN;
            case 2:
                newX = getX() + 16;
                if (!getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
                    return WALK_RIGHT;
            case 3:
                newX = getX() - 16;
                if (!getWorld()->isEmpty(newX/SPRITE_WIDTH, newY/SPRITE_HEIGHT))
                    return WALK_LEFT;
        }
    }
}

/*
    Randomly allow enemy to move to some random square in any direction at a fork
*/
bool Enemy::atFork()
{
    int counter = 0;
    bool upValid = false, downValid = false, rightValid = false, leftValid = false;

    forkPositions(upValid, downValid, rightValid, leftValid, counter);
    if (counter == 2)
        return false;

    for (;;)
    {
        int possibility = randInt(0, 3);
        switch (possibility)
        {
        case 0:
            if (upValid)
                setWalkingDirection(WALK_UP);
            return true;
        case 1:
            if (downValid)
                setWalkingDirection(WALK_DOWN);
            return true;
        case 2:
            if (rightValid)
                setWalkingDirection(WALK_RIGHT);
            return true;
        case 3:
            if (leftValid)
                setWalkingDirection(WALK_LEFT);
            return true;
        }
    }
}

/*
    Action that bowser does
*/
void Bowser::executeBowser(Player* p)
{
    if (p->getState() != WAITING_TO_ROLE)
        return;
    bool playerOverlap = getWorld()->isOverlap(p->getX(), p->getY(), getX(), getY());
    if (playerOverlap)
    {
        if (p->getState() == WAITING_TO_ROLE)
        {
            int possibility = randInt(0, 1);
            switch(possibility)
            {
                case 0:
                    p->reset_coins();
                    p->reset_stars();
                    getWorld()->playSound(SOUND_BOWSER_ACTIVATE);
                    break;
            }
        }
    }
}

/*
    Action that boo does
*/
void Boo::executeBoo(Player* p1, Player* p2)
{
    if (p1->getState() != WAITING_TO_ROLE)
        return;
    bool playerOverlap = getWorld()->isOverlap(p1->getX(), p1->getY(), getX(), getY());
    if (playerOverlap)
    {
        int possibility = randInt(0, 1);
        if (possibility == 0)
        {
            int tempCoins = p1->get_coins();
            p1->reset_coins();
            p1->adjust_coins(p2->get_coins());
            p2->reset_coins();
            p2->adjust_coins(tempCoins);
        }
        else
        {
            int tempStars = p1->get_stars();
            p1->reset_stars();
            p1->adjust_stars(p2->get_stars());
            p2->reset_stars();
            p2->adjust_stars(tempStars);
        }
        getWorld()->playSound(SOUND_BOO_ACTIVATE);
    }
}

/*
    Interaction with Peach by an enemy
*/
bool Enemy::attackPeach()
{
    if (getWorld()->getPeach()->getState() == WAITING_TO_ROLE)
        if (getWorld()->isOverlap(getWorld()->getPeach()->getX(), getWorld()->getPeach()->getY(), getX(), getY()))
            if (affectPeach())
            {
                affectPeachStatus(false);
                return true;
            }
    affectPeachStatus(true);
    return false;
}

/*
    Interaction with Yoshi by an enemy
*/
bool Enemy::attackYoshi()
{
    if (getWorld()->getYoshi()->getState() == WAITING_TO_ROLE)
        if (getWorld()->isOverlap(getWorld()->getYoshi()->getX(), getWorld()->getYoshi()->getY(), getX(), getY()))
            if (affectYoshi())
            {
                affectYoshiStatus(false);
                return true;
            }
    affectYoshiStatus(true);
    return false;
}
