// zmq-sub.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <random>

#include <zmq.hpp>
#include <zmq_addon.hpp>

void RequestThread2(zmq::context_t* ctx)
{
    zmq::socket_t requester(*ctx, zmq::socket_type::req);
    requester.connect("tcp://localhost:5558");

    std::string buff_data("hello world");
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

    while (1)
    {
        if (!requester.send(zmq::buffer(buff_data.c_str(), buff_data.size())))
        {
            break;
        }

        zmq::message_t recv_msg;
        zmq::recv_result_t result = requester.recv(recv_msg);

        std::cout << recv_msg << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


int main(int argc, char** argv)
{
    int major, minor, patch;
    zmq::version(&major, &minor, &patch);
    printf("ZMQ Version: %d.%d.%d\n", major, minor, patch);

    zmq::context_t ctx(8);

    auto thread2 = std::async(std::launch::async, RequestThread2, &ctx);

    thread2.wait();
}