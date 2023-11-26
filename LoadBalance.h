#pragma once

#include <string>
#include <list>
#include <vector>

class Node
{
public:
    std::string url_;

    int weight_;

    int active_;
};

class LoadBalance
{
protected:
    std::vector<Node> nodes_;

public:
    LoadBalance();
    virtual ~LoadBalance();

public:
    void add(const std::string& url, int weight, int active);
    virtual Node doSelect();
};

