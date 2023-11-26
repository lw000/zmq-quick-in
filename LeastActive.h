#pragma once

#include "LoadBalance.h"

// 最小活跃数（Least Active）算法 将请求分发到连接数/请求数最少的候选服务器（目前处理请求最少的服务器）
class LeastActive : public LoadBalance
{
public:
	virtual ~LeastActive();

public:
	virtual Node doSelect() override;
};
