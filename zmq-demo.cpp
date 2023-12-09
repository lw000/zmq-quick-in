// zmq-demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

#include <memory>//
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <random>
#include <mutex>
#include <shared_mutex>
#include "ring_queue.h"

#include "Random.h"

#include "../zmq-demo/protos/msg/msg.pb.h"


class User : public std::enable_shared_from_this<User>
{
    std::string name_;
    int age_;

public:
    User() : age_(0) {

    }

    ~User() {

    }

    void set_age(int age) {
        age_ = age;
    }

    void set_name(const std::string& name) {
        name_ = name;
    }

    int get_age() const {
        return age_;
    }

    const std::string& get_name() const {
        return name_;
    }
};

void printf_user(const std::shared_ptr<User>& user)
{
    auto use_count = user.use_count();
    std::cout << use_count << std::endl;
}

int main() {
    {
        std::shared_ptr<User> u = std::make_shared<User>();
        std::shared_ptr<User> u1 = u->shared_from_this();

        u->set_age(10);
        u->set_name("levi");

        std::cout << "name:" << u->get_name() << " age:" << u->get_age() << std::endl;
        std::cout << "name:" << u1->get_name() << " age:" << u1->get_age() << std::endl;

        for (auto i = 0; i < 10; i++)
        {
            printf_user(u);
        }
    }

    {
        std::string data;
        msg::ItemValue v;
        v.set_n("1111111111");
        v.set_nid("2222222222");
        v.set_q(3333);
        v.set_t(3333);
        v.set_dt(msg::INT32);
        msg::Variable* var = v.mutable_v(); // new msg::Variable();
        var->set_i32(3333);
        //v.set_allocated_v(var); 
        v.SerializeToString(&data);

        msg::ItemValue v1;
        v1.ParseFromString(data);
    }

    {
        std::unique_ptr<LoadBalance> lb = std::make_unique<Random>();
        lb->add("192.168.0.100", 1, 1);
        lb->add("192.168.0.101", 1, 1);
        lb->add("192.168.0.102", 1, 1);
        lb->add("192.168.0.103", 1, 1);
        lb->add("192.168.0.104", 1, 1);
        lb->add("192.168.0.105", 1, 1);
        lb->add("192.168.0.106", 1, 1);
        lb->add("192.168.0.107", 1, 1);
        lb->add("192.168.0.108", 1, 1);
        lb->add("192.168.0.109", 1, 1);

        for (int i = 0; i < 100; i++)
        {
            auto node = lb->doSelect();

            std::cout << node.url_ << std::endl;
        }
    }

    RQ::ring_queue<int, 1024, 2> rq;
    std::thread t1([&rq]() {
        auto writable = rq.make_writable();
        for (;;)
        {
            int v = rand();
            writable->push(v);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        });

    std::thread t2([&rq]() {
        auto reader = rq.make_reader();
        for (;;)
        {
            int v;
            reader->pull(v);
            std::cout << v << std::endl;
            std::cout << "t2: " << v << std::endl;
        }
      
        });

    std::thread t3([&rq]() {
        auto reader = rq.make_reader();
        for (;;)
        {
            int v;
            reader->pull(v);
            std::cout << "t3: " << v << std::endl;
        }

        });
    t1.join();
    t2.join();
    t3.join();

}