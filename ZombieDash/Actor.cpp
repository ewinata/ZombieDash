#include "Actor.h"
#include "StudentWorld.h"
#include <cstdlib>
using namespace std;

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

/*-----------------------------------------------------------------------------------
		DOSOMETHING
------------------------------------------------------------------------------------*/
void Penelope::doSomething() {
	if (isDead())
		return;
	if (getInfected())
		incrementInfected();
	if (getInfectionDuration() >= 500)
	{
		setDead();
		getWorld()->playSound(SOUND_PLAYER_DIE);
		return;
	}
	int keyPressed;
	if(getWorld()->getKey(keyPressed))
		switch (keyPressed)
		{
		case KEY_PRESS_DOWN:
			setDirection(down);
			//if not wall, citizen or zombie
			if (getWorld()->canMove(getX(), getY() - 4, this))
				moveTo(getX(), getY() - 4);
			break;
		case KEY_PRESS_LEFT:
			setDirection(left);
			//if not wall, citizen or zombie
			if (getWorld()->canMove(getX() - 4, getY(), this))
				moveTo(getX() - 4, getY());
			break;
		case KEY_PRESS_RIGHT:
			setDirection(right);
			//if not wall, citizen or zombie
			if (getWorld()->canMove(getX() + 4, getY(), this)) 
				moveTo(getX() + 4, getY());
			break;
		case KEY_PRESS_UP:
			setDirection(up);
			//if not wall, citizen or zombie
			if (getWorld()->canMove(getX(), getY() + 4, this))
				moveTo(getX(), getY() + 4);
			break;
		case KEY_PRESS_ENTER:
			if (m_vaccineCount > 0)
			{
				setInfected(false);
				clearInfection();
				m_vaccineCount--;
			}
			break;
		case KEY_PRESS_SPACE:
			if (m_flameCount > 0)
			{
				m_flameCount--;
				getWorld()->playSound(SOUND_PLAYER_FIRE);
				double posX = getX(), posY = getY();
				for (int i = 1; i < 4; i++)
				{
					if (getDirection() == up)
						posY = getY() + i *SPRITE_HEIGHT;
					else if (getDirection() == left)
						posX = getX() - i *SPRITE_HEIGHT;
					else if (getDirection() == right)
						posX = getX() + i *SPRITE_HEIGHT;
					else if (getDirection() == down)
						posY = getY() - i *SPRITE_HEIGHT;
					//if flame at posX posY overlaps with a wall or exit object
					//then stop the loop: break;
					//else add a flame object at posX posY
					if (!getWorld()->isFlameBlockedAt(posX, posY))
						getWorld()->addActor(new Flame(getWorld(), posX, posY, getDirection()));
					else
						break;
				}
			}
			break;
		case KEY_PRESS_TAB:
			if (m_landCount > 0)
			{
				getWorld()->addActor(new Landmine(getWorld(), getX(), getY()));
				m_landCount--;
			}
			break;
		default:
			break;
		}
}

void Citizen::doSomething()
{
	if (isDead())
		return;
	if (getInfected())
		incrementInfected();
	if (getInfectionDuration() >= 500)
	{
		setDead();
		getWorld()->decrementCitizen();
		getWorld()->playSound(SOUND_ZOMBIE_BORN);
		getWorld()->increaseScore(-1000);
		//70% dumb, 30% smart
		int x = rand() % 100; // range from 0 to 99
		if (x <= 29)
			getWorld()->addActor(new SmartZombie(getWorld(), getX(), getY()));
		else
			getWorld()->addActor(new DumbZombie(getWorld(), getX(), getY()));
		return;
	}
	if (getWorld()->getTick() % 2 == 0) //even tick
		return;
	double x, y, distance;
	bool isThreat;
	if (!getWorld()->locateNearestCitizenTrigger(getX(), getY(), x, y, distance, isThreat))
		return; //No zombie or penelope alive
	if (distance <= 80 && !isThreat)
	{
		//Penelope is closer to citizen
		if (getX() == x || getY() == y)
		{
			//If same row or same column
			if (move(x, y))
				return;
		}
		else
		{
			//If not same row or same column
			int randmove = rand() % 2; // 0 to 1
			if (randmove == 0)
			{
				if (move(x, -1))
					return;
				//if cannot move in x direction
				if (move(-1, y))
					return;
			}
			else
			{
				if (move(-1, y))
					return;
				//if cannot move in y direction
				if (move(x, -1))
					return;
			}
		}
	}
	else if (distance <= 80)
	{
		double m_x = -1, m_y = -1;
		//Zombie's closer or equal to Penelope
		for (int i = 0; i < 4; i++)
		{
			switch (i)
			{
			default:
				break;
			case 0:
				//UP
				if (getWorld()->canMove(getX(), getY() + 2, this))
				{
					double dd = getWorld()->getDist(x, y, getX(), getY() + 2);
					if (dd > distance)
					{
						distance = dd;
						m_y = getY() + 2;
						m_x = getX();
					}
				}
				break;
			case 1:
				//DOWN
				if (getWorld()->canMove(getX(), getY() - 2, this))
				{
					double dd = getWorld()->getDist(x, y, getX(), getY() - 2);
					if (dd > distance)
					{
						distance = dd;
						m_y = getY() - 2;
						m_x = getX();
					}
				}
				break;
			case 2:
				//LEFT
				if (getWorld()->canMove(getX() - 2, getY(), this))
				{
					double dd = getWorld()->getDist(x, y, getX() - 2, getY());
					if (dd > distance)
					{
						distance = dd;
						m_x = getX() - 2;
						m_y = getY();
					}
				}
				break;
			case 3:
				//RIGHT
				if (getWorld()->canMove(getX() + 2, getY(), this))
				{
					double dd = getWorld()->getDist(x, y, getX() + 2, getY());
					if (dd > distance)
					{
						distance = dd;
						m_x = getX() + 2;
						m_y = getY();
					}
				}
				break;
			}
		}
		if (m_x < 0 || m_y < 0)
			return;
		else
			moveTo(m_x, m_y);
		return;
	}
}

bool Citizen::move(double x, double y)
{
	if (x < 0 && y < 0) //invalid coordinates
		return false;
	// if y is less than 0, then only move x;
	if ( y >= 0)
		if (getY() < y)
		{
			if (getWorld()->canMove(getX(), getY() + 2, this))
			{
				setDirection(up);
				moveTo(getX(), getY() + 2);
				return true;
			}
		}
		else
		{
			if (getWorld()->canMove(getX(), getY() - 2, this))
			{
				setDirection(down);
				moveTo(getX(), getY() - 2);
				return true;
			}
		}
	// if x is less than 0, then only move y
	if ( x >= 0)
		if (getX() < x)
		{
			if (getWorld()->canMove(getX() + 2, getY(), this))
			{
				setDirection(right);
				moveTo(getX() + 2, getY());
				return true;
			}
		}
		else
		{
			if (getWorld()->canMove(getX() - 2, getY(), this))
			{
				setDirection(left);
				moveTo(getX() - 2, getY());
				return true;
			}
		}
	return false;
}

void Exit::doSomething()
{
	getWorld()->activateOnAppropriateActors(this);
}


void Pit::doSomething()
{
	getWorld()->activateOnAppropriateActors(this);
}

void Flame::doSomething()
{
	if (isDead())
		return;
	incrementLiveCount();
	if (getLiveCount() == 2)
	{
		setDead();
		return;
	}
	//damage all damageable objects that overlap with the flame
	getWorld()->activateOnAppropriateActors(this);
}

void Vomit::doSomething()
{
	if (isDead())
		return;
	incrementLiveCount();
	if (getLiveCount() == 2)
	{
		setDead();
		return;
	}
	getWorld()->activateOnAppropriateActors(this);
}

void Landmine::doSomething()
{
	if (isDead())
		return;
	if (!m_active)
	{
		incrementLiveCount();
		if (getLiveCount() == 30)
		{
			m_active = true;
		}
		return;
	}
	getWorld()->activateOnAppropriateActors(this);
}

void VaccineGoodie::doSomething()
{
	if (isDead())
		return;
	getWorld()->activateOnAppropriateActors(this);
}

void GasCanGoodie::doSomething()
{
	if (isDead())
		return;
	getWorld()->activateOnAppropriateActors(this);
}

void LandmineGoodie::doSomething()
{
	if (isDead())
		return;
	getWorld()->activateOnAppropriateActors(this);
}

void DumbZombie::doSomething()
{
	if (isDead())
		return;
	if (getWorld()->getTick() % 2 == 0)
		return;
	double x = -1, y = -1;
	if (getDirection() == left)
		if (getWorld()->isZombieVomitTriggerAt(getX() - SPRITE_WIDTH, getY()))
		{
			x = getX() - SPRITE_WIDTH;
			y = getY();
		}
	if (getDirection() == right)
		if (getWorld()->isZombieVomitTriggerAt(getX() + SPRITE_WIDTH, getY()))
		{
			x = getX() + SPRITE_WIDTH;
			y = getY();
		}
	if (getDirection() == up)
		if (getWorld()->isZombieVomitTriggerAt(getX(), getY() + SPRITE_HEIGHT))
		{
			x = getX();
			y = getY() + SPRITE_HEIGHT;
		}
	if (getDirection() == down)
		if (getWorld()->isZombieVomitTriggerAt(getX(), getY() - SPRITE_HEIGHT))
		{
			x = getX();
			y = getY() - SPRITE_HEIGHT;
		}
	if (x >= 0 && y >= 0)
	{
		//Vomit coordinates are updated
		int v = rand() % 3; // 0 to 2
		if (v == 1)
		{
			getWorld()->addActor(new Vomit(getWorld(), x, y, getDirection()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	if (getPlan() == 0)
	{
		//moveplan
		int x = (rand() % 8) + 3; // 3 to 10
		setPlan(x);
		//direction
		int d = rand() % 4; // 0 to 3
		switch (d)
		{
		default:
			break;
		case 0:
			setDirection(up);
			break;
		case 1:
			setDirection(down);
			break;
		case 2:
			setDirection(left);
			break;
		case 3:
			setDirection(right);
			break;
		}
	}
	int dir = getDirection();
	switch (dir)
	{
	default:
		break;
	case up:
		if (getWorld()->canMove(getX(), getY() + 1, this))
		{
			moveTo(getX(), getY() + 1);
			setPlan(getPlan() - 1);
		}
		else
			setPlan(0);
		break;
	case down:
		if (getWorld()->canMove(getX(), getY() - 1, this))
		{
			moveTo(getX(), getY() - 1);
			setPlan(getPlan() - 1);
		}
		else
			setPlan(0);
		break;
	case left:
		if (getWorld()->canMove(getX() - 1, getY(), this))
		{
			moveTo(getX() - 1, getY());
			setPlan(getPlan() - 1);
		}
		else
			setPlan(0);
		break;
	case right:
		if (getWorld()->canMove(getX() + 1, getY(), this))
		{
			moveTo(getX() + 1, getY());
			setPlan(getPlan() - 1);
		}
		else
			setPlan(0);
		break;
	}
}

void SmartZombie::doSomething()
{
	if (isDead())
		return;
	if (getWorld()->getTick() % 2 == 0)
		return;
	double x = -1, y = -1;
	if (getDirection() == left)
		if (getWorld()->isZombieVomitTriggerAt(getX() - SPRITE_WIDTH, getY()))
		{
			x = getX() - SPRITE_WIDTH;
			y = getY();
		}
	if (getDirection() == right)
		if (getWorld()->isZombieVomitTriggerAt(getX() + SPRITE_WIDTH, getY()))
		{
			x = getX() + SPRITE_WIDTH;
			y = getY();
		}
	if (getDirection() == up)
		if (getWorld()->isZombieVomitTriggerAt(getX(), getY() + SPRITE_HEIGHT))
		{
			x = getX();
			y = getY() + SPRITE_HEIGHT;
		}
	if (getDirection() == down)
		if (getWorld()->isZombieVomitTriggerAt(getX(), getY() - SPRITE_HEIGHT))
		{
			x = getX();
			y = getY() - SPRITE_HEIGHT;
		}
	if (x >= 0 && y >= 0)
	{
		//Vomit coordinates are updated
		int z = rand() % 3; // 0 to 2
		if (z == 1)
		{
			getWorld()->addActor(new Vomit(getWorld(), x, y, getDirection()));
			getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
			return;
		}
	}
	double px = -1, py = -1;
	if (getPlan() == 0)
	{
		//moveplan
		int x = (rand() % 8) + 3; // 3 to 10
		setPlan(x);
		int d = rand() % 4; // 0 to 3
		switch (d)
		{
		default:
			break;
		case 0:
			setDirection(up);
			break;
		case 1:
			setDirection(down);
			break;
		case 2:
			setDirection(left);
			break;
		case 3:
			setDirection(right);
			break;
		}
	}
	switch (getDirection())
	{
	default:
		break;
	case up:
		px = getX();
		py = getY() + 1;
		break;
	case down:
		px = getX();
		py = getY() - 1;
		break;
	case left:
		px = getX() - 1;
		py = getY();
		break;
	case right:
		px = getX() + 1;
		py = getY();
		break;
	}
	//get the direction to go
	double distance, otherX, otherY;
	if (!getWorld()->locateNearestCitizenThreat(getX(), getY(), otherX, otherY, distance))
		return; //no zombie (although this code shouldnt be reached)
	if (distance > 80)
	{
		if (getWorld()->canMove(px, py, this))
		{
			moveTo(px, py);
			setPlan(getPlan() - 1);
		}
		else
			setPlan(0);
	}
	else
	{
		if (getX() == otherX || getY() == otherY)
		{
			//If same row or same column
			if (move(otherX, otherY))
			{
				setPlan(getPlan() - 1);
				return;
			}
			else
				setPlan(0);
		}
		else
		{
			//If not same row or same column
			int randmove = rand() % 2; // 0 to 1
			if (randmove == 0)
			{
				if (move(otherX, -1))
				{
					setPlan(getPlan() - 1);
					return;
				}
				//if cannot move in x direction
				else
					setPlan(0);
			}
			else
			{
				if (move(-1, otherY))
				{
					setPlan(getPlan() - 1);
					return;
				}
				//if cannot move in y direction
				else
					setPlan(0);
			}
		}
	}

}

bool SmartZombie::move(double x, double y)
{
	if (x < 0 && y < 0) //invalid coordinates
		return false;
	// if y is less than 0, then only move x;
	if (y >= 0)
		if (getY() < y)
		{
			setDirection(up);
			if (getWorld()->canMove(getX(), getY() + 1, this))
			{
				moveTo(getX(), getY() + 1);
				return true;
			}
		}
		else
		{
			setDirection(down);
			if (getWorld()->canMove(getX(), getY() - 1, this))
			{
				moveTo(getX(), getY() - 2);
				return true;
			}
		}
	// if x is less than 0, then only move y
	if (x >= 0)
		if (getX() < x)
		{
			setDirection(right);
			if (getWorld()->canMove(getX() + 1, getY(), this))
			{
				moveTo(getX() + 1, getY());
				return true;
			}
		}
		else
		{
			setDirection(left);
			if (getWorld()->canMove(getX() - 1, getY(), this))
			{
				moveTo(getX() - 1, getY());
				return true;
			}
		}
	return false;
}

/*--------------------------------------------------------------------------------
		DIEBYFALLORBURN
--------------------------------------------------------------------------------*/

void DumbZombie::dieByFallOrBurnIfAppropriate()
{
	bool thrown = false;
	int x = rand() % 10; // 0 to 9
	if (x == 0)
	{
		int c = 0;
		while (!thrown)
		{
			double mx = -1, my = -1;
			int d = rand() % 4; // 0 to 3
			switch (d)
			{
			default:
				break;
			case 0:
				//UP
				my = getY() + SPRITE_HEIGHT;
				mx = getX();
				c++;
				break;
			case 1:
				//DOWN
				my = getY() - SPRITE_HEIGHT;
				mx = getX();
				c++;
				break;
			case 2:
				//LEFT
				my = getY();
				mx = getX() - SPRITE_WIDTH;
				c++;
				break;
			case 3:
				//RIGHT
				my = getY();
				mx = getX() + SPRITE_WIDTH;
				c++;
				break;
			}
			if (mx >= 0 && my >= 0 && !getWorld()->vaccineOverlap(mx, my))
			{
				getWorld()->addActor(new VaccineGoodie(getWorld(), mx, my));
				thrown = true;
			}
			else //tries next direction
				if (d == 3)
					d = 0;
				else
					d++;
			if (c >= 4) // all options exhausted
				thrown = true;
		}
	}
	getWorld()->playSound(SOUND_ZOMBIE_DIE);
	getWorld()->increaseScore(1000);
	setDead();
}

void SmartZombie::dieByFallOrBurnIfAppropriate()
{
	getWorld()->playSound(SOUND_ZOMBIE_DIE);
	getWorld()->increaseScore(2000);
	setDead();
}

void Landmine::dieByFallOrBurnIfAppropriate()
{
	activateIfAppropriate(nullptr);
}

void Penelope::dieByFallOrBurnIfAppropriate()
{
	setDead();
	getWorld()->playSound(SOUND_PLAYER_DIE);
}

void Citizen::dieByFallOrBurnIfAppropriate()
{
	setDead();
	getWorld()->decrementCitizen();
	getWorld()->playSound(SOUND_CITIZEN_DIE);
	getWorld()->increaseScore(-1000);
}

void Goodie::dieByFallOrBurnIfAppropriate()
{
	setDead();
}

/*--------------------------------------------------------------------------------
		USEEXITIFAPPROPRIATE
--------------------------------------------------------------------------------*/

void Penelope::useExitIfAppropriate()
{
	getWorld()->recordLevelFinishedIfAllCitizensGone();
}

void Citizen::useExitIfAppropriate()
{
	//Inform StudentWorld object that user is to receive 500 points
	getWorld()->increaseScore(500);
	setDead();
	getWorld()->decrementCitizen();
	getWorld()->playSound(SOUND_CITIZEN_SAVED);
}

/*--------------------------------------------------------------------------------
		BEVOMMITTEDONIFAPPROPRIATE
--------------------------------------------------------------------------------*/

void Human::beVomitedOnIfAppropriate()
{
	setInfected(true);
}

/*--------------------------------------------------------------------------------
		ACTIVATEIFAPPROPRIATE
--------------------------------------------------------------------------------*/

void Exit::activateIfAppropriate(Actor* a)
{
	a->useExitIfAppropriate();
}


void Pit::activateIfAppropriate(Actor* a)
{
	a->dieByFallOrBurnIfAppropriate();
}

void Flame::activateIfAppropriate(Actor* a)
{
	a->dieByFallOrBurnIfAppropriate();
}

void Vomit::activateIfAppropriate(Actor* a)
{
	a->beVomitedOnIfAppropriate();
}

void Landmine::activateIfAppropriate(Actor* a)
{
	setDead();
	getWorld()->playSound(SOUND_LANDMINE_EXPLODE);
	getWorld()->addActor(new Flame(getWorld(), getX(), getY(), up));
	//NORTH
	getWorld()->addActor(new Flame(getWorld(), getX(), getY() + 16, up));
	//NORTHEAST
	getWorld()->addActor(new Flame(getWorld(), getX() + 16, getY() + 16, up));
	//EAST
	getWorld()->addActor(new Flame(getWorld(), getX() + 16, getY(), up));
	//SOUTHEAST
	getWorld()->addActor(new Flame(getWorld(), getX() + 16, getY() - 16, up));
	//SOUTH
	getWorld()->addActor(new Flame(getWorld(), getX(), getY() - 16, up));
	//SOUTHWEST
	getWorld()->addActor(new Flame(getWorld(), getX() - 16, getY() - 16, up));
	//WEST
	getWorld()->addActor(new Flame(getWorld(), getX() - 16, getY(), up));
	//NORTHWEST
	getWorld()->addActor(new Flame(getWorld(), getX() - 16, getY() + 16, up));
	getWorld()->addActor(new Pit(getWorld(), getX(), getY()));
}

void Goodie::activateIfAppropriate(Actor* a)
{
	a->pickUpGoodieIfAppropriate(this);
}

/*--------------------------------------------------------------------------------
		PICKUPGOODIEIFAPPROPRIATE
--------------------------------------------------------------------------------*/

void Penelope::pickUpGoodieIfAppropriate(Goodie* g)
{
	getWorld()->increaseScore(50);
	g->setDead();
	g->pickUp(this);
	getWorld()->playSound(SOUND_GOT_GOODIE);
}

/*--------------------------------------------------------------------------------
		PICKUP
--------------------------------------------------------------------------------*/
void VaccineGoodie::pickUp(Penelope* p)
{
	p->increaseVaccines();
}

void LandmineGoodie::pickUp(Penelope* p)
{
	p->increaseLandmines();
}

void GasCanGoodie::pickUp(Penelope* p)
{
	p->increaseFlameCharges();
}