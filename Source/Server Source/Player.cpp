#include "Player.h"

Player::Player(float posx, float posz, float movefactor, int flag)
{
	PosX = posx;
	PosZ = posz;
	MoveFactor = movefactor;
	Flag = flag;
	Health = 4;
	SpeedTime = 0.0f;
	InvulnerabilityTime = 0.0f;
	LightningTime = 0.0f;
	Bombs = 0;
	Surges = 0;
	NewDestination = false;
	Connected = false;
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