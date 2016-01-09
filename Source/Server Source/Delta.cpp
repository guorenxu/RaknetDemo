#include "Delta.h"
#include "time.h"

Delta::Delta(long int base)
{
	Timer = 0;
	Base = base;
	Active = false;
}

void Delta::Reset(int resetoradd)
{
	if (resetoradd == 0)
	{
		Timer = Base;
	}
	else
	{
		Timer = Timer + Base;
	}
}

void Delta::Update(long timepassed)
{
	if (Active)
	{
		Timer = Timer - timepassed;
	}
}

void Delta::UpdateZero(long timepassed)
{
	if (Active)
	{
		Timer = Timer - timepassed;

		if (Timer < 0)
		{
			Timer = 0;
		}
	}
}

long int Delta::Check()
{
	return Timer;
}

void Delta::Activate()
{
	Active = true;
}

void Delta::Deactivate()
{
	Active = false;
}