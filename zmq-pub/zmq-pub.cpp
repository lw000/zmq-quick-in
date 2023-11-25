// zmq-pub.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <random>
#include <csignal>
#include <condition_variable>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "../protos/msg/msg.pb.h"

std::atomic_bool _g_running = true;

void stopHandler(int sign)
{
    _g_running = false;
}

void PublisherThread(zmq::context_t* ctx) {
    //  Prepare publisher
    zmq::socket_t publisher(*ctx, zmq::socket_type::pub);
    publisher.bind("tcp://*:5556");

    std::string buff_data;

    //msg::ItemValue v;
    //v.set_n("1111111111");
    //v.set_nid("2222222222");
    //v.set_q(3333);
    //v.set_t(123);
    //v.set_dt(msg::INT32);
    //msg::Variable* var = new msg::Variable();
    //var->Clear();
    //var->set_i32(123);
    //v.set_allocated_v(var);
    //v.SerializeToString(&buff_data);

    int idx = 0;
    uint64_t msgId = 1;
    while (_g_running)
    {
        msg::BlockItemValue package;
        for (int i = idx; i < idx + 10; i++)
        {
            auto value = package.add_values();
            value->set_n("1111111111");
            value->set_nid("2222222222");
            value->set_q(123 + i);
            value->set_t(123 + static_cast<int64_t>(i));
            value->set_dt(msg::INT32);
            msg::Variable* var = new msg::Variable();
            var->set_i32(123 + i);
            value->set_allocated_v(var);
        }

        idx += 10;
        package.set_idx(msgId++);
        package.SerializeToString(&buff_data);

        publisher.send(zmq::buffer(buff_data.c_str(), buff_data.size()));

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "PublisherThread Exit." << std::endl;
}

int main(int argc, char** argv)
{
    signal(SIGINT, stopHandler); /* ctrl-c */
    signal(SIGTERM, stopHandler);
    signal(SIGBREAK, stopHandler);

    int major, minor, patch;
    zmq::version(&major, &minor, &patch);
    printf("ZMQ Version: %d.%d.%d\n", major, minor, patch);

    zmq::context_t ctx(8);

    auto thread1 = std::async(std::launch::async, PublisherThread, &ctx);
    thread1.wait();

    std::cout << "main Exit." << std::endl;
}