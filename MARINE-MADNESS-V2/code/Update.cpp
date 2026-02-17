#include "MarineMachine.h"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iostream>
#include "SoundManager.h"

using namespace std;

SoundManager soundManager2;
Time gameTimeTotal;

void MarineMachine::update(float dtAsSeconds)
{
	// Update particle positions for the main menu
	if (state == State::MAIN_MENU)
	{
		mainMenu.updateParticles(dtAsSeconds);
		soundManager2.playTrack0();
	}

	if (state == State::STORY_MENU)
	{
		// Stop any current tracks
		soundManager2.stopTracks();

		// Make text scroll
		scrollStoryText(dtAsSeconds);
	}

	if (state == State::PLAYING)
	{
		// Make a note of the players new position
		Vector2f playerPosition(marine.getCenter());

		// Update mouse positions
		mouseScreenPosition = Mouse::getPosition();
		mouseWorldPosition = m_Window.mapPixelToCoords(Mouse::getPosition(), m_MainView);

		// Set crosshair position
		spriteCrosshair.setPosition(mouseWorldPosition);

		// Update the player
		marine.update(dtAsSeconds, mouseWorldPosition);

		// Apply the centered view immediately
		m_MainView.setCenter(marine.getCenter());
		m_Window.setView(m_MainView); 

		// Update bullets
		for (int i = 0; i < 100; i++)
		{
			if (bullets[i].isInFlight())
			{
				bullets[i].update(dtAsSeconds);
			}
		}

		// Update enemy bullets
		for (int i = 0; i < 100; i++)
		{
			if (enemyBullets[i].isInFlight())
			{
				enemyBullets[i].update(dtAsSeconds);
			}
		}

		// Update enemies
		for (auto& enemy : enemies)
		{
			if (enemy->isAlive())
			{
				enemy->update(dtAsSeconds, playerPosition);

				// Handle enemy shooting only if the enemy type shoots
				if (enemy->isReadyToShoot())
				{
					for (int j = 0; j < 100; ++j)
					{
						if (!enemyBullets[j].isInFlight())
						{
							Vector2f center = enemy->getCenter();
							Vector2f target = marine.getCenter();

							enemyBullets[j].shoot(center.x, center.y, target.x, target.y);

							// Different sound based on type
							string typeName = typeid(*enemy).name();
							if (typeName.find("Cowboy") != string::npos)
							{
								soundManager2.playShoot();
							}
							else if (typeName.find("Android") != string::npos)
							{
								soundManager2.playLazer();
							}

							enemy->resetShootTimer();
							break;
						}
					}
				}
			}
		}

		// Update the dodge variables and reset the player
		if (isShifting && (gameTimeTotal - lastShiftTime >= shiftDuration)) 
		{
			isShifting = false;
			marine.setSpeed(originalSpeed);
		}

		// Check for collision
		detectCollision();

		// Update text before loading level
		if (state == State::STORY_MENU && testBool)
		{
			// Reset position of text
			setStoryTextPosition();

			// Change story text
			setLevelText(lm.getCurrentLevel());
			testBool = false;
		}

		// Start of a new level
		if (m_NewLevelRequired)
		{
			loadLevel();
		}
	}
}