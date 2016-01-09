#ifndef _GAME_H
#define _GAME_H

#include "WindowsIncludes.h"
#include "MessageIdentifiers.h"
#include "RakPeer.h"
#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#include <d3d9.h>
#include <dxerr.h>
#include <dsound.h>
#include <string>

#include "dxgraphics.h"
#include "Effects.h"
#include "Player.h"
#include "Delta.h"
#include "Circle.h"

#define APPTITLE "Star Engine"

#define FULLSCREEN 0
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 350

void Game_Loop(HWND hwnd);

void Timers();

void Network();

void Set_Up();

void Game_Calc();

void Mouse();

void Interface();

void Present();

int Game_Init(HWND);
void Game_Run(HWND);
void Game_End(HWND);

void ServerDisconnect();

void Init_PP();
bool Init_Shaders();
bool Init_Effects();

HRESULT ReleaseLostObjects();
HRESULT RestoreLostObjects();

bool CheckLostDevices();

void GetPickingRay(D3DXVECTOR3& originW, D3DXVECTOR3& dirW);

void SetMouseChange(int x, int y);
void SetMouseClick(int button, bool click);
void SetMouseWheel(int wheeldelta);
void SetKeyboard(int key, bool value);

#endif