#ifndef __CIRCLE_H
#define __CIRCLE_H

class Circle 											//Self Explanatory storage for circle information
{
	public:

		float Radius;
		float PosX;
		float PosZ;

		Circle(float radius, float posx, float posz);
		~Circle(){};
};

#endif