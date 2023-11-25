// zmq-sub.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <random>

#include <zmq.hpp>
#include <zmq_addon.hpp>

void ResponseThread2(zmq::context_t* ctx)
{
    zmq::socket_t requester(*ctx, zmq::socket_type::rep);
    requester.connect("tcp://localhost:5559");

    while (1)
    {
        {
            zmq::message_t recv_msg;
            zmq::recv_result_t result = requester.recv(recv_msg);

            std::cout << recv_msg << std::endl;
        }

        {
            //zmq::message_t msg("ok");
            std::string buff_data("ok");
            zmq::recv_result_t result = requester.send(zmq::buffer(buff_data.c_str(), buff_data.size()));
            if (!result)
            {
                break;
            }
        }
    }
}


int main(int argc, char** argv)
{
    int major, minor, patch;
    zmq::version(&major, &minor, &patch);
    printf("ZMQ Version: %d.%d.%d\n", major, minor, patch);

    zmq::context_t ctx(8);

    auto thread2 = std::async(std::launch::async, ResponseThread2, &ctx);

    thread2.wait();
}