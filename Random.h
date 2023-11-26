#pragma once

#include "LoadBalance.h"

// 随机算法
class Random : public LoadBalance
{
public:
	virtual ~Random();

public:
	virtual Node doSelect() override;
};

