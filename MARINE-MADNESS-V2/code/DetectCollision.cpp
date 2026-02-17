#include "MarineMachine.h"
#include "SoundManager.h"
#include <iostream>
#include <typeinfo>

using namespace std;

extern SoundManager soundManager2;
extern Time gameTimeTotal;

void MarineMachine::detectCollision()
{
	// Bullet hitting an enemy
	for (int i = 0; i < 100; ++i)
	{
		if (!bullets[i].isInFlight()) continue;

		for (auto& enemy : enemies)
		{
			if (!enemy->isAlive()) continue;

			if (bullets[i].getPosition().intersects(enemy->getPosition()))
			{
				bullets[i].stop();

				if (enemy->hit())
				{
					// RNG
					srand(time(0));
					int pts = (rand() % 20) + 5;

					// Update score
					m_ScoreSystem.addPoints(pts);
				}

				// Play appropriate sound
				string typeName = typeid(*enemy).name();
				if (typeName.find("Dinosaur") != string::npos)
				{
					soundManager2.playSplat();
				}
				else if (typeName.find("Cowboy") != string::npos)
				{
					soundManager2.playSplat();
				}
				else if (typeName.find("Android") != string::npos)
				{
					soundManager2.playBoom();
				}
			}
		}
	}

	// Enemy is attacked by melee weapon
	for (auto& enemy : enemies)
	{
		if (meleeAttackRect.getGlobalBounds().intersects(enemy->getPosition()))
		{
			if (enemy->hit())
			{
				// Remove collision for weapon
				meleeAttackRect.setPosition(2000, 60);
				
				// RNG
				srand(time(0));
				int pts = (rand() % 30) + 10;

				// Update score
				m_ScoreSystem.addPoints(pts);

				// Update health; risk-reward type of system
				marine.setHealth(marine.getHealth() + (pts * 2));
			}
		}
	}


	// Make a rect for all his parts
	FloatRect detectionZone = marine.getPosition();

	// Make a FloatRect to test each block
	FloatRect block;

	// Allign block dimensions with dimensions of map tiles
	block.width = TILE_SIZE;
	block.height = TILE_SIZE;

	// Build a zone around player to detect collisions
	int startX = (int)(round(detectionZone.left) / TILE_SIZE) - 1;
	int startY = (int)(round(detectionZone.top) / TILE_SIZE) - 1;
	int endX = (int)(round(detectionZone.left) / TILE_SIZE) + 2;
	int endY = (int)round((detectionZone.top) / TILE_SIZE) + 2;

	// Make sure we don't test positions lower than zero
	// Or higher than the end of the array
	if (startX < 0)startX = 0;
	if (startY < 0)startY = 0;
	if (endX >= lm.getLevelSize().x)
		endX = lm.getLevelSize().x;
	if (endY >= lm.getLevelSize().y)
		endY = lm.getLevelSize().y;

	// Collision detection
	for (int x = startX; x < endX; x++)
	{
		for (int y = startY; y < endY; y++)
		{
			// Block tile, representing the top and left sides
			block.left = x * TILE_SIZE;
			block.top = y * TILE_SIZE;

			// Is player colliding with a wall?
			if (m_ArrayLevel[y][x] == 0)
			{
				// Check if player collides with wall on left or right sides
				if (marine.getRight().intersects(block))
				{
					marine.stopRight(block.left);
				}
				else if (marine.getLeft().intersects(block))
				{
					marine.stopLeft(block.left);
				}

				// Check if player collides with wall on top or bottom sides
				if (marine.getBottom().intersects(block))
				{
					marine.stopDown(block.top);
				}
				else if (marine.getTop().intersects(block))
				{
					marine.stopUp((block.top));
				}

				for (int i = 0; i < 100; i++)
				{
					// The enemy has shot at a wall
					if (enemyBullets[i].getPosition().intersects(block) && m_ArrayLevel[y][x] == 0)
					{
						// Destroy bullet
						enemyBullets[i].stop();
					}
				}
			}
		}
	}
	
	// Enemy bullets hitting the player/wall
	for (int i = 0; i < 100; ++i)
	{
		if (!enemyBullets[i].isInFlight()) continue;

		if (enemyBullets[i].getPosition().intersects(marine.getPosition()) && marine.getHealth() > 0)
		{
			enemyBullets[i].stop();

			if (marine.hit(gameTimeTotal))
			{
				soundManager2.playHit();
			}

			if (marine.getHealth() <= 0)
			{
				m_ScoreSystem.saveScores();
				resetGame();
				state = State::MAIN_MENU;
				return;
			}
		}
	}

	// Enemies intersect the player
	for (auto& enemy : enemies)
	{
		if (enemy->isAlive() && marine.getPosition().intersects(enemy->getPosition()))
		{
			if (marine.hit(gameTimeTotal))
			{
				soundManager2.playHit();
			}

			if (marine.getHealth() <= 0)
			{
				m_ScoreSystem.saveScores();
				resetGame();
				state = State::MAIN_MENU;
				return;
			}
		}
	}


	// Has the marine player touched time warp?
	if (marine.getPosition().intersects(wp.getPosition()))
	{
		// Play warping sound
		soundManager2.playWarp();

		// Reload the level
		m_NewLevelRequired = true;

		// Alter tiles for next level
		setTileSheets(lm.getCurrentLevel());

		// Load the subsequent level
		lm.setCurrentLevel(lm.getCurrentLevel());

		if (lm.getCurrentLevel() > 3)
		{
			// The game is prepared for finishing
			m_HasReturned = true;
		}

		// DO NOT REMOVE THIS BOOL AS IT ALLOWS STORY TEXT TO UPDATE PROPERLY!
		testBool = true;
		state = State::STORY_MENU;
	}
}