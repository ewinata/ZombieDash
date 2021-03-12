#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <vector>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;
class Penelope;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
	~StudentWorld();
    virtual int init(); //load level.h
    virtual int move();
    virtual void cleanUp();

	void levelFinished() { m_levelFinished = true; }
	void levelNotFinished() { m_levelFinished = false; }
	void incrementTick() { m_tick++; }
	void decrementCitizen() { m_citizen--; }
	int getTick() { return m_tick; }
	double getDist(double x, double y, double p_x, double p_y) const;
	bool overlap(double x, double y, double p_x, double p_y) const;
	// Add an actor to the world.
	void addActor(Actor* a);

	// Indicate that the player has finished the level if all citizens
	// are gone.
	void recordLevelFinishedIfAllCitizensGone();

	// For each actor overlapping a, activate a if appropriate.
	void activateOnAppropriateActors(Actor* a);

	// Is an agent blocked from moving to the indicated location?
	bool canMove(double x, double y, Actor* a) const;

	// Is creation of a flame blocked at the indicated location?
	bool isFlameBlockedAt(double x, double y) const;

	// Is there something at the indicated location that might cause a
	// zombie to vomit (i.e., a human)?
	bool isZombieVomitTriggerAt(double x, double y) const;

	// Return true if there is a living human, otherwise false.  If true,
	// otherX, otherY, and distance will be set to the location and distance
	// of the human nearest to (x,y).
	bool locateNearestVomitTrigger(double x, double y, double& otherX, double& otherY, double& distance) const;

	// Return true if there is a living zombie or Penelope, otherwise false.
	// If true, otherX, otherY, and distance will be set to the location and
	// distance of the one nearest to (x,y), and isThreat will be set to true
	// if it's a zombie, false if a Penelope. (if d_penelope < d_zombie, isThreat is false)
	bool locateNearestCitizenTrigger(double x, double y, double& otherX, double& otherY, double& distance, bool& isThreat) const;

	// Return true if there is a living zombie, false otherwise.  If true,
	  // otherX, otherY and distance will be set to the location and distance
	  // of the one nearest to (x,y).
	bool locateNearestCitizenThreat(double x, double y, double& otherX, double& otherY, double& distance) const;

	bool vaccineOverlap(double x, double y) const;

private:
	std::vector<Actor*> m_actor;
	Penelope* m_penelope = nullptr;
	int m_tick = 0;
	int m_citizen = 0;
	bool m_levelFinished = false;
};

#endif // STUDENTWORLD_H_
