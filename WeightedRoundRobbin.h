#pragma once

#include "LoadBalance.h"

// 加权轮询（Weighted Round Robbin）算法在轮询算法的基础上，
// 增加了权重属性来调节转发服务器的请求数目。性能高、处理速度快的节点应该设置更高的权重，
// 使得分发时优先将请求分发到权重较高的节点上
class WeightedRoundRobbin : public LoadBalance
{
public:
	virtual ~WeightedRoundRobbin();

public:
	virtual Node doSelect() override;
};