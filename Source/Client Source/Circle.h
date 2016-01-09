#include <d3d9.h>

#ifndef __CIRCLE_H
#define __CIRCLE_H

class Circle											//Self Explanatory storage for circle information
{
	public:

		float Radius;
		float PosX;
		float PosZ;

		LPDIRECT3DTEXTURE9 Texture;						//Holds the texture of the circular object

		Circle(float radius, float posx, float posz);
		~Circle(){};
};

#endif