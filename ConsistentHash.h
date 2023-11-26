#pragma once

#include "LoadBalance.h"

// 一致性哈希（Consistent Hash）算法的目标是：相同的请求尽可能落到同一个服务器上。
class ConsistentHash : public LoadBalance
{
public:
	virtual ~ConsistentHash();

public:
	virtual Node doSelect() override;
};