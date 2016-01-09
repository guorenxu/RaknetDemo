#ifndef _DXGRAPHICS_H
#define _DXGRAPHICS_H

#include "WindowsIncludes.h"

#include <d3d9.h>
#include <d3dx9.h>

#define D3DFVF_MYVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

int Init_Direct3D(HWND, int, int, int);
LPDIRECT3DSURFACE9 LoadSurface(char*, D3DCOLOR);
LPDIRECT3DTEXTURE9 LoadTexture(char*, D3DCOLOR);
D3DXMATRIX SetCamera(float,float,float,float,float,float);
D3DXMATRIX SetPerspective(float,float,float,float);
void ClearScene(D3DXCOLOR);

extern LPDIRECT3D9 d3d; 
extern LPDIRECT3DDEVICE9 d3ddev; 
extern D3DXVECTOR3 cameraSource;
extern D3DXVECTOR3 cameraTarget;

#endif