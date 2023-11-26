// zmq-proxy.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <random>
#include <memory>
#include <csignal>
#include <condition_variable>
#include <mutex>

#include <zmq.hpp>
#include <zmq_addon.hpp>

static std::mutex g_mtx_;
static std::condition_variable g_cv_;
static std::atomic_bool g_app_running_ = true;

class Proxy
{
    std::atomic_bool stoped_;

    zmq::context_t context_;
    std::unique_ptr<zmq::socket_t> frontend_;
    std::unique_ptr<zmq::socket_t> backend_;
    std::future<void> thread_result_;

public:
    Proxy(int io_threads = 1);

public:
    bool init();
    bool start();
    void stop();
    void wait();

private:
    void _run();

private:
    static void stop_handler(int sig);
};

Proxy::Proxy(int io_threads) : context_(io_threads), stoped_(false)
{
    frontend_ = std::make_unique<zmq::socket_t>(context_, zmq::socket_type::router);
    backend_ = std::make_unique<zmq::socket_t>(context_, zmq::socket_type::dealer);
}

bool Proxy::init()
{
    signal(SIGINT, stop_handler);
    signal(SIGBREAK, stop_handler);
    signal(SIGTERM, stop_handler);

    bool init = false;

    try
    {
        frontend_->bind("tcp://*:5558");
        backend_->bind("tcp://*:5559");
        init = true;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return init;
}

bool Proxy::start()
{
    stoped_ = false;

    thread_result_ = std::async(std::launch::async, [this]() { this->_run(); });

    return true;
}

void Proxy::stop()
{
    std::unique_lock<std::mutex> lock(g_mtx_);
    g_app_running_ = false;
    stoped_ = true;
    g_cv_.notify_one();
}

void Proxy::wait()
{
    {
        std::unique_lock<std::mutex> lock(g_mtx_);
        g_cv_.wait(lock, []() { return !g_app_running_; });
    }

    stoped_ = true;
    thread_result_.wait();
}

void Proxy::stop_handler(int sig)
{
    std::unique_lock<std::mutex> lock(g_mtx_);

    g_app_running_ = false;

    g_cv_.notify_one();
}

void Proxy::_run()
{
    zmq_pollitem_t poll_items[] =
    {
        {*frontend_.get(), 0, ZMQ_POLLIN, 0},
        {*backend_.get(), 0, ZMQ_POLLIN, 0},
    };

    while (!stoped_)
    {
        zmq::message_t msg;
        size_t more = 0;

        int rc = 0;
        try
        {
            rc = zmq::poll(poll_items, 2, std::chrono::milliseconds(100));
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
        }
        catch (const std::exception& e)
        {
            std::cerr << "POLL:" << e.what() << std::endl;

            break;
        }

        // ROUTER 套接字有数据来
        if (poll_items[0].revents & ZMQ_POLLIN)
        {
            int idx = 0;
            while (1)
            {
                try
                {
                    frontend_->recv(msg);

                    std::cout << "frontend " << ++idx << " " << msg << std::endl;

                    more = frontend_->get(zmq::sockopt::rcvmore);

                    backend_->send(msg, more ? zmq::send_flags::sndmore : zmq::send_flags::none);

                    if (!more)
                    {
                        break;
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ROUTER: " << e.what() << std::endl;
                    break;
                }
            }
        }

        // DEALER 套接字有数据来
        if (poll_items[1].revents & ZMQ_POLLIN)
        {
            while (1)
            {
                try
                {
                    backend_->recv(msg);

                    std::cout << "backend " << msg << std::endl;

                    more = backend_->get(zmq::sockopt::rcvmore);
                    frontend_->send(msg, more ? zmq::send_flags::sndmore : zmq::send_flags::none);

                    if (!more)
                    {
                        break;
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "DEALER: " << e.what() << std::endl;
                    break;
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    int major, minor, patch;
    zmq::version(&major, &minor, &patch);
    printf("ZMQ Version: %d.%d.%d\n", major, minor, patch);
    
    printf("Proxy Start\n");

    Proxy proxy(1);
    if (proxy.init())
    {
        if (!proxy.start())
        {
            proxy.stop();
        }
        proxy.wait();
    }

    printf("Proxy Exit\n");
}
