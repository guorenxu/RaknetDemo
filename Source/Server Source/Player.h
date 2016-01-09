#ifndef __PLAYER_H
#define __PLAYER_H

class Player
{
	public:

		float PosX;
		float PosZ;
		float MoveFactor;

		float DestinationX;
		float DestinationZ;

		int Health;												//Current Health
		float SpeedTime;										//Time remaining for powerups
		float InvulnerabilityTime;
		float LightningTime;
		int Bombs;												//Number of items
		int Surges;

		bool NewDestination;
		bool Connected;

		int Flag;

		Player(float posx, float posz, float movefactor, int flag);
		~Player(){};

		void ChangePosition(float x, float z);					//Offsets the position by parameters
		void SetPosition(float x, float z);						//Changes position to parameters
};

#endif