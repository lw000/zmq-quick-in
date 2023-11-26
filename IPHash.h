#pragma once

#include "LoadBalance.h"

// 源地址哈希（IP Hash）算法 根据请求源 IP，通过哈希计算得到一个数值，用该数值在候选服务器列表的进行取模运算，得到的结果便是选中的服务器
class IPHash : public LoadBalance
{
public:
	virtual ~IPHash();

public:
	virtual Node doSelect() override;
};