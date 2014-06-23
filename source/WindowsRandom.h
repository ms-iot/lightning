#ifndef WINDOWS_RANDOM_H
#define WINDOWS_RANDOM_H

#include <random>

// implement the Arduino random functions for Windows
class WindowsRandom
{
private:
	std::mt19937 Engine;
	std::uniform_int_distribution<int> Distribution;

public:
	WindowsRandom()
	{
	}

	void Seed(int seed)
	{
		Engine.seed(seed);
	}

	long Next()
	{
		return Distribution(Engine);
	}
};

__declspec(selectany) WindowsRandom _WindowsRandom;

#endif
