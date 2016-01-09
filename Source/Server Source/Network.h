#ifndef __NETWORK_H
#define __NETWORK_H

#include "WindowsIncludes.h"
#include "MessageIdentifiers.h"
#include "RakPeer.h"
#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#include <iostream>
#include <stdio.h>
#include "string"
#include "Player.h"
#include "Delta.h"
#include "Circle.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

void Timers();
void Network();
void Game();

bool CirclePlayerCollision(Circle circle, Player player);

void Print(std::string str);
void Skip();

#endif