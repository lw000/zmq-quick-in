#include "Random.h"

#include <cmath>
#include <random>
#include <ctime>

Random::~Random()
{
}

Node Random::doSelect()
{
	int i = rand() % nodes_.size();

	return nodes_.at(i);
}
