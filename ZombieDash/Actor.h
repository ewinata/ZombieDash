#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;
class Goodie;
class Penelope;

class Actor : public GraphObject
{
public:
	Actor(StudentWorld* world, int imageID, double startX, double startY,
		int depth = 0, int startDirection = right) : GraphObject(imageID,
			startX, startY, startDirection, depth), m_world(world) {};
	virtual void doSomething() = 0;
	// Is this actor dead?
	bool isDead() const { return m_dead; }
	// Mark this actor as dead.
	void setDead() { m_dead = true; }
	// If this is an activated object, perform its effect on a (e.g., for an
	// Exit have a use the exit).
	virtual void activateIfAppropriate(Actor* a) {}
	// If this object uses exits, use the exit.
	virtual void useExitIfAppropriate() {}
	// If this object can die by falling into a pit or burning, die.
	virtual void dieByFallOrBurnIfAppropriate() {}
	// If this object can be infected by vomit, get infected.
	virtual void beVomitedOnIfAppropriate() {}
	// If this object can pick up goodies, pick up g
	virtual void pickUpGoodieIfAppropriate(Goodie* g) {}
	// Does this object block agent movement?
	virtual bool blocksMovement() const { return false; }
	// Does this object block flames?
	virtual bool blocksFlame() const { return false; }
	// Does this object trigger landmines only when they're active?
	virtual bool triggersOnlyActiveLandmines() const { return false; }
	// Can this object cause a zombie to vomit?
	virtual bool triggersZombieVomit() const { return false; }
	// Is this object a threat to citizens?
	virtual bool threatensCitizens() const { return false; }
	StudentWorld* getWorld() const { return m_world;  }
private:
	StudentWorld* m_world;
	bool m_dead = false;
};

class Wall : public Actor
{
public:
	Wall(StudentWorld* world, double startX, double startY)
		: Actor(world, IID_WALL, startX, startY) {};
	virtual void doSomething() { return; }
	virtual bool blocksMovement() const { return true; }
	virtual bool blocksFlame() const { return true; }
};

class ActivatingObject : public Actor
{
public:
	ActivatingObject(StudentWorld* w, int imageID, double x, double y, int depth=0, int dir = right)
		: Actor(w, imageID, x, y, depth, dir) {};
	int getLiveCount() const { return m_liveCount; }
	void incrementLiveCount() { m_liveCount++; }
private:
	int m_liveCount = 0;
};

class Exit : public ActivatingObject
{
public:
	Exit(StudentWorld* world, double startX, double startY) : ActivatingObject(world,
		IID_EXIT, startX, startY, 1) {};
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
	virtual bool blocksFlame() const { return true; }
};

class Pit : public ActivatingObject
{
public:
	Pit(StudentWorld* world, double startX, double startY) : ActivatingObject(world,
		IID_PIT, startX, startY) {};
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
};

class Flame : public ActivatingObject
{
public:
	Flame(StudentWorld* world, double startX, double startY, int direction) :
		ActivatingObject(world, IID_FLAME, startX, startY, 0, direction) {};
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor *a);
};

class Vomit : public ActivatingObject
{
public:
	Vomit(StudentWorld* w, double x, double y, int direction)
		: ActivatingObject(w, IID_VOMIT, x, y, 0, direction) {}
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
};

class Landmine : public ActivatingObject
{
public:
	Landmine(StudentWorld* w, double x, double y)
		: ActivatingObject(w, IID_LANDMINE, x, y, 1) {}
	virtual void doSomething();
	virtual void activateIfAppropriate(Actor* a);
	virtual void dieByFallOrBurnIfAppropriate();
private:
	bool m_active = false;
};

class Goodie : public ActivatingObject
{
public:
	Goodie(StudentWorld* w, int imageID, double x, double y):
		ActivatingObject(w, imageID, x, y, 1) {}
	virtual void activateIfAppropriate(Actor* a);
	virtual void dieByFallOrBurnIfAppropriate();

	// Have p pick up this goodie.
	virtual void pickUp(Penelope* p) = 0;
};

class VaccineGoodie : public Goodie
{
public:
	VaccineGoodie(StudentWorld* w, double x, double y)
		: Goodie(w, IID_VACCINE_GOODIE, x, y) {}
	virtual void doSomething();
	virtual void pickUp(Penelope* p);
};

class GasCanGoodie : public Goodie
{
public:
	GasCanGoodie(StudentWorld* w, double x, double y)
		: Goodie(w, IID_GAS_CAN_GOODIE, x, y) {}
	virtual void doSomething();
	virtual void pickUp(Penelope* p);
};

class LandmineGoodie : public Goodie
{
public:
	LandmineGoodie(StudentWorld* w, double x, double y)
		: Goodie(w, IID_LANDMINE_GOODIE, x, y) {}
	virtual void doSomething();
	virtual void pickUp(Penelope* p);
};

class Agent : public Actor
{
public:
	Agent(StudentWorld* world, int imageID, double startX, double startY) : 
		Actor(world, imageID, startX, startY) {};
	virtual bool blocksMovement() const { return true; }
	virtual bool triggersOnlyActiveLandmines() const { return true; }
};

class Human : public Agent
{
public:
	Human(StudentWorld* w, int imageID, double x, double y)
		: Agent(w, imageID, x, y) {}
	virtual void beVomitedOnIfAppropriate();
	virtual bool triggersZombieVomit() const { return true; }
	// Make this human uninfected by vomit.
	void clearInfection() { m_infectCount = 0; }

	// How many ticks since this human was infected by vomit?
	int getInfectionDuration() const { return m_infectCount; }

	void setInfected(bool b) { m_infected = b; }
	void incrementInfected() { m_infectCount++; }
	bool getInfected() { return m_infected; }
private:
	bool m_infected = false;
	int m_infectCount = 0;
};

class Penelope : public Human
{
public:
	Penelope(StudentWorld* world, double startX, double startY) : Human(world,
		IID_PLAYER, startX, startY) {};
	virtual void doSomething();
	virtual void useExitIfAppropriate();
	virtual void dieByFallOrBurnIfAppropriate();
	virtual void pickUpGoodieIfAppropriate(Goodie* g);

	// Increase the number of vaccines the object has.
	void increaseVaccines() { m_vaccineCount++; }
	// Increase the number of flame charges the object has.
	void increaseFlameCharges() { m_flameCount+=5; }
	// Increase the number of landmines the object has.
	void increaseLandmines() { m_landCount+=2; }
	// How many vaccines does the object have?
	int getNumVaccines() const { return m_vaccineCount; }
	// How many flame charges does the object have?
	int getNumFlameCharges() const { return m_flameCount; }
	// How many landmines does the object have?
	int getNumLandmines() const { return m_landCount; }
private:
	int m_flameCount = 0;
	int m_landCount = 0;
	int m_vaccineCount = 0;
};

class Citizen : public Human
{
public:
	Citizen(StudentWorld* w, double x, double y)
		: Human(w, IID_CITIZEN, x, y) {}
	virtual void doSomething();
	virtual void useExitIfAppropriate();
	virtual void dieByFallOrBurnIfAppropriate();
	//x,y stands for the coordinate of closest zombie/penelope
	//returns true if the citizen has been moved
	bool move(double x, double y);
};

class Zombie : public Agent
{
public:
	Zombie(StudentWorld* w, double x, double y)
		: Agent(w, IID_ZOMBIE, x, y) {}
	virtual bool threatensCitizens() const { return true; }
	int getPlan() const { return m_plan; }
	void setPlan(int x) { m_plan = x; }
private:
	int m_plan = 0;
};

class DumbZombie : public Zombie
{
public:
	DumbZombie(StudentWorld* w, double x, double y)
		: Zombie(w, x, y) {}
	virtual void doSomething();
	virtual void dieByFallOrBurnIfAppropriate();
};

class SmartZombie : public Zombie
{
public:
	SmartZombie(StudentWorld* w, double x, double y)
		: Zombie(w, x, y) {}
	virtual void doSomething();
	virtual void dieByFallOrBurnIfAppropriate();
	bool move(double x, double y);
};

#endif // ACTOR_H_