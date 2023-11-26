// zmq-demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <random>
#include <mutex>
#include <shared_mutex>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "protos/msg/msg.pb.h"
#include "utils.h"

#include "ring_queue.h"

#include "Random.h"

void SubscriberThread1(zmq::context_t* ctx) {
    //  Prepare subscriber
    zmq::socket_t subscriber(*ctx, zmq::socket_type::sub);
    subscriber.connect("inproc://#1");
    //subscriber.connect("ipc://weather.ipc");
    
    //  Thread2 opens "A" and "B" envelopes
    subscriber.set(zmq::sockopt::subscribe, "A");
    subscriber.set(zmq::sockopt::subscribe, "B");

    while (1) {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result =
            zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
        assert(result && "recv failed");
        assert(*result == 2);

        //std::cout << "Thread2: [" << recv_msgs[0].to_string() << "] "
        //    << recv_msgs[1].to_string() << std::endl;

        //msg::ItemValue v1;
        msg::BlockItemValue values;
        values.ParseFromString(recv_msgs[1].to_string());

        std::cout << "Thread2: [" << recv_msgs[0].to_string() << "] "
            << values << std::endl;
    }
}

void SubscriberThread2(zmq::context_t* ctx) {
    //  Prepare our context and subscriber
    zmq::socket_t subscriber(*ctx, zmq::socket_type::sub);
    subscriber.connect("inproc://#1");
    //subscriber.connect("ipc://weather.ipc");

    //  Thread3 opens ALL envelopes
    subscriber.set(zmq::sockopt::subscribe, "");

    while (1) {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result =
            zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
        assert(result && "recv failed");
        assert(*result == 2);

        //std::cout << "Thread3: [" << recv_msgs[0].to_string() << "] "
        //    << recv_msgs[1].to_string() << std::endl;

        //msg::ItemValue v1;
        msg::BlockItemValue values;
        values.ParseFromString(recv_msgs[1].to_string());

        std::cout << "Thread3: [" << recv_msgs[0].to_string() << "] "
            << values << std::endl;
    }
}

void PublisherThread(zmq::context_t* ctx) {
    //  Prepare publisher
    zmq::socket_t publisher(*ctx, zmq::socket_type::pub);
    publisher.bind("inproc://#1");
    //publisher.bind("ipc://weather.ipc");


    // Give the subscribers a chance to connect, so they don't lose any messages
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::string buff_data;

    msg::ItemValue v;
    v.set_n("1111111111");
    v.set_nid("2222222222");
    v.set_q(3333);
    v.set_t(123);
    v.set_dt(msg::INT32);
    msg::Variable* var = new msg::Variable();
    var->Clear();
    var->set_i32(123);
    v.set_allocated_v(var);
    //v.SerializeToString(&buff_data);

    msg::BlockItemValue package;
    for (int i = 0; i < 100; i++)
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

    package.SerializeToString(&buff_data);

    while (true) {

        //  Write three messages, each with an envelope and content
        publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
        publisher.send(zmq::buffer(buff_data.c_str(), buff_data.size()));

        publisher.send(zmq::str_buffer("B"), zmq::send_flags::sndmore);
        //publisher.send(zmq::str_buffer("Message in B envelope"));
        publisher.send(zmq::buffer(buff_data.c_str(), buff_data.size()));

        publisher.send(zmq::str_buffer("C"), zmq::send_flags::sndmore);
        //publisher.send(zmq::str_buffer("Message in C envelope"));
        publisher.send(zmq::buffer(buff_data.c_str(), buff_data.size()));

        publisher.send(zmq::str_buffer("D"), zmq::send_flags::sndmore);
        //zmq::message_t t("aaaaaaaaaaa", strlen("aaaaaaaaaaa"));
        publisher.send(zmq::buffer(buff_data.c_str(), buff_data.size()));
        //publisher.send(t, zmq::send_flags::none);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}



int main() {
    /*
     * No I/O threads are involved in passing messages using the inproc transport.
     * Therefore, if you are using a ØMQ context for in-process messaging only you
     * can initialise the context with zero I/O threads.
     *
     * Source: http://api.zeromq.org/4-3:zmq-inproc
     */

    int major, minor, patch;
    zmq::version(&major, &minor, &patch);
    printf("ZMQ Version: %d.%d.%d\n", major, minor, patch);


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

    int c = getchar();

    RQ::ring_queue<int, 1024, 2> rq;
    std::thread t1([&rq]() {
        auto writable = rq.make_writable();
        for (;;)
        {
            int v = rand();
            writable->push(v);
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });

    std::thread t2([&rq]() {
        auto reader = rq.make_reader();
        for (;;)
        {
            int v;
            reader->pull(v);
            std::cout << v << std::endl;
            std::cout << "t2 " << v << std::endl;
        }
      
        });

    std::thread t3([&rq]() {
        auto reader = rq.make_reader();
        for (;;)
        {
            int v;
            reader->pull(v);
            std::cout << "t3 " << v << std::endl;
        }

        });
    t1.join();
    t2.join();
    t3.join();

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

    zmq::context_t ctx(0);

    auto thread1 = std::async(std::launch::async, PublisherThread, &ctx);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto thread2 = std::async(std::launch::async, SubscriberThread1, &ctx);
    //auto thread3 = std::async(std::launch::async, SubscriberThread2, &ctx);

    thread1.wait();
    thread2.wait();
    //thread3.wait();

    /*
     * Output:
     *   An infinite loop of a mix of:
     *     Thread2: [A] Message in A envelope
     *     Thread2: [B] Message in B envelope
     *     Thread3: [A] Message in A envelope
     *     Thread3: [B] Message in B envelope
     *     Thread3: [C] Message in C envelope
     */
}