#include "Network.h"

enum GameMessages
{
	//Client message describing where a move command was made to
	CID_MOVE_MESSAGE = ID_USER_PACKET_ENUM + 1,

	//Server message authenticating a move command; Answer to ID_MOVE_MESSAGE
	SID_MOVE_AUTH = ID_USER_PACKET_ENUM + 2,

	//Server message with the data of another player's move command
	SID_MOVE_OTHER = ID_USER_PACKET_ENUM + 3,

	//Client message requesting data of all players
	CID_POS_REQUEST = ID_USER_PACKET_ENUM + 4,

	//Server messsage initiating position of all players; Answer to ID_POS_REQUEST
	SID_POS_INITIATE = ID_USER_PACKET_ENUM + 5,

	//Server message initiating position of another player
	SID_POS_OTHER = ID_USER_PACKET_ENUM + 6,

	//Server message with regular updates of all players
	SID_POS_UPDATE = ID_USER_PACKET_ENUM + 7,

	//Client message requesting disconnection
	CID_DISCONNECT = ID_USER_PACKET_ENUM + 8,

	//Server message to other nondisconnected players
	SID_DISCONNECT_OTHER = ID_USER_PACKET_ENUM + 9,

	//Server message telling players that they lost or gained health
	SID_HEALTH_CHANGE = ID_USER_PACKET_ENUM + 10,

	//Server message telling players that someone else lost or gained health
	SID_HEALTH_CHANGE_OTHER = ID_USER_PACKET_ENUM + 11,

	//Debug message for testing
	DID_DEBUG = ID_USER_PACKET_ENUM + 100
};

unsigned long Start = timeGetTime();
unsigned long Total = timeGetTime();
long TimeDelta;

const int ClientPort = 10005;
const int ServerPort = 10006;
const int MaxClients = 500;
RakNet::Packet* Packet;
RakNet::RakPeerInterface *NetDevice;

RakNet::SystemAddress PlayerAddresses[4];
RakNet::SystemAddress EmptyAddress;

Player Players[4] = {Player(100.0f, 100.0f, 0.2f, 0), 
					Player(100.0f, 200.0f, 0.2f, 1), 
					Player(200.0f, 100.0f, 0.2f, 2), 
					Player(200.0f, 200.0f, 0.2f, 3)};


Player resetplayers[4] = {Player(100.0f, 100.0f, 0.2f, 0), 
					Player(100.0f, 200.0f, 0.2f, 1), 
					Player(200.0f, 100.0f, 0.2f, 2), 
					Player(200.0f, 200.0f, 0.2f, 3)};

Delta UpdateTimer(100);											//How often do you send update information to players

Delta HealthLossTimers[4] = {Delta(3000),						//How often can you lose health
							Delta(3000), 
							Delta(3000), 
							Delta(3000)};

Circle TestBall(50.0f, 200.0f, 200.0f);

void main()
{
	for (int i = 0; i < 4; i++)
	{
		PlayerAddresses[i].port = 0;
	}

	NetDevice = RakNet::RakPeerInterface::GetInstance();

	NetDevice->Startup(MaxClients, &RakNet::SocketDescriptor(ServerPort, 0), 1);

	Print("Starting the server...");
	Skip();
	NetDevice->SetMaximumIncomingConnections(MaxClients);

	while(1)
	{
			TimeDelta = timeGetTime() - Start;

			Start = timeGetTime();

			for (int i = 0; i < 1000; i++)
			{
				std::cout << "";
			}

			Timers();

			Network();

			Game();
	}

	RakNet::RakPeerInterface::DestroyInstance(NetDevice);
}

void Timers()
{
	UpdateTimer.Update(TimeDelta);				//Lower timer times by time passed

	for (int i = 0; i < 4; i++)
	{
		HealthLossTimers[i].UpdateZero(TimeDelta);
	}
}

void Network()
{
	if (UpdateTimer.Check() <= 0 && UpdateTimer.Active == true)
	{
		UpdateTimer.Reset(1);

		int numconnected = 0;	//Number of connected players

		//Find out the number of players connected

		for (int i = 0; i < 4; i++)
		{
			if (Players[i].Connected)
			{
				numconnected++;
			}
		}

		for (int i = 0; i < 4; i++)
		{
			if (Players[i].Connected)
			{
				RakNet::BitStream BSOut;
				BSOut.Write((RakNet::MessageID)SID_POS_UPDATE);

				//Send number of connected players

				BSOut.Write((int)numconnected);

				//Send own player's information

				BSOut.Write((float)Players[i].PosX);
				BSOut.Write((float)Players[i].PosZ);
				BSOut.Write((int)Players[i].Health);

				//Send other players' information

				for (int j = 0; j < 4; j++)
				{
					//If we haven't already sent this player's information and if he/she is connected..

					if (j != i)
					{
						//Is the player connected?

						if (Players[j].Connected)
						{
							//Flag for client identification of other players

							BSOut.Write((int)Players[j].Flag);
							BSOut.Write((float)Players[j].PosX);
							BSOut.Write((float)Players[j].PosZ);
						}
					}
				}

				NetDevice->Send(&BSOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, PlayerAddresses[i], false);
			}
		}
	}

	for (Packet = NetDevice->Receive(); Packet; NetDevice->DeallocatePacket(Packet), Packet = NetDevice->Receive())
		{
			switch (Packet->data[0])
				{
				case CID_MOVE_MESSAGE :
					{
						for (int i = 0; i < 4; i++)
						{
							//Which player sent the message? Is he/she still connected?

							if (Packet->systemAddress == PlayerAddresses[i] && Players[i].Connected)
							{
								//Read messages into server player classes

								RakNet::BitStream BSIn(Packet->data, Packet->length, false);
								BSIn.IgnoreBytes(sizeof(RakNet::MessageID));
								BSIn.Read(Players[i].DestinationX);
								BSIn.Read(Players[i].DestinationZ);

								std::cout << "Player " << i <<  " ordered a move to (" << Players[i].DestinationX << " , " << Players[i].DestinationZ << ")";
								Skip();

								//Send authentication packet to player who sent message

								RakNet::BitStream BSOut;
								BSOut.Write((RakNet::MessageID)SID_MOVE_AUTH);
								BSOut.Write((float)Players[i].DestinationX);
								BSOut.Write((float)Players[i].DestinationZ);
								NetDevice->Send(&BSOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, Packet->systemAddress, false);

								Players[i].NewDestination = true;

								//Send move messages to all other online players

								for (int j = 0; j < 4; j++)
								{
									if (j != i)
									{
										//Is the player connected?

										if (Players[j].Connected)
										{
											RakNet::BitStream BSOutOther;
											BSOutOther.Write((RakNet::MessageID)SID_MOVE_OTHER);
											BSOutOther.Write((int)Players[i].Flag);
											BSOutOther.Write((float)Players[i].DestinationX);
											BSOutOther.Write((float)Players[i].DestinationZ);
											NetDevice->Send(&BSOutOther, HIGH_PRIORITY, RELIABLE_ORDERED, 0, PlayerAddresses[j], false);
										}
									}
								}

								break;
							}
						}
					}
					break;
				case CID_POS_REQUEST :
					{	
						int numconnected = 0;	//Number of connected players

						//Find out the number of players connected

						for (int i = 0; i < 4; i++)
						{
							if (Players[i].Connected)
							{
								numconnected++;
							}
						}

						for (int i = 0; i < 4; i++)
						{
							//Who sent the packet? Is he/she still connected?

							if (Packet->systemAddress == PlayerAddresses[i] && Players[i].Connected)
							{
								std::cout << "Player " << i << " has requested an update.";
								Skip();

								//Send current player situation

								RakNet::BitStream BSOut;
								BSOut.Write((RakNet::MessageID)SID_POS_INITIATE);
								BSOut.Write((float)Players[i].PosX);
								BSOut.Write((float)Players[i].PosZ);
								BSOut.Write((float)Players[i].MoveFactor);
								BSOut.Write((int)Players[i].Health);

								//Send the flag

								BSOut.Write((int)(i));

								//Send the number of connected players

								BSOut.Write((int)numconnected);

								for (int j = 0; j < 4; j++)
								{
									//If we haven't already sent this player's information and if he/she is connected..

									if (j != i)
									{
										//Is the player connected?

										if (Players[j].Connected)
										{
											//Flag for client identification of other players

											BSOut.Write((int)Players[j].Flag);
											BSOut.Write((float)Players[j].PosX);
											BSOut.Write((float)Players[j].PosZ);
											BSOut.Write((float)Players[j].MoveFactor);
										}
									}
								}

								NetDevice->Send(&BSOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, Packet->systemAddress, false);

								//Send initiation messages to all other online players

								for (int j = 0; j < 4; j++)
								{
									if (j != i)
									{
										//Is the player connected?

										if (Players[j].Connected)
										{
											RakNet::BitStream BSOutOther;
											BSOutOther.Write((RakNet::MessageID)SID_POS_OTHER);
											BSOutOther.Write((int)Players[i].Flag);
											BSOutOther.Write((float)Players[i].PosX);
											BSOutOther.Write((float)Players[i].PosZ);
											BSOutOther.Write((float)Players[i].MoveFactor);
											NetDevice->Send(&BSOutOther, HIGH_PRIORITY, RELIABLE_ORDERED, 0, PlayerAddresses[j], false);
										}
									}
								}

								break;
							}
						}
					}
					break;
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					Print("Another client has disconnected.");
					Skip();
					break;
				case ID_REMOTE_CONNECTION_LOST:
					Print("Another client has lost the connection.");
					Skip();
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION:
					Print("Another client has connected.");
					Skip();
					break;				
				case ID_NEW_INCOMING_CONNECTION:
					Print("A connection is incoming.");
					for (int i = 0; i < 4; i++)
					{
						//Allocate Player IP

						if (PlayerAddresses[i].port == 0)
						{
							PlayerAddresses[i] = Packet->systemAddress;

							Players[i].Connected = true;

							//Begin appropriate timer

							for (int i = 0; i < 4; i++)
							{
								HealthLossTimers[i].Activate();
							}

							break;
						}
					}

					//Begin the timer(s)

					UpdateTimer.Activate();

					if (UpdateTimer.Timer == 0)			//Check if reset is nessasary (Only for initiation)
					{
						UpdateTimer.Reset(0);
					}

					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					Print("The server is full.");
					Skip();
					break;
				case CID_DISCONNECT:
					for (int i = 0; i < 4; i++)
					{
						//Which player sent the message?

						if (Packet->systemAddress == PlayerAddresses[i])
						{
							//Close the connection

							NetDevice->CloseConnection(PlayerAddresses[i], true, 1, IMMEDIATE_PRIORITY);

							//Reset the player's data

							Players[i] = resetplayers[i];

							//Reset the port and address to allow reconnection

							PlayerAddresses[i] = EmptyAddress;
							PlayerAddresses[i].port = 0;

							std::cout << "Player " << i << " " << "has disconnected manually.";
							Skip();

							//Send disconnect information to other players

							for (int j = 0; j < 4; j++)
							{
								if (j != i)
								{
									//Is the player connected?

									if (Players[j].Connected)
									{
										RakNet::BitStream BSOutOther;
										BSOutOther.Write((RakNet::MessageID)SID_DISCONNECT_OTHER);
										BSOutOther.Write((int)Players[i].Flag);
										NetDevice->Send(&BSOutOther, HIGH_PRIORITY, RELIABLE_ORDERED, 0, PlayerAddresses[j], false);
									}
								}
							}
						}
					}
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					for (int i = 0; i < 4; i++)
					{
						//Which player sent the message?

						if (Packet->systemAddress == PlayerAddresses[i])
						{
							//Reset the player's data

							Players[i] = resetplayers[i];

							//Reset the port and address to allow reconnection

							PlayerAddresses[i] = EmptyAddress;
							PlayerAddresses[i].port = 0;

							std::cout << "Player " << i << " " << "has disconnected.";
							Skip();
						}
					}
					break;
				case ID_CONNECTION_LOST:
					Print("A client lost the connection.");
					Skip();

					break;
				default:
					printf("Message with identifier %i has arrived.\n\n", Packet->data[0]);
					break;
				}
		}
}

void Game()
{
	for (int i = 0; i < 4; i++)
	{
		if (Players[i].NewDestination && Players[i].Connected)			//Does the player have a new destination
		{
			//Calculate distance

			D3DXVECTOR3 travel(Players[i].DestinationX - Players[i].PosX, 0, Players[i].DestinationZ - Players[i].PosZ);

			//Is the distance less than one move tick?

			if (sqrt((travel.x * travel.x) + (travel.z * travel.z)) < (Players[i].MoveFactor * float(TimeDelta)))
			{
				//Move to destination

				Players[i].PosX = Players[i].DestinationX;
				Players[i].PosZ = Players[i].DestinationZ;

				Players[i].NewDestination = false;
			}
			else
			{
				//Move based on how much time passed

				D3DXVec3Normalize(&travel, &travel);

				Players[i].PosX = Players[i].PosX + (travel.x * Players[i].MoveFactor * float(TimeDelta));
				Players[i].PosZ = Players[i].PosZ + (travel.z * Players[i].MoveFactor * float(TimeDelta));
			}
		}
	}

	//Check for collision with ball

	for (int i = 0; i < 4; i++)
	{
		//Is the player connected?

		if (Players[i].Connected)
		{
			//Did collision happen?

			if (CirclePlayerCollision(TestBall, Players[i]))
			{
				//Can the player lose health right now?

				if (HealthLossTimers[i].Check() <= 0 && HealthLossTimers[i].Active && Players[i].Health > 0)
				{
					//Reduce the player's health

					Players[i].Health = Players[i].Health - 1;

					std::cout << "Health lost by player " << i << std::endl;

					//Reset the health loss timer

					HealthLossTimers[i].Reset(1);

					//Send health loss message to the player who lost it

					RakNet::BitStream BSOut;
					BSOut.Write((RakNet::MessageID)SID_HEALTH_CHANGE);
					BSOut.Write((int)Players[i].Health);
					NetDevice->Send(&BSOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, PlayerAddresses[i], false);

					//Send health loss message to other players

					for (int j = 0; j < 4; j++)
					{
						if (j != i)
						{
							if (Players[j].Connected)
							{
								RakNet::BitStream BSOutOther;
								BSOutOther.Write((RakNet::MessageID)SID_HEALTH_CHANGE_OTHER);
								BSOutOther.Write((int)Players[i].Flag);
								NetDevice->Send(&BSOutOther, HIGH_PRIORITY, RELIABLE_ORDERED, 0, PlayerAddresses[j], false);
							}
						}
					}
				}
			}
		}
	}
}

bool CirclePlayerCollision(Circle circle, Player player)
{
	float xdiff;
	float zdiff;

	xdiff = circle.PosX - player.PosX;
	zdiff = circle.PosZ - player.PosZ;

	if (sqrt((xdiff * xdiff) + (zdiff * zdiff)) < circle.Radius)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Print(std::string str)
{
	std::cout << str << std::endl << std::endl;
}

void Skip()
{
	std::cout << std::endl << std::endl;
}