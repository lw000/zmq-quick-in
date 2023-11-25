// zmq-sub.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <random>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "../protos/msg/msg.pb.h"
#include "../utils.h"

void SubscriberThread2(zmq::context_t* ctx) {
    //  Prepare our context and subscriber
    zmq::socket_t subscriber(*ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://localhost:5556");

    subscriber.set(zmq::sockopt::subscribe, "");

    while (1) {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result =
            zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
        assert(result && "recv failed");
        assert(*result == 1);

        msg::BlockItemValue values;
        values.ParseFromString(recv_msgs[0].to_string());

        std::cout << values << std::endl;
    }
}


int main(int argc, char** argv)
{
    int major, minor, patch;
    zmq::version(&major, &minor, &patch);
    printf("ZMQ Version: %d.%d.%d\n", major, minor, patch);

    zmq::context_t ctx(8);

    auto thread2 = std::async(std::launch::async, SubscriberThread2, &ctx);

    thread2.wait();
}