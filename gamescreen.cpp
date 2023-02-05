#include "gamescreen.h"

#define BG_Z 1
#define EXIT_Z 10
#define ROOT_Z 20
#define SCORE_TEXT_Z 40
#define LEVEL_CLEAR_Z 50

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	MugenSounds mSounds;
} gGameScreenData;

enum class Direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

void levelClearAnimationOverCB(void*);
void levelClearFadeOutOverCB(void*);
void natureDiedAnimationOverCB(void*);
void natureDiedFadeOutOverCB(void*);
void titleFadeOutOverCB(void*);
void levelStartAnimationCB(void*);
void quoteOverFadeOutCB(void*);

class GameScreen
{
public:

	int currentScreen = 0; // 0 = Title, 1 = game, 2 = quote

	int quoteBGEntity;
	int quoteText[3];
	Duration quoteDurationLeft = 60;

	int titleScreenBGEntity;
	int titleScreenLogoEntity;
	int titleScreenStartEntity[2];
	int titleScreenHighScoreText;
	int highScore = 0;
	int titleIsFadingOut = 0;

	int bgEntity;
	int rootEntities[24][32];
	int exitEntity;
	int scoreText;
	int scoreValue = 0;
	bool isActive = false;

	Vector2D gridOffset = Vector2D(16, 12);
	Vector2DI currentTile = Vector2DI(0, 0);
	Direction enterDirection = Direction::RIGHT;
	Direction exitDirection = Direction::RIGHT;
	Vector2DI exitTile = Vector2DI(30, 20);

	GameScreen() {
		gGameScreenData.mSprites = loadMugenSpriteFileWithoutPalette("game/GAME.sff");
		gGameScreenData.mAnimations = loadMugenAnimationFile("game/GAME.air");
		gGameScreenData.mSounds = loadMugenSoundFile("game/GAME.snd");
		
		quoteBGEntity = addBlitzEntity(Vector3D(0, 0, BG_Z));
		addBlitzMugenAnimationComponent(quoteBGEntity, &gGameScreenData.mSprites, &gGameScreenData.mAnimations, 2000);

		quoteText[0] = addMugenTextMugenStyle("", Vector3D(40, 100, SCORE_TEXT_Z), Vector3DI(1, 0, 1));
		setMugenTextTextBoxWidth(quoteText[0], 240);
		quoteText[1] = addMugenTextMugenStyle("", Vector3D(20, 140, SCORE_TEXT_Z), Vector3DI(1, 0, 1));
		setMugenTextTextBoxWidth(quoteText[1], 260);
		quoteText[2] = addMugenTextMugenStyle("", Vector3D(310, 230, SCORE_TEXT_Z), Vector3DI(-1, 7, -1));

		titleScreenBGEntity = addBlitzEntity(Vector3D(0, 0, BG_Z));
		addBlitzMugenAnimationComponent(titleScreenBGEntity, &gGameScreenData.mSprites, &gGameScreenData.mAnimations, 1000);

		titleScreenLogoEntity = addBlitzEntity(Vector3D(160, 110, SCORE_TEXT_Z));
		addBlitzMugenAnimationComponent(titleScreenLogoEntity, &gGameScreenData.mSprites, &gGameScreenData.mAnimations, 1010);

		for (int i = 0; i < 2; i++)
		{
			titleScreenStartEntity[i] = addBlitzEntity(Vector3D(160 + i * 380, 224, SCORE_TEXT_Z));
			addBlitzMugenAnimationComponent(titleScreenStartEntity[i], &gGameScreenData.mSprites, &gGameScreenData.mAnimations, 1020);
		}

		titleScreenHighScoreText = addMugenTextMugenStyle("High Score: 000", Vector3D(160, 200, SCORE_TEXT_Z), Vector3DI(1, 0, 0));
		setMugenTextVisibility(titleScreenHighScoreText, false);

		bgEntity = addBlitzEntity(Vector3D(0,0, BG_Z));
		addBlitzMugenAnimationComponent(bgEntity, &gGameScreenData.mSprites, &gGameScreenData.mAnimations, 1);

		exitEntity = addBlitzEntity(Vector3D(exitTile.x * 9, exitTile.y * 9, EXIT_Z) + gridOffset);
		addBlitzMugenAnimationComponent(exitEntity, &gGameScreenData.mSprites, &gGameScreenData.mAnimations, 100);

		scoreText = addMugenTextMugenStyle("Score: 000", Vector3D(gridOffset.x, gridOffset.y - 1, SCORE_TEXT_Z), Vector3DI(1, 0, 1));

		for (int j = 0; j < 24; j++)
		{
		
			for (int i = 0; i < 32; i++)
			{
				rootEntities[j][i] = -1;
			}
		}

		currentTile = Vector2DI(0, 10);
		enterDirection = exitDirection = Direction::RIGHT;
		setTitleScreenActive();
	}

	void setQuoteScreenInactive()
	{
		setBlitzMugenAnimationVisibility(quoteBGEntity, false);
		for (int i = 0; i < 3; i++) setMugenTextVisibility(quoteText[i], false);
	}

	void setQuoteScreenActive()
	{
		setTitleScreenInactive();
		setGameScreenInactive();
		setBlitzMugenAnimationVisibility(quoteBGEntity, true);

		for (int i = 0; i < 3; i++) setMugenTextVisibility(quoteText[i], true);
		setNewQuote();
		currentScreen = 2;
		quoteDurationLeft = 180;
	}

	#define QUOTE_COUNT 20
	std::string quotes[QUOTE_COUNT * 3] = {
		"Roots... Below...",
		"Our origins...",

		"Here it comes",

		"Some search for roots.",
		"Some reject roots.",

		"This is going to hit the vague-quote-after-gameplay market",

		"Is a live without roots possible?",
		"Can we sever that connection?",

		"Deep, I tried it irl and the plant just died though",

		"They say the apple doesn't fall far from the tree...",
		"But what if it rolls away?",

		"Eat that Isaac Newton",

		"Are our roots just a part of nature we have to accept?",
		"Or should we use our human ingenuity to overcome them?",

		"Humans don't actually have roots, look down to check",

		"It's our source of energy.",
		"But then again, there are many sources.",

		"You should keep going yeah yeah and nodding with the deepness",

		"They lie deep. Dark.",
		"Do we really want to find them?",

		"Again with the deepness, I think the BAFTA is mine for this",

		"Isn't it good enough to know that roots are there?",
		"And if there aren't, would that affect us?",

		"Or the Game Awards maybe, I could call Geoff my little root man",

		"Are they even a good thing?",
		"Holding us down.",

		"The dying plant seemed rather unhappy without them",

		"But cutting them off feels blunt.",
		"It feels wrong.",

		"Because there's tons of dirt when you do it (not deep, fact)",

		"Maybe relocation?  Carefully...",
		"... Uprooting?",

		"Little wordplay, just what you expect from this game",

		"But some roots can only live in certain soil.",
		"However: Roots are roots. We are we.",

		"You should also make a face deep in thoughts while reading these",

		"Roots should be kept underground.",
		"Roots on the ground are just a trip hazard.",

		"Relatable",

		"But one day we might become roots.",
		"Could you be underground as the invisible provider?",

		"Going to invisible grocery stores, paying invisible rent",

		"Does selflessness like that exist?",
		"Do roots not serve their own purpose?",

		"Root uprising, the sequel shooter",

		"Maybe the separation between rootsand ourselves was wrong.",
		"In a way, aren't we all part of the same thing?",

		"I'm not a root myself personally",

		"But that is just arguing semantics.",
		"Ultimately there may not be a correct choice to make.",

		"Don't really know what semantics means but it sounds like a cool word",

		"The roots may beg to differ.",
		"We may beg to differ.",

		"I beg for game downloads mostly",

		"But such conflict is a part  of life.",
		"Avoiding it is nothing but a delusion.",

		"Just going aruond with a flamethrower torching roots to keep it real",

		"Maybe the conflicting sides  of this are the reason.",
		"Maybe they are the root cause.",

		"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoooooooooo",
	};

	void setNewQuote()
	{
		int currentStage = (scoreValue / 100) - 1;
		if (currentStage >= QUOTE_COUNT) return;

		changeMugenText(quoteText[0], quotes[currentStage * 3].c_str());
		setMugenTextBuildup(quoteText[0], 1);

		changeMugenText(quoteText[1], quotes[currentStage * 3 + 1].c_str());
		setMugenTextBuildup(quoteText[1], 1);

		changeMugenText(quoteText[2], quotes[currentStage * 3 + 2].c_str());
		setMugenTextBuildup(quoteText[2], 1);

		setMugenTextPosition(quoteText[0], Vector3D(40, 100, SCORE_TEXT_Z));
		setMugenTextPosition(quoteText[1], Vector3D(20, 140, SCORE_TEXT_Z));
		setMugenTextPosition(quoteText[2], Vector3D(310, 230, SCORE_TEXT_Z));
	}

	void setGameScreenInactive()
	{
		setBlitzMugenAnimationVisibility(bgEntity, false);
		setBlitzMugenAnimationVisibility(exitEntity, false);
		setMugenTextVisibility(scoreText, false);
		for (int j = 0; j < 24; j++)
		{
			for (int i = 0; i < 32; i++)
			{
				if (rootEntities[j][i] != -1)
				{
					removeBlitzEntity(rootEntities[j][i]);
					rootEntities[j][i] = -1;
				}
			}
		}
	}

	void setTitleScreenInactive()
	{
		setBlitzMugenAnimationVisibility(titleScreenBGEntity, false);
		setBlitzMugenAnimationVisibility(titleScreenLogoEntity, false);
		for (int i = 0; i < 2; i++)
		{
			setBlitzMugenAnimationVisibility(titleScreenStartEntity[i], false);
		}
		setMugenTextVisibility(titleScreenHighScoreText, false);
	}

	void setTitleScreenActive()
	{
		setQuoteScreenInactive();
		setGameScreenInactive();
		setBlitzMugenAnimationVisibility(titleScreenBGEntity, true);
		setBlitzMugenAnimationVisibility(titleScreenLogoEntity, true);
		for (int i = 0; i < 2; i++)
		{
			setBlitzMugenAnimationVisibility(titleScreenStartEntity[i], true);
		}
		if (highScore > 0)
		{
			std::stringstream ss;
			ss << "High Score: " << highScore;
			changeMugenText(titleScreenHighScoreText, ss.str().c_str());
			setMugenTextVisibility(titleScreenHighScoreText, true);
		}
		stopStreamingMusicFile();
		streamMusicFile("tracks/4.ogg");
		titleIsFadingOut = 0;
		currentScreen = 0;
	}

	void setGameScreenActive(int fullRestart = true)
	{
		setQuoteScreenInactive();
		setTitleScreenInactive();
		setBlitzMugenAnimationVisibility(bgEntity, true);
		setBlitzMugenAnimationVisibility(exitEntity, true);
		setMugenTextVisibility(scoreText, true);
		for (int j = 0; j < 24; j++)
		{
			for (int i = 0; i < 32; i++)
			{
				if (rootEntities[j][i] != -1)
				{
					removeBlitzEntity(rootEntities[j][i]);
					rootEntities[j][i] = -1;
				}
			}
		}
		currentScreen = 1;
		if (fullRestart)
		{
			stopStreamingMusicFile();
			streamMusicFile("tracks/3.ogg");
			startGame();
		}
	}

	void startGame()
	{
		resetGame();
	}

	void startGameplay()
	{
		isActive = true;
		startCurrentTile();
	}

	void resetScore()
	{
		scoreValue = 0;
		changeMugenText(scoreText, "Score: 000");
	}

	void increaseScore()
	{
		scoreValue += 100;
		std::stringstream ss;
		ss << "Score: " << scoreValue;
		changeMugenText(scoreText, ss.str().c_str());
	}

	void quoteOverToGameScreen()
	{
		setGameScreenActive(false);
		resetLevel();
		enableDrawing();
		addFadeIn(20, NULL, NULL);
	}

	void resetGame()
	{
		resetScore();
		resetLevel();
	}

	void resetLevel()
	{
		do
		{
			auto directionInteger = randfromInteger(0, 3);
			if (directionInteger == 0)
			{
				enterDirection = exitDirection = Direction::RIGHT;
				currentTile = Vector2DI(0, randfromInteger(0, 23));
			}
			else if (directionInteger == 1)
			{
				enterDirection = exitDirection = Direction::LEFT;
				currentTile = Vector2DI(31, randfromInteger(0, 23));

			}
			else if (directionInteger == 2)
			{
				enterDirection = exitDirection = Direction::DOWN;
				currentTile = Vector2DI(randfromInteger(0, 31), 0);
			}
			else
			{
				enterDirection = exitDirection = Direction::UP;
				currentTile = Vector2DI(randfromInteger(0, 31), 23);
			}

			do
			{
				exitTile = Vector2DI(randfromInteger(0, 31), randfromInteger(0, 23));
			} while (vecLength(currentTile - exitTile) < 5);
			setBlitzEntityPosition(exitEntity, Vector3D(exitTile.x * 9, exitTile.y * 9, EXIT_Z) + gridOffset);

			for (int j = 0; j < 24; j++)
			{
				for (int i = 0; i < 32; i++)
				{
					if (rootEntities[j][i] != -1)
					{
						removeBlitzEntity(rootEntities[j][i]);
						rootEntities[j][i] = -1;
					}
				}
			}

			generateObstacles();
		} while (!canLevelBeBeaten());

		isActive = false;

		auto levelStartAnimation = addMugenAnimation(getMugenAnimation(&gGameScreenData.mAnimations, 500), &gGameScreenData.mSprites, Vector3D(160, 120, LEVEL_CLEAR_Z));
		setMugenAnimationCallback(levelStartAnimation, levelStartAnimationCB, NULL);
		setMugenAnimationNoLoop(levelStartAnimation);
	}

	int canLevelBeBeaten()
	{
		return 1;
	}

	void generateObstacles()
	{
		for (int j = 0; j < 24; j++)
		{
			for (int i = 0; i < 32; i++)
			{
				int rando = randfromInteger(0, 100);
				auto tile = Vector2DI(i, j);
				if (rando < 10 && vecLength(tile - exitTile) > 5 && vecLength(tile - currentTile) > 5)
				{
					rootEntities[j][i] = addBlitzEntity(Vector3D(i * 9, j * 9, ROOT_Z) + gridOffset);
					addBlitzMugenAnimationComponent(rootEntities[j][i], &gGameScreenData.mSprites, &gGameScreenData.mAnimations, 300);
				}
			}
		}
	}

	void startCurrentTile()
	{
		if (currentTile.x < 0 || currentTile.x >= 32 || currentTile.y < 0 || currentTile.y >= 24)
		{
			startGameOver();
			return;
		}

		auto currentTileEntity = rootEntities[currentTile.y][currentTile.x];
		if (currentTileEntity != -1)
		{
			startGameOver();
			return;
		}

		enterDirection = exitDirection;
		int animationNo = 10;
		if (enterDirection == Direction::RIGHT)
		{
			animationNo = 10;
		} 
		else if (enterDirection == Direction::LEFT)
		{
			animationNo = 11;
		}
		else if (enterDirection == Direction::DOWN)
		{
			animationNo = 12;
		}
		else if (enterDirection == Direction::UP)
		{
			animationNo = 13;
		}

		rootEntities[currentTile.y][currentTile.x] = addBlitzEntity(Vector3D(currentTile.x * 9, currentTile.y * 9, ROOT_Z) + gridOffset);
		addBlitzMugenAnimationComponent(rootEntities[currentTile.y][currentTile.x], &gGameScreenData.mSprites, &gGameScreenData.mAnimations, animationNo);
	}

	void update()
	{
		if (currentScreen == 0)
		{
			updateTitleScreen();
		}
		else if (currentScreen == 1)
		{
			updateGame();
		}
		else
		{
			updateQuote();
		}
	}

	void updateQuote()
	{
		addMugenTextPosition(quoteText[0], Vector3D(0.1, 0, 0));
		addMugenTextPosition(quoteText[1], Vector3D(0.1, 0, 0));
		addMugenTextPosition(quoteText[2], Vector3D(-0.1, 0, 0));

		if (quoteDurationLeft <= 0) return;
		quoteDurationLeft--;

		if (quoteDurationLeft <= 0 || hasPressedStartFlank())
		{
			quoteDurationLeft = 0;
			addFadeOut(20, quoteOverFadeOutCB, NULL);
		}
		
	}

	void updateTitleScreen()
	{
		updateCreditsMove();
		updateTitleInput();
	}

	void updateCreditsMove()
	{
		for (int i = 0; i < 2; i++)
		{
			addBlitzEntityPositionX(titleScreenStartEntity[i], -1);
		}
		if (getBlitzEntityPositionX(titleScreenStartEntity[0]) < -(380 / 2))
		{
			addBlitzEntityPositionX(titleScreenStartEntity[0], 380 * 2);
			swap(titleScreenStartEntity[0], titleScreenStartEntity[1]);
		}
	}

	void updateTitleInput()
	{
		if (titleIsFadingOut) return;

		if (hasPressedAFlank() || hasPressedStartFlank())
		{
			titleIsFadingOut = 1;
			tryPlayMugenSound(&gGameScreenData.mSounds, 1, 0);
			addFadeOut(20, titleFadeOutOverCB, NULL);
		}
	}

	void titleFadeOutOver()
	{
		setGameScreenActive();
		enableDrawing();
		addFadeIn(20, NULL, NULL);
	}

	void updateGame()
	{
		if (isActive)
		{
			updateInputBuffering();
			updateRootMovement();
		}
		else
		{
			releaseInputBuffering();
		}
	}

	int hasPressedRightBuffered = 0;
	int hasPressedLeftBuffered = 0;
	int hasPressedDownBuffered = 0;
	int hasPressedUpBuffered = 0;
	void updateInputBuffering()
	{
		if (hasPressedRight())  hasPressedRightBuffered = 1;
		if (hasPressedLeft())  hasPressedLeftBuffered = 1;
		if (hasPressedDown())  hasPressedDownBuffered = 1;
		if (hasPressedUp())  hasPressedUpBuffered = 1;

		if (hasPressedLeftFlank() || hasPressedRightFlank() || hasPressedDownFlank() || hasPressedUpFlank())
		{
			tryPlayMugenSound(&gGameScreenData.mSounds, 1, 3);
		}

		if (hasPressedStartFlank() && false)
		{
			startLevelClear();
		}
	}

	void releaseInputBuffering()
	{
		hasPressedRightBuffered = hasPressedLeftBuffered = hasPressedDownBuffered = hasPressedUpBuffered = 0;
	}

	void updateRootMovement()
	{
		auto currentTileEntity = rootEntities[currentTile.y][currentTile.x];
		if (currentTileEntity == -1) return;
		auto animationNo = getBlitzMugenAnimationAnimationNumber(currentTileEntity);
		auto frame = getBlitzMugenAnimationAnimationStep(currentTileEntity);
		if (animationNo < 20)
		{
			if (frame == getBlitzMugenAnimationAnimationStepAmount(currentTileEntity) - 1)
			{
				switchToFullAnimation();
			}
		}
		else
		{
			if (frame == 4)
			{
				goToNextTile();
			}
		}
	}

	void goToNextTile()
	{
		if (exitDirection == Direction::RIGHT)
		{
			currentTile.x += 1;
		}
		else if (exitDirection == Direction::LEFT)
		{
			currentTile.x -= 1;
		}
		else if (exitDirection == Direction::DOWN)
		{
			currentTile.y += 1;
		}
		else if (exitDirection == Direction::UP)
		{
			currentTile.y -= 1;
		}

		startCurrentTile();
	}


	void switchToFullAnimation()
	{
		if (currentTile == exitTile)
		{
			startLevelClear();
			return;
		}

		if (hasPressedDownBuffered && enterDirection != Direction::UP && enterDirection != Direction::DOWN)
		{
			exitDirection = Direction::DOWN;
		}
		else if (hasPressedUpBuffered && enterDirection != Direction::DOWN && enterDirection != Direction::UP)
		{
			exitDirection = Direction::UP;
		}
		else if (hasPressedLeftBuffered && enterDirection != Direction::RIGHT && enterDirection != Direction::LEFT)
		{
			exitDirection = Direction::LEFT;
		}
		else if (hasPressedRightBuffered && enterDirection != Direction::LEFT && enterDirection != Direction::RIGHT)
		{
			exitDirection = Direction::RIGHT;
		}
		releaseInputBuffering();

		if (enterDirection == Direction::RIGHT)
		{
			if (exitDirection == Direction::DOWN)
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 24);
			}
			else if (exitDirection == Direction::UP)
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 25);
			}
			else
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 20);
			}
		}
		else if (enterDirection == Direction::LEFT)
		{
			if (exitDirection == Direction::DOWN)
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 26);
			}
			else if (exitDirection == Direction::UP)
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 27);
			}
			else
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 21);
			}
		}
		else if (enterDirection == Direction::DOWN)
		{
			if (exitDirection == Direction::RIGHT)
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 28);
			}
			else if (exitDirection == Direction::LEFT)
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 29);
			}
			else
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 22);
			}
		}
		else if (enterDirection == Direction::UP)
		{
			if (exitDirection == Direction::RIGHT)
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 30);
			}
			else if (exitDirection == Direction::LEFT)
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 31);
			}
			else
			{
				changeBlitzMugenAnimation(rootEntities[currentTile.y][currentTile.x], 23);
			}
		}
	}

	void startLevelClear()
	{
		tryPlayMugenSound(&gGameScreenData.mSounds, 1, 1);
		increaseScore();
		auto levelFinishAnimation = addMugenAnimation(getMugenAnimation(&gGameScreenData.mAnimations, 200), &gGameScreenData.mSprites, Vector3D(160, 120, LEVEL_CLEAR_Z));
		setMugenAnimationCallback(levelFinishAnimation, levelClearAnimationOverCB, NULL);
		setMugenAnimationNoLoop(levelFinishAnimation);
		pauseRoots();
		isActive = false;
	}

	void levelClearAnimationOver()
	{
		addFadeOut(20, levelClearFadeOutOverCB, NULL);
	}

	void levelClearFadeOutOver()
	{
		if (scoreValue >= 2100)
		{
			resetLevel();
		}
		else
		{
			setQuoteScreenActive();
		}
		enableDrawing();
		addFadeIn(20, NULL, NULL);
	}

	int finalScoreText;
	void startGameOver()
	{
		stopStreamingMusicFile();
		tryPlayMugenSound(&gGameScreenData.mSounds, 1, 2);
		auto levelFinishAnimation = addMugenAnimation(getMugenAnimation(&gGameScreenData.mAnimations, 210), &gGameScreenData.mSprites, Vector3D(160, 120, LEVEL_CLEAR_Z));
		setMugenAnimationCallback(levelFinishAnimation, natureDiedAnimationOverCB, NULL);
		setMugenAnimationNoLoop(levelFinishAnimation);
		pauseRoots();

		std::stringstream ss;
		ss << "Final Score: " << scoreValue;
		finalScoreText = addMugenTextMugenStyle(ss.str().c_str(), Vector3D(160, 150, SCORE_TEXT_Z), Vector3DI(1, 0, 0));

		highScore = max(highScore, scoreValue);
		isActive = false;
	}

	void natureDiedAnimationOver()
	{
		removeMugenText(finalScoreText);
		addFadeOut(20, natureDiedFadeOutOverCB, NULL);
	}
	
	void natureDiedFadeOutOver()
	{
		setTitleScreenActive();
		enableDrawing();
		addFadeIn(20, NULL, NULL);
	}

	void pauseRoots()
	{
		for (int j = 0; j < 24; j++)
		{
			for (int i = 0; i < 32; i++)
			{
				if (rootEntities[j][i] != -1)
				{
					pauseBlitzMugenAnimation(rootEntities[j][i]);
				}
			}
		}
	}
};

EXPORT_SCREEN_CLASS(GameScreen);

void levelClearAnimationOverCB(void*)
{
	gGameScreen->levelClearAnimationOver();
}

void levelClearFadeOutOverCB(void*)
{
	gGameScreen->levelClearFadeOutOver();
}

void natureDiedAnimationOverCB(void*)
{
	gGameScreen->natureDiedAnimationOver();
}

void natureDiedFadeOutOverCB(void*)
{
	gGameScreen->natureDiedFadeOutOver();
}

void titleFadeOutOverCB(void*)
{
	gGameScreen->titleFadeOutOver();
}

void levelStartAnimationCB(void*)
{
	gGameScreen->startGameplay();
}

void quoteOverFadeOutCB(void*)
{
	gGameScreen->quoteOverToGameScreen();
}