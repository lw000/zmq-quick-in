// zmq-proxy.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <random>

#include <zmq.hpp>
#include <zmq_addon.hpp>

void ProxyThread2(zmq::context_t* ctx)
{
    zmq::socket_t frontend(*ctx, zmq::socket_type::router);
    zmq::socket_t backend(*ctx, zmq::socket_type::dealer);

    bool init = false;

    do
    {
        try
        {
            frontend.bind("tcp://*:5558");
            backend.bind("tcp://*:5559");
            init = true;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    } while (0);
   
    if (!init)
    {
        return;
    }

    zmq_pollitem_t poll_items[] =
    {
        {frontend, 0, ZMQ_POLLIN, 0},
        {backend, 0, ZMQ_POLLIN, 0},
    };

    while (1)
    {
        zmq::message_t msg;
        size_t more = 0;

        try
        {
            int rc = zmq::poll(poll_items, 2, std::chrono::milliseconds(100));
            // zmq::poll出错
            if (rc == -1)
            {
                break;
            }
            // zmq::poll超时
            if (rc == 0)
            {
                continue;
            }

            // ROUTER 套接字有数据来
            if (poll_items[0].revents & ZMQ_POLLIN)
            {
                while (1)
                {
                    frontend.recv(msg);

                    std::cout << "frontend " << msg << std::endl;

                    more = frontend.get(zmq::sockopt::rcvmore);

                    backend.send(msg, more ? zmq::send_flags::sndmore : zmq::send_flags::none);

                    if (!more)
                    {
                        break;
                    }
                }
            }

            // DEALER套接字有数据来
            if (poll_items[1].revents & ZMQ_POLLIN)
            {
                while (1)
                {

                    backend.recv(msg);

                    std::cout << "backend " << msg << std::endl;

                    more = backend.get(zmq::sockopt::rcvmore);
                    frontend.send(msg, more ? zmq::send_flags::sndmore : zmq::send_flags::none);

                    if (!more)
                    {
                        break;
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
        
    }
}

int main(int argc, char** argv)
{
    int major, minor, patch;
    zmq::version(&major, &minor, &patch);
    printf("ZMQ Version: %d.%d.%d\n", major, minor, patch);

    zmq::context_t ctx(4);

    auto thread2 = std::async(std::launch::async, ProxyThread2, &ctx);

    thread2.wait();
}