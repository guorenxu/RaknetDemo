#include "Game.h"

#define SafeRelease(p)      { if(p) { (p)->Release(); (p)=NULL; } }

extern D3DPRESENT_PARAMETERS d3dpp;

float ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

int MouseChangeX;
int MouseChangeY;
bool RightClick;
bool LeftClick;
int MouseWheelDelta;
bool DPress;
bool WPress;
bool APress;
bool SPress;

bool Death = false;

HWND ghwnd;

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

struct Post_Vertex
{
	float x, y, z, w;
	float u, v;

	static const unsigned long FVF;

	Post_Vertex(float X, float Y, float U, float V)
	{
		x  = X;  y  = Y;  z  = 0.0f;  w  = 1.0f;
		u  = U;  v  = V;
	}
};

float PI = 3.14159265f;

const D3DXCOLOR      White( D3DCOLOR_XRGB(255, 255, 255) );
const D3DXCOLOR      Black( D3DCOLOR_XRGB(  0,   0,   0) );
const D3DXCOLOR        Red( D3DCOLOR_XRGB(255,   0,   0) );
const D3DXCOLOR      Green( D3DCOLOR_XRGB(  0, 255,   0) );
const D3DXCOLOR       Blue( D3DCOLOR_XRGB(  0,   0, 255) );
const D3DXCOLOR     Yellow( D3DCOLOR_XRGB(255, 255,   0) );
const D3DXCOLOR       Cyan( D3DCOLOR_XRGB(  0, 255, 255) );
const D3DXCOLOR    Magenta( D3DCOLOR_XRGB(255,   0, 255) );

long int Start = timeGetTime();
long int TimeDelta;
int ClickCooldown = 0;

bool DeviceLost = false;

LPD3DXSPRITE SpriteHandler;

LPDIRECT3DSURFACE9 MainSurface = NULL;

LPDIRECT3DTEXTURE9 Cursor = NULL;
LPDIRECT3DTEXTURE9 CursorDrag = NULL;
LPDIRECT3DTEXTURE9 Health = NULL;
LPDIRECT3DTEXTURE9 Hurt = NULL;

Player MyPlayer(-1.0f, -1.0f, 0.0f);
Player OtherPlayers[3] = {Player(-1.0f, -1.0f, 0.0f),
						Player(-1.0f, -1.0f, 0.0f),
						Player(-1.0f, -1.0f, 0.0f)};

Player EmptyPlayer = Player(-1.0f, -1.0f, 0.0f);

Delta MoveClickTimer(200);								//Timer on holding mouse for movement
Delta MyHurtTimer(2000);								//Timers for hurt graphics
Delta OtherHurtTimers[3] = {Delta(2000), 
	Delta(2000), 
	Delta(2000)};

Circle TestBall(50.0f, 200.0f, 200.0f);

bool NewDestination = false;							//Lock on movement in case if players moved without command (for gameplay purposes)
float DestinationX = 100.0f;							//Movement destinations
float DestinationZ = 100.0f;

const int ClientPort = 10005;
const int ServerPort = 10006;
const int MaxClients = 500;
RakNet::RakPeerInterface *NetDevice;

RakNet::SystemAddress ServerAddress;

RakNet::RakString ServerIP = "127.0.0.1";
bool GreenLight = false;
RakNet::Packet* Packet;
		
bool SendMove = false;									//Controls sending
float NetDestinationX;									//Destinations to send to server
float NetDestinationZ;

bool CheckFrame = false;								//If the server sent an update, check only for this frame is synced

bool NetLock = true;									//Until connection established don't allow sending

D3DXMATRIX Proj;

D3DXMATRIX View;

int Game_Init(HWND hwnd)
{
	ghwnd = hwnd;

	HRESULT result;

	result = D3DXCreateSprite(d3ddev, &SpriteHandler);			//Create Sprite Handler

	Cursor = LoadTexture("Cursor.tga", White);
	CursorDrag = LoadTexture("Cursor Drag.tga", White);
	Health = LoadTexture("Health.tga", White);
	TestBall.Texture = LoadTexture("Sphere.tga", White);
	Hurt = LoadTexture("Hurt.tga", White);

	float ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	Proj = SetPerspective(1.0f, ratio, 1.0f, 10000.0f);			//REMEMBER FAR RANGE

	Init_PP();

	NetDevice = RakNet::RakPeerInterface::GetInstance();

	NetDevice->Startup(1, &RakNet::SocketDescriptor(), 1);
	
	NetDevice->Connect(ServerIP.C_String(), ServerPort, 0, 0);

	//Activate and initiate timers

	MoveClickTimer.Activate();
	MoveClickTimer.Reset(0);

	MyHurtTimer.Activate();

	for (int i = 0; i < 3; i++)
	{
		OtherHurtTimers[i].Activate();
	}

	return 1;
}

void Init_PP()
{
}

void Game_Run(HWND hwnd)
{	
	if (d3ddev == NULL)
	{
		return;
	}

	TimeDelta = timeGetTime() - Start;

	Start = timeGetTime();

	Game_Loop(hwnd);

	if (Death)
	{
		PostMessage(hwnd, WM_DESTROY, 0, 0);
		Game_End(hwnd);
	}

}

void Game_Loop(HWND hwnd)
{
	if (CheckLostDevices() == false)
	{
		return;
	}

	Timers();

	Network();

	Set_Up();

	Game_Calc();

	Mouse();

	Interface();

	Present();
}

void Timers()
{
	//Update timers' times by time passed every frame

	MoveClickTimer.Update(TimeDelta);

	MyHurtTimer.UpdateZero(TimeDelta);

	for (int i = 0; i < 3; i++)
	{
		OtherHurtTimers[i].UpdateZero(TimeDelta);
	}
}

void Network()
{
	if (SendMove == true)
	{
		SendMove = false;
		RakNet::BitStream BSOut;
		BSOut.Write((RakNet::MessageID)CID_MOVE_MESSAGE);
		BSOut.Write((float)NetDestinationX);
		BSOut.Write((float)NetDestinationZ);
		NetDevice->Send(&BSOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, ServerAddress, false);
	}

	for (Packet = NetDevice->Receive(); Packet; NetDevice->DeallocatePacket(Packet), Packet = NetDevice->Receive())
	{
		switch (Packet->data[0])
		{
			case SID_MOVE_AUTH :
				{
					//Receive authenticated destinations

					RakNet::BitStream BSIn(Packet->data, Packet->length, false);
					BSIn.IgnoreBytes(sizeof(RakNet::MessageID));
					BSIn.Read(DestinationX);
					BSIn.Read(DestinationZ);
					NewDestination = true;
				}
				break;
			case SID_POS_INITIATE :
				{
					//Receive initial situation

					RakNet::BitStream BSIn(Packet->data, Packet->length, false);
					BSIn.IgnoreBytes(sizeof(RakNet::MessageID));
					BSIn.Read(MyPlayer.PosX);
					BSIn.Read(MyPlayer.PosZ);
					BSIn.Read(MyPlayer.MoveFactor);
					BSIn.Read(MyPlayer.Health);

					//Receive my player's flag in order to establish the other players' flags

					BSIn.Read(MyPlayer.Flag);

					//Set up the other players' flags

					int current = 0;

					for (int i = 0; i < 4; i++)
					{
						if (MyPlayer.Flag != i)
						{
							OtherPlayers[current].Flag = i;

							current++;
						}
					}

					//Unlock movement!

					NetLock = false;

					int numconnected;

					//Receive number of players connected

					BSIn.Read(numconnected);

					//Receive other players' information

					for (int i = 0; i < numconnected; i++)
					{
						int flag;

						//Receive flag of other player

						BSIn.Read((int)flag);

						for (int j = 0; j < 3; j++)
						{
							//Compare flags and write to correct player's data

							if (OtherPlayers[j].Flag == flag)
							{
								BSIn.Read((float)OtherPlayers[j].PosX);
								BSIn.Read((float)OtherPlayers[j].PosZ);
								BSIn.Read((float)OtherPlayers[j].MoveFactor);
							}
						}
					}
				}
				break;
			case SID_POS_UPDATE :
				{
					//Number of players connected

					int numconnected;

					RakNet::BitStream BSIn(Packet->data, Packet->length, false);
					BSIn.IgnoreBytes(sizeof(RakNet::MessageID));

					//Receive number of players connected

					BSIn.Read(numconnected);

					//Receive my player's information

					BSIn.Read(MyPlayer.NetPosX);
					BSIn.Read(MyPlayer.NetPosZ);
					BSIn.Read(MyPlayer.Health);

					//Receive other players' information

					for (int i = 0; i < numconnected; i++)
					{
						int flag;

						//Receive flag of other player

						BSIn.Read((int)flag);

						for (int j = 0; j < 3; j++)
						{
							//Compare flags and write to correct player's data

							if (OtherPlayers[j].Flag == flag)
							{
								BSIn.Read((float)OtherPlayers[j].NetPosX);
								BSIn.Read((float)OtherPlayers[j].NetPosZ);
							}
						}
					}

					CheckFrame = true;
				}
				break;
			case SID_POS_OTHER :
				{
					RakNet::BitStream BSIn(Packet->data, Packet->length, false);
					BSIn.IgnoreBytes(sizeof(RakNet::MessageID));

					int flag;

					//Receive flag of other player

					BSIn.Read((int)flag);

					for (int j = 0; j < 3; j++)
					{
						//Compare flags and write to correct player's data

						if (OtherPlayers[j].Flag == flag)
						{
							BSIn.Read((float)OtherPlayers[j].PosX);
							BSIn.Read((float)OtherPlayers[j].PosZ);
							BSIn.Read((float)OtherPlayers[j].MoveFactor);
						}
					}
				}
				break;
			case SID_MOVE_OTHER :
				{
					RakNet::BitStream BSIn(Packet->data, Packet->length, false);
					BSIn.IgnoreBytes(sizeof(RakNet::MessageID));

					int flag;

					//Receive flag of other player

					BSIn.Read((int)flag);

					for (int j = 0; j < 3; j++)
					{
						//Compare flags and write to correct player's data

						if (OtherPlayers[j].Flag == flag)
						{
							BSIn.Read((float)OtherPlayers[j].DestinationX);
							BSIn.Read((float)OtherPlayers[j].DestinationZ);

							OtherPlayers[j].NewDestination = true;
						}
					}
				}
				break;
			case SID_DISCONNECT_OTHER :
				{
					RakNet::BitStream BSIn(Packet->data, Packet->length, false);
					BSIn.IgnoreBytes(sizeof(RakNet::MessageID));

					int flag;

					//Receive flag of other player

					BSIn.Read((int)flag);

					for (int j = 0; j < 3; j++)
					{
						//Compare flags and write to correct player's data

						if (OtherPlayers[j].Flag == flag)
						{
							OtherPlayers[j] = EmptyPlayer;
						}
					}
				}
				break;
			case SID_HEALTH_CHANGE :
				{
					//Receive new health amount

					RakNet::BitStream BSIn(Packet->data, Packet->length, false);
					BSIn.IgnoreBytes(sizeof(RakNet::MessageID));
					BSIn.Read(MyPlayer.Health);

					//Change graphics to hurt

					MyHurtTimer.Reset(0);
				}
				break;
			case SID_HEALTH_CHANGE_OTHER :
				{
					RakNet::BitStream BSIn(Packet->data, Packet->length, false);
					BSIn.IgnoreBytes(sizeof(RakNet::MessageID));

					int flag;

					//Receive flag of other player

					BSIn.Read((int)flag);

					for (int i = 0; i < 3; i++)
					{
						//Compare flags and change the graphics of the right player

						if (OtherPlayers[i].Flag == flag)
						{
							OtherHurtTimers[i].Reset(0);
						}
					}
				}
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				//Print("Another client has disconnected.");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				//Print("Another client has lost the connection.");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				//Print("Another client has connected.");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					GreenLight = true;

					ServerAddress = Packet->systemAddress;

					RakNet::BitStream BSOut;
					BSOut.Write((RakNet::MessageID)CID_POS_REQUEST);
					NetDevice->Send(&BSOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, ServerAddress, false);
				}
				break;					
			case ID_NEW_INCOMING_CONNECTION:
				//Print("A connection is incoming.");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				//Print("The server is full.");
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				Death = true;
				break;
			case ID_CONNECTION_LOST:
				//Print("Connection lost.");
				break;
			default:
				//printf("Message with identifier %i has arrived.\n\n", Packet->data[0]);
				break;
		}
	}
}

void Set_Up()
{
	d3ddev->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
}

void Game_Calc()
{
	if (NetLock == true)
	{
		LeftClick = false;
	}

	POINT mousepoint;
	GetCursorPos(&mousepoint);

	ScreenToClient(ghwnd, &mousepoint);

	float xoffset = 0;
	float yoffset = 0;

	float w = (float)SCREEN_WIDTH;
	float h = (float)SCREEN_HEIGHT;

	if (d3dpp.Windowed)
	{
		xoffset = 17;
		yoffset = 41;
	}

	mousepoint.x = mousepoint.x + (xoffset * (mousepoint.x / w));
	mousepoint.y = mousepoint.y + (yoffset * (mousepoint.y / h));

	if (LeftClick && MoveClickTimer.Check() <= 0)
	{
		SendMove = true;
		NetDestinationX = mousepoint.x;
		NetDestinationZ = mousepoint.y;

		MoveClickTimer.Reset(1);
	}
	else if (!LeftClick)
	{
		MoveClickTimer.Zero();
	}
}

void Mouse()
{
	//Is this frame a network frame? If so, check if the client and server are synced

	if (CheckFrame)
	{
		CheckFrame = false;

		float NetDifferenceX = MyPlayer.PosX - MyPlayer.NetPosX;
		float NetDifferenceZ = MyPlayer.PosZ - MyPlayer.NetPosZ;

		//Calculate the distance and compare it to the network threshold

		if (sqrt((NetDifferenceX * NetDifferenceX) + (NetDifferenceZ * NetDifferenceZ)) > MyPlayer.MoveFactor * 100)
		{
			MyPlayer.PosX = MyPlayer.NetPosX;
			MyPlayer.PosZ = MyPlayer.NetPosZ;
		}

		for (int i = 0; i < 3; i++)
		{
			//Do the same for other players

			float OtherNetDifferenceX = OtherPlayers[i].PosX - OtherPlayers[i].NetPosX;
			float OtherNetDifferenceZ = OtherPlayers[i].PosZ - OtherPlayers[i].NetPosZ;

			if (sqrt((OtherNetDifferenceX * OtherNetDifferenceX)
				+ (OtherNetDifferenceZ * OtherNetDifferenceZ)) > OtherPlayers[i].MoveFactor * 150)
			{
				OtherPlayers[i].PosX = OtherPlayers[i].NetPosX;
				OtherPlayers[i].PosZ = OtherPlayers[i].NetPosZ;
			}
		}
	}

	//If there is a destination, move the player

	if (NewDestination)
	{
		D3DXVECTOR3 travel(DestinationX - MyPlayer.PosX, 0, DestinationZ - MyPlayer.PosZ);

		if (sqrt((travel.x * travel.x) + (travel.z * travel.z)) < (MyPlayer.MoveFactor * float(TimeDelta)))
		{
			MyPlayer.PosX = DestinationX;
			MyPlayer.PosZ = DestinationZ;

			NewDestination = false;
		}
		else
		{
			D3DXVec3Normalize(&travel, &travel);

			MyPlayer.PosX = MyPlayer.PosX + (travel.x * MyPlayer.MoveFactor * float(TimeDelta));
			MyPlayer.PosZ = MyPlayer.PosZ + (travel.z * MyPlayer.MoveFactor * float(TimeDelta));
		}
	}

		//Check the other players

	for (int i = 0; i < 3; i++)
	{
		if (OtherPlayers[i].NewDestination)
		{
			D3DXVECTOR3 travelother(OtherPlayers[i].DestinationX - OtherPlayers[i].PosX, 0, 
				OtherPlayers[i].DestinationZ - OtherPlayers[i].PosZ);

			if (sqrt((travelother.x * travelother.x) 
				+ (travelother.z * travelother.z)) < (OtherPlayers[i].MoveFactor * float(TimeDelta)))
			{
				OtherPlayers[i].PosX = OtherPlayers[i].DestinationX;
				OtherPlayers[i].PosZ = OtherPlayers[i].DestinationZ;

				OtherPlayers[i].NewDestination = false;
			}
			else
			{
				D3DXVec3Normalize(&travelother, &travelother);

				OtherPlayers[i].PosX = OtherPlayers[i].PosX + (travelother.x * OtherPlayers[i].MoveFactor * float(TimeDelta));
				OtherPlayers[i].PosZ = OtherPlayers[i].PosZ + (travelother.z * OtherPlayers[i].MoveFactor * float(TimeDelta));
			}
		}
	}
}

void Interface()
{
	d3ddev->BeginScene();

	SpriteHandler->Begin(D3DXSPRITE_ALPHABLEND);

	//Create my character's position

	D3DXVECTOR3 CharacterPosition(MyPlayer.PosX - 10, MyPlayer.PosZ - 10, 0);
	D3DXVECTOR3 NetPosition(MyPlayer.NetPosX - 10, MyPlayer.NetPosZ - 10, 0);
	D3DXVECTOR3	GreenLightPosition(560, 5, 0);
	D3DXVECTOR3 HealthPosition(470, 320, 0);
	D3DXVECTOR3 CirclePosition((TestBall.PosX - TestBall.Radius), (TestBall.PosZ - TestBall.Radius), 0);

	//Create other characters' positions

	D3DXVECTOR3 OtherPlayerPositions[3] = {D3DXVECTOR3(OtherPlayers[0].PosX - 10, OtherPlayers[0].PosZ - 10, 0),
										D3DXVECTOR3(OtherPlayers[1].PosX - 10, OtherPlayers[1].PosZ - 10, 0),
										D3DXVECTOR3(OtherPlayers[2].PosX - 10, OtherPlayers[2].PosZ - 10, 0)};

	//Draw character sprites if they are on screen

	if (CharacterPosition.x > 0 || CharacterPosition.y > 0)
	{
		if (MyHurtTimer.Check())
		{
			SpriteHandler->Draw(Hurt, 0, 0, &CharacterPosition, White);
		}
		else
		{
			SpriteHandler->Draw(CursorDrag, 0, 0, &CharacterPosition, White);
		}
	}

	for (int i = 0; i < 3; i++)
	{
		if (OtherPlayerPositions[i].x > 0 || OtherPlayerPositions[i].y > 0)
		{
			if (OtherHurtTimers[i].Check())
			{
				SpriteHandler->Draw(Hurt, 0, 0, &OtherPlayerPositions[i], White);
			}
			else
			{
				SpriteHandler->Draw(Cursor, 0, 0, &OtherPlayerPositions[i], White);
			}
		}
	}

	//Draw health sprites depending on how much health the player has

	if (true)
	{
		for (int i = 0; i < MyPlayer.Health; i++)
		{
			SpriteHandler->Draw(Health, 0, 0, &D3DXVECTOR3((HealthPosition.x + (i * 30)), HealthPosition.y, 0), White);
		}
	}

	if (NetPosition.x > 0 || NetPosition.y > 0)
	{
		//Network debugging

		//SpriteHandler->Draw(Cursor, 0, 0, &NetPosition, White);
	}

	//Draw Circle Sprites

	SpriteHandler->Draw(TestBall.Texture, 0, 0, &CirclePosition, White);

	if (GreenLight)
	{
		SpriteHandler->Draw(Cursor, 0, 0, &GreenLightPosition, White);
	}

	SpriteHandler->End();

	d3ddev->EndScene();
}

void Present()
{
	HRESULT hr;

	hr = d3ddev->Present(NULL, NULL, NULL, NULL);

	if(hr == D3DERR_DEVICELOST)
	{
		DeviceLost = true;
	}

	MouseChangeX = 0;
	MouseChangeY = 0;
	MouseWheelDelta = 0;
	DPress = false;
	APress = false;
	SPress = false;
	WPress = false;
}

bool CheckLostDevices()
{
	HRESULT hr;

    if(DeviceLost == true)
    {
        Sleep(100);

        if(FAILED(hr = d3ddev->TestCooperativeLevel()))
        {
            if(hr == D3DERR_DEVICELOST)
			{
                return false;
			}

            if(hr == D3DERR_DEVICENOTRESET)
            {
	            ReleaseLostObjects();

                hr = d3ddev->Reset(&d3dpp);

                if(hr == D3DERR_INVALIDCALL)
				{
                    return false;
				}

                RestoreLostObjects();
            }

            return false;										//Return to skip rendering once
        }

        DeviceLost = false;
    }

	if (DeviceLost == false)
	{
		return true;
	}
}

HRESULT ReleaseLostObjects()
{
	SafeRelease(MainSurface);

	SpriteHandler->OnLostDevice();

	return S_OK;
}

HRESULT RestoreLostObjects()
{
	SpriteHandler->OnResetDevice();

	d3ddev->GetRenderTarget(0, &MainSurface);

	Proj = SetPerspective(1.0f, ratio, 1.0f, 10000.0f);

	return S_OK;
}

void SetMouseChange(int x, int y)
{
	MouseChangeX = x;
	MouseChangeY = y;
}

void SetMouseClick(int button, bool click)
{
	if (button == 0)
	{
		LeftClick = click;
	}
	else if (button == 1)
	{
		RightClick = click;
	}
}

void SetMouseWheel(int wheeldelta)
{
	MouseWheelDelta = wheeldelta;
}

void SetKeyboard(int key, bool value)
{
	switch (key)
	{
		case 1:
		{
			APress = value;
		}
		case 4:
		{
			DPress = value;
		}
		case 19:
		{
			SPress = value;
		}
		case 23:
		{
			WPress = value;
		}
	}
}

void ServerDisconnect()
{		
	RakNet::BitStream BSOut;
	BSOut.Write((RakNet::MessageID)CID_DISCONNECT);
	NetDevice->Send(&BSOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, ServerAddress, false);
}

void Game_End(HWND hwnd)
{
	SafeRelease(d3d);

	SafeRelease(d3ddev);

	SafeRelease(SpriteHandler);

	SafeRelease(MainSurface);

	SafeRelease(Cursor);

	SafeRelease(CursorDrag);

	SafeRelease(Health);

	RakNet::RakPeerInterface::DestroyInstance(NetDevice);
}