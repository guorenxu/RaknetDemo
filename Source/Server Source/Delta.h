#ifndef __DELTA_H
#define __DELTA_H

class Delta
{
	public:

		long Timer;
		long Base;
		bool Active;

		Delta(long int base);
		~Delta(){};

		void Update(long timepassed);				//Updates the timer for time passed
		void UpdateZero(long timepassed);			//Updates the timer for time passed in a special mode which doesn't let time past 0
		void Reset(int resetoradd);					//0 - Resets to base		 1 - Adds the base time to the timer
		long Check();								//Returns current time remaining

		void Activate();							//Remember order of activation and deactivation for frame by
		void Deactivate();							//frame accuracy
};

#endif