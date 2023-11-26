#include "LoadBalance.h"
#include <cmath>
#include <random>
#include <ctime>

LoadBalance::LoadBalance()
{
	srand(time(NULL));
}

LoadBalance::~LoadBalance()
{
}

void LoadBalance::add(const std::string& url, int weight, int active)
{
	nodes_.push_back(Node{url, weight, active});
}

Node LoadBalance::doSelect()
{
	return Node();
}
