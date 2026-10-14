#include "MovementInputFromOptions.h"

#include "GameSettings.h"
#include "KeyBinding.h"

MovementInputFromOptions::MovementInputFromOptions(GameSettings *gamesettings)
{
	for (int i = 0; i < 10; i++)
		movementKeyStates[i] = false;
	gameSettings = gamesettings;
}

void MovementInputFromOptions::checkKeyForMovementInput(int i, bool flag)
{
	signed char byte0 = -1;
	if (i == gameSettings->keyBindForward->keyCode)
		byte0 = 0;
	if (i == gameSettings->keyBindBack->keyCode)
		byte0 = 1;
	if (i == gameSettings->keyBindLeft->keyCode)
		byte0 = 2;
	if (i == gameSettings->keyBindRight->keyCode)
		byte0 = 3;
	if (i == gameSettings->keyBindJump->keyCode)
		byte0 = 4;
	if (i == gameSettings->keyBindSneak->keyCode)
		byte0 = 5;
	if (byte0 >= 0)
		movementKeyStates[byte0] = flag;
}

void MovementInputFromOptions::resetKeyState()
{
	for (int i = 0; i < 10; i++)
		movementKeyStates[i] = false;
}

void MovementInputFromOptions::updatePlayerMoveState(EntityPlayer *entityplayer)
{
	moveStrafe = 0.0f;
	moveForward = 0.0f;
	if (movementKeyStates[0])
		moveForward++;
	if (movementKeyStates[1])
		moveForward--;
	if (movementKeyStates[2])
		moveStrafe++;
	if (movementKeyStates[3])
		moveStrafe--;
	jump = movementKeyStates[4];
	sneak = movementKeyStates[5];
	if (sneak)
	{
		moveStrafe *= 0.29999999999999999;
		moveForward *= 0.29999999999999999;
	}
}
