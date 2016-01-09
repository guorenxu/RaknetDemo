#ifndef __PLAYER_H
#define __PLAYER_H

class Player
{
	public:

		float PosX;
		float PosZ;

		float NetPosX;											//Client-Only position storages for server-client checking
		float NetPosZ;

		float DestinationX;										//Current Destination if any
		float DestinationZ;

		bool NewDestination;

		int Health;												//Current Health
		float SpeedTime;										//Time remaining for powerups
		float InvulnerabilityTime;
		float LightningTime;
		int Bombs;												//Number of items
		int Surges;

		float MoveFactor;

		int Flag;												//Which Player are you?

		Player(float posx, float posz, float movefactor);
		~Player(){};

		void ChangePosition(float x, float z);					//Offsets the position by parameters
		void SetPosition(float x, float z);						//Changes position to parameters
};

#endif