#include "Player.h"

Player::Player(float posx, float posz, float movefactor)
{
	PosX = posx;
	PosZ = posz;
	MoveFactor = movefactor;

	NetPosX = posx;
	NetPosZ = posz;

	NewDestination = 0;

	DestinationX = -1.0f;
	DestinationZ = -1.0f;

	Health = 0;
	SpeedTime = 0.0f;
	InvulnerabilityTime = 0.0f;
	LightningTime = 0.0f;
	Bombs = 0;
	Surges = 0;
}

void Player::ChangePosition(float x, float z)
{
	PosX = PosX + x;
	PosZ = PosZ + z;
}

void Player::SetPosition(float x, float z)
{
	PosX = x;
	PosZ = z;
}