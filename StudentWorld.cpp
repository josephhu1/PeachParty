/*
Author: Joseph Hu
*/

#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <cmath>

using namespace std;

/*
    Creates game world
*/
GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath){}

/*
    Deallocates dynamically-allocated objects
*/
StudentWorld::~StudentWorld()
{
    cleanUp();
}

/*
    Pick a game board and start countdown timer
    Loads up game board with all objects
*/
int StudentWorld::init()
{
    char boardNum = getBoardNumber() + '0';
    string board_file = assetPath() + "board0" + boardNum + ".txt";
    Board::LoadResult result = bd.loadBoard(board_file);
    if (result == Board::load_fail_file_not_found)
        cerr << "Could not find board0" << boardNum << ".txt data file\n";
    else if (result == Board::load_fail_bad_format)
        cerr << "Your board was improperly formatted\n";
    else if (result == Board::load_success)
    {
        cerr << "Successfully loaded board\n";
        for (int col = 0; col < BOARD_WIDTH; col++)
            for (int row = 0; row < BOARD_HEIGHT; row++)
            {
                Board::GridEntry ge = bd.getContentsOf(col, row);
                switch (ge)
                {
                    case Board::player:
                        m_peach = new Peach(this, SPRITE_WIDTH*col, SPRITE_HEIGHT*row);
                        m_yoshi = new Yoshi(this, SPRITE_WIDTH*col, SPRITE_HEIGHT*row);
                        m_actors.push_back(new CoinSquare(this, IID_BLUE_COIN_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, 3));
                        break;
                    case Board::blue_coin_square:
                        m_actors.push_back(new CoinSquare(this, IID_BLUE_COIN_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, 3));
                        break;
                    case Board::empty:
                        break;
                    case Board::boo:
                        m_actors.push_back(new Boo(this, IID_BOO, SPRITE_WIDTH*col, SPRITE_HEIGHT*row));
                        m_actors.push_back(new CoinSquare(this, IID_BLUE_COIN_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, 3));
                        break;
                    case Board::bowser:
                        m_actors.push_back(new Bowser(this, IID_BOWSER, SPRITE_WIDTH*col, SPRITE_HEIGHT*row));
                        m_actors.push_back(new CoinSquare(this, IID_BLUE_COIN_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, 3));
                        break;
                    case Board::red_coin_square:
                        m_actors.push_back(new CoinSquare(this, IID_RED_COIN_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, -3));
                        break;
                    case Board::up_dir_square:
                        m_actors.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, 90, 90));
                        break;
                    case Board::down_dir_square:
                        m_actors.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, 270, 270));
                        break;
                    case Board::left_dir_square:
                        m_actors.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, 180, 180));
                        break;
                    case Board::right_dir_square:
                        m_actors.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row, 0, 0));
                        break;
                    case Board::event_square:
                        m_actors.push_back(new EventSquare(this, IID_EVENT_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row));
                        break;
                    case Board::bank_square:
                        m_actors.push_back(new BankSquare(this, IID_BANK_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row));
                        break;
                    case Board::star_square:
                        m_actors.push_back(new StarSquare(this, IID_STAR_SQUARE, SPRITE_WIDTH*col, SPRITE_HEIGHT*row));
                        break;
                }
            }
    }
    reset_bank_coins();
    startCountdownTimer(99);
    return GWSTATUS_CONTINUE_GAME;
}

/*
    Force all objects to perform a behavior or remove if not needed anymore
    Initializes game board text
    Once timer runs out, determine the winner out of Peach and Yoshi
*/
int StudentWorld::move()
{
    m_peach->do_something();
    m_yoshi->do_something();
    for (int i = 0; i < m_actors.size(); i++)
    {
        if (m_actors[i]->isAlive())
            m_actors[i]->do_something();
        else
        {
            //not alive:
            delete m_actors[i];
            m_actors.erase(m_actors.begin() + i);
        }
    }
    setGameStatText("P1 Roll: " + to_string(m_peach->get_dice()) + " Stars: " + to_string(m_peach->get_stars()) + " $$: " + to_string(m_peach->get_coins()) + " | Time: " + to_string(timeRemaining()) + " | Bank: " + to_string(get_bank_coins()) + " | P2 Roll: " + to_string(m_yoshi->get_dice()) + " Stars: " + to_string(m_yoshi->get_stars()) + " $$: " + to_string(m_yoshi->get_coins()));
    if (timeRemaining() <= 0)
    {
        playSound(SOUND_GAME_FINISHED);
        if (m_yoshi->get_stars() > m_peach->get_stars())
        {
            setFinalScore(m_yoshi->get_stars(), m_yoshi->get_coins());
            return GWSTATUS_YOSHI_WON;
        }
        else if (m_peach->get_stars() > m_yoshi->get_stars())
        {
            setFinalScore(m_peach->get_stars(), m_peach->get_coins());
            return GWSTATUS_PEACH_WON;
        }
        else if (m_yoshi->get_coins() > m_peach->get_coins())
        {
            setFinalScore(m_yoshi->get_stars(), m_yoshi->get_coins());
            return GWSTATUS_YOSHI_WON;
        }
        else if (m_peach->get_coins() > m_yoshi->get_coins())
        {
            setFinalScore(m_peach->get_stars(), m_peach->get_coins());
            return GWSTATUS_PEACH_WON;
        }
        else
        {
            setFinalScore(m_peach->get_stars(), m_peach->get_coins());
            return GWSTATUS_PEACH_WON;
        }
    }
    return GWSTATUS_CONTINUE_GAME;
}

/*
    Deallocate dynamically-allocated objects
*/
void StudentWorld::cleanUp()
{
    while (!m_actors.empty())
    {
        delete m_actors.back();
        m_actors.pop_back();
    }
    delete m_peach;
    delete m_yoshi;
    m_peach = nullptr;
    m_yoshi = nullptr;
}

/*
    Check if object exists at some position in the game board
*/
bool StudentWorld::isEmpty(int x, int y)
{
    if (bd.getContentsOf(x, y) == Board::empty)
        return true;
    return false;
}

/*
    Auxiliary functions: 
*/
bool StudentWorld::isOverlap(int coordX1, int coordY1, int coordX2, int coordY2)
{
    bool validPos = coordX1 % SPRITE_WIDTH == 0 && coordY1 % SPRITE_HEIGHT == 0;
    bool matchingCoordinates = coordX1/16 == coordX2/16 && coordY1/16 == coordY2/16;
    return validPos && matchingCoordinates;
}

Peach* StudentWorld::getPeach() const
{
    return m_peach;
}

Yoshi* StudentWorld::getYoshi() const
{
    return m_yoshi;
}

Actor* StudentWorld::get_random_square(double x, double y)
{
    for (;;)
    {
        int i = randInt(0, static_cast<int>(m_actors.size()) - 1);
        if (m_actors[i]->is_a_square() && m_actors[i]->getX() != x && m_actors[i]->getY() != y)
            return m_actors[i];
    }
    return nullptr;
}

Actor* StudentWorld::get_square_at_location(double x, double y)
{
    for (int i = 0; i < m_actors.size(); i++)
    {
        if (m_actors[i]->is_a_square() && m_actors[i]->getX() == x && m_actors[i]->getY() == y)
            return m_actors[i];
    }
    return nullptr;
}

void StudentWorld::exchangeSquare(double x, double y)
{
    for (int i = 0; i < m_actors.size(); i++)
    {
        if (m_actors[i]->is_a_square() && m_actors[i]->getX() == x && m_actors[i]->getY() == y)
        {
            delete m_actors[i];
            m_actors[i] = new DroppingSquare(this, IID_DROPPING_SQUARE, x, y);
        }
    }
}
