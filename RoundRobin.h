#pragma once

#include "LoadBalance.h"

// 轮询（Round Robin）算法的策略是：将请求依次分发到候选服务器
class RoundRobin : public LoadBalance
{
public:
	virtual ~RoundRobin();

public:
	virtual Node doSelect() override;
};