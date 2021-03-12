#include "StudentWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include "Actor.h"
#include <string>
#include <iostream>
#include <cmath>
#include <sstream>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::~StudentWorld()
{
	cleanUp();
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
}

int StudentWorld::init()
{
	Level lev(assetPath());

	string levelno = to_string(getLevel());
	if (levelno.length() < 2)
		levelno = "0" + levelno;
	string levelFile = "level" + levelno + ".txt";
	Level::LoadResult result = lev.loadLevel(levelFile);
	if (result == Level::load_fail_file_not_found || getLevel() > 99)
		return GWSTATUS_PLAYER_WON;
	else if (result == Level::load_fail_bad_format)
		return GWSTATUS_LEVEL_ERROR;
	else if (result == Level::load_success)
	{

		for (int i = 0; i < LEVEL_WIDTH; i++) //X
		{
			for (int j = 0; j < LEVEL_HEIGHT; j++) //Y
			{
				Level::MazeEntry ge = lev.getContentsOf(i, j);
				switch (ge)
				{
				case Level::empty:
					break;
				case Level::player:
					m_penelope = new Penelope(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT);
					break;
				case Level::wall:
					addActor(new Wall(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				case Level::citizen:
					m_citizen++;
					addActor(new Citizen(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				case Level::dumb_zombie:
					addActor(new DumbZombie(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				case Level::exit:
					addActor(new Exit(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				case Level::gas_can_goodie:
					addActor(new GasCanGoodie(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				case Level::landmine_goodie:
					addActor(new LandmineGoodie(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				case Level::pit:
					addActor(new Pit(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				case Level::smart_zombie:
					addActor(new SmartZombie(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				case Level::vaccine_goodie:
					addActor(new VaccineGoodie(this, i * LEVEL_WIDTH, j * LEVEL_HEIGHT));
					break;
				default:
					cout << "Error in level file" << endl;
					break;
				}
			}
		}
	}
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::addActor(Actor* a)
{
	m_actor.push_back(a);
}

int StudentWorld::move()
{
	m_penelope->doSomething();
	if (m_penelope->isDead())
	{
		decLives();
		m_citizen = 0;
		return GWSTATUS_PLAYER_DIED;
	}
	for (vector<Actor*>::iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if (!(*it)->isDead())
		{
			(*it)->doSomething();
			if (m_penelope->isDead())
			{
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}

			if (m_levelFinished)
			{
				m_tick = 0;
				playSound(SOUND_LEVEL_FINISHED);
				levelNotFinished();
				return GWSTATUS_FINISHED_LEVEL;
			}
		}
	}
	incrementTick();
	//update status, remove all dead actors
	for (vector<Actor*>::iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if ((*it)->isDead())
		{
			delete *it;
			it = m_actor.erase(it);
			it--;
		}
	}
	//m_actor.shrink_to_fit();
	
	stringstream txt;
	txt << "Score: ";
	int z = (to_string(getScore()).length());
	int score = getScore();
	if (score < 0)
	{
		txt << "-";
		score = score * -1;
	}
	for (int i = 0; i < 6 - z; i++)
	{
		txt << "0";
	}
	txt << to_string(score);
	txt << "  Level: ";
	txt << to_string(getLevel());
	txt << "  Lives: " + to_string(getLives());
	txt << "  Vaccines: " + to_string(m_penelope->getNumVaccines());
	txt << "  Flames: " + to_string(m_penelope->getNumFlameCharges());
	txt << "  Mines: " + to_string(m_penelope->getNumLandmines());
	txt << "  Infected: " + to_string(m_penelope->getInfectionDuration());
	setGameStatText(txt.str());
	
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	for (vector<Actor*>::iterator it = m_actor.begin(); it != m_actor.end();)
	{
		delete *it;
		it = m_actor.erase(it);
	}
	if (m_penelope != nullptr)
		delete m_penelope;
	//m_actor.shrink_to_fit();
	m_actor.clear();
}

bool StudentWorld::vaccineOverlap(double x, double y) const
{
	for (vector<Actor*>:: const_iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if (overlap((*it)->getX(), (*it)->getY(), x, y))
			return true;
	}
	if (overlap(m_penelope->getX(), m_penelope->getY(), x, y))
		return true;
	return false;
}


bool StudentWorld::isFlameBlockedAt(double x, double y) const
{
	for (vector<Actor*>::const_iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if (!(*it)->blocksFlame())
			continue;
		if (overlap((*it)->getX(), (*it)->getY(), x, y))
			return true;
	}
	return false;
}

bool StudentWorld::canMove(double x, double y, Actor* a) const
{
	int xUpper = x + SPRITE_WIDTH - 1;
	int yUpper = y + SPRITE_HEIGHT - 1;
	for (vector<Actor*>::const_iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if (!(*it)->blocksMovement() || (*it) == a)
			continue;
		int c = 0;
		int it_xUpper = (*it)->getX() + SPRITE_WIDTH - 1;
		int it_yUpper = (*it)->getY() + SPRITE_HEIGHT - 1;
		if ((x >= (int)(*it)->getX() && x <= it_xUpper)
			|| (xUpper >= (int)(*it)->getX() && xUpper <= it_xUpper))
			c++;
		if ((y >= (int)(*it)->getY() && y <= it_yUpper)
			|| (yUpper >= (int)(*it)->getY() && yUpper <= it_yUpper))
			c++;
		if (c == 2)
			return false;
	}
	if (a != m_penelope)
	{
		int c1 = 0;
		int p_xUpper = m_penelope->getX() + SPRITE_WIDTH - 1;
		int p_yUpper = m_penelope->getY() + SPRITE_HEIGHT - 1;
		if ((x >= (int)m_penelope->getX() && x <= p_xUpper)
			|| (xUpper >= (int)m_penelope->getX() && xUpper <= p_xUpper))
			c1++;
		if ((y >= (int)m_penelope->getY() && y <= p_yUpper)
			|| (yUpper >= (int)m_penelope->getY() && yUpper <= p_yUpper))
			c1++;
		if (c1 == 2)
			return false;
	}
	return true;
}

bool StudentWorld::overlap(double x, double y, double p_x, double p_y) const
{
	return pow(x - p_x, 2) + pow(y - p_y, 2) <= 100;
}

void StudentWorld::activateOnAppropriateActors(Actor* a)
{
	//other actors
	vector<Actor*>::iterator it = m_actor.begin();
	int size = m_actor.size();
	for (int i = 0; i < size; i++)
	{
		if (overlap(a->getX(), a->getY(), (*it)->getX(), (*it)->getY()) && !(*it)->isDead() && a != (*it))
			a->activateIfAppropriate(*it);
		it++;
	}
	//penelope
	if (overlap(a->getX(), a->getY(), m_penelope->getX(), m_penelope->getY()))
		a->activateIfAppropriate(m_penelope);
}

void StudentWorld::recordLevelFinishedIfAllCitizensGone()
{
	//cout << m_citizen << endl;
	//If all citizens are gone
	if (m_citizen == 0)
		levelFinished();
}

// Is there something at the indicated location that might cause a
	// zombie to vomit (i.e., a human)?
bool StudentWorld::isZombieVomitTriggerAt(double x, double y) const
{
	for (vector<Actor*>::const_iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if ((*it)->triggersZombieVomit() && !((*it)->isDead()))
		{
			if (overlap(x, y, (*it)->getX(), (*it)->getY()))
				return true;
		}
	}
	//penelope
	if (overlap(x, y, m_penelope->getX(), m_penelope->getY()) && !m_penelope->isDead())
		return true;
	return false;
}

// Return true if there is a living human, otherwise false.  If true,
	// otherX, otherY, and distance will be set to the location and distance
	// of the human nearest to (x,y).
bool StudentWorld::locateNearestVomitTrigger(double x, double y, double& otherX, double& otherY, double& distance) const
{
	if (m_penelope->isDead())
		return false;
	distance = -1;
	bool found = false;
	for (vector<Actor*>::const_iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if ((*it)->triggersZombieVomit() && !(*it)->isDead())
		{
			found = true;
			if (distance < 0)
			{
				distance = getDist(x, y, (*it)->getX(), (*it)->getY());
				otherX = (*it)->getX();
				otherY = (*it)->getY();
				continue;
			}
			double d = getDist(x, y, (*it)->getX(), (*it)->getY());
			if (distance > d)
			{
				distance = d;
				otherX = (*it)->getX();
				otherY = (*it)->getY();
			}
		}
	}
	double d_pen = getDist(x, y, m_penelope->getX(), m_penelope->getY());
	if (distance > d_pen || distance < 0)
	{
		distance = d_pen;
		otherX = m_penelope->getX();
		otherY = m_penelope->getY();
	}
	if (!m_penelope->isDead())
		found = true;
	return found;
}

double StudentWorld::getDist(double x, double y, double p_x, double p_y) const
{
	return pow(pow(x - p_x, 2.0) + pow(y - p_y, 2.0), 0.5);
}

// Return true if there is a living zombie or Penelope, otherwise false.
// If true, otherX, otherY, and distance will be set to the location and
// distance of the one nearest to (x,y), and isThreat will be set to true
// if it's a zombie, false if a Penelope. (if d_penelope < d_zombie, isThreat is false)
bool StudentWorld::locateNearestCitizenTrigger(double x, double y, double& otherX, double& otherY, double& distance, bool& isThreat) const
{
	distance = -1;
	bool found = false;
	if (m_penelope->isDead())
		return false;
	else
		found = true;
	for (vector<Actor*>::const_iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if ((*it)->threatensCitizens())
		{
			found = true;
			//Zombie
			if (distance < 0)
			{
				distance = getDist(x, y, (*it)->getX(), (*it)->getY());
				otherX = (*it)->getX();
				otherY = (*it)->getY();
				continue;
			}
			double d = getDist(x, y, (*it)->getX(), (*it)->getY());
			if (distance > d)
			{
				distance = d;
				otherX = (*it)->getX();
				otherY = (*it)->getY();
			}
		}
	}
	double d_pen = getDist(x, y, m_penelope->getX(), m_penelope->getY());
	if (distance > d_pen || distance < 0)
	{
		isThreat = false;
		distance = d_pen;
		otherX = m_penelope->getX();
		otherY = m_penelope->getY();
	}
	else
	{
		isThreat = true;
	}
	return found;
}

// Return true if there is a living zombie, false otherwise.  If true,
	  // otherX, otherY and distance will be set to the location and distance
	  // of the one nearest to (x,y).
bool StudentWorld::locateNearestCitizenThreat(double x, double y, double& otherX, double& otherY, double& distance) const
{
	bool found = false;
	distance = -1;
	for (vector<Actor*>::const_iterator it = m_actor.begin(); it != m_actor.end(); it++)
	{
		if ((*it)->threatensCitizens())
			found = true;
		if ((*it)->triggersZombieVomit())
		{
			//Citizen
			if (distance < 0)
			{
				distance = getDist(x, y, (*it)->getX(), (*it)->getY());
				otherX = (*it)->getX();
				otherY = (*it)->getY();
				continue;
			}
			double d = getDist(x, y, (*it)->getX(), (*it)->getY());
			if (distance > d)
			{
				distance = d;
				otherX = (*it)->getX();
				otherY = (*it)->getY();
			}
		}
	}
	double d_pen = getDist(x, y, m_penelope->getX(), m_penelope->getY());
	if (distance > d_pen || distance < 0)
	{
		distance = d_pen;
		otherX = m_penelope->getX();
		otherY = m_penelope->getY();
	}
	return found;
}