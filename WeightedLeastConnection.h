#pragma once

#include "LoadBalance.h"

// 加权最小活跃数
class WeightedLeastConnection : public LoadBalance
{
public:
	virtual ~WeightedLeastConnection();

public:
	virtual Node doSelect() override;
};