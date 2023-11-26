#pragma once

#include "LoadBalance.h"

// 加权随机算法
class WeightedRandom : public LoadBalance
{
public:
	virtual ~WeightedRandom();

public:
	virtual Node doSelect() override;
};