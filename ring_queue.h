#pragma once

#include <assert.h>
#include <stdint.h>
#include <atomic>
#include <memory>
#include <utility>

namespace RQ
{
    // size大小必须是2的幂

    template <typename T, size_t SIZE, size_t READER_NUM>
    class ring_queue
    {
    public:
        class reader
        {
        private:
            friend ring_queue;
            ring_queue* queue;

            std::atomic<size_t> limit;
            reader() {};
            reader(ring_queue* queue) : queue(queue)
            {
                this->limit = this->queue->writable_limit.load();
            }

        public:
            void pull(T& ret)
            {
                //等待数据可读
                while (queue->readable_flag[this->limit].load() < (limit / SIZE) + 1)
                    ;

                ret = queue->buffer[this->limit.load()];
                limit.fetch_add(1);
            }
        };

        class writable
        {
        private:
            ring_queue* queue;

        public:
            writable(ring_queue* queue) : queue(queue) {}

            void push(const T& val)
            {
                size_t index = queue->writable_limit.fetch_add(1);

                //等待可写
                while (!queue->min_index(index))
                    ;

                queue->buffer[index] = val;
                queue->readable_flag[index].exchange((index / SIZE) + 1);
            }
        };

    private:
        template <typename _T, size_t _SIZE>
        class ring_buffer
        {
        private:
            _T array[_SIZE];

            static constexpr size_t pow_2(const size_t num)
            {
                size_t t_num = num;

                if ((t_num & (t_num - 1))) //去掉一个1，判断是否为0
                    return 0;

                size_t i = 0;
                for (; t_num; i++)
                {
                    t_num = t_num >> 1;
                }

                return i;
            }

            size_t get_index(const size_t index) const
            {
                return index & (_SIZE - 1);
            }

        public:
            ring_buffer()
            {
                static_assert(pow_2(_SIZE), "size is not a power of two\n");
            }

            _T& operator[](const size_t index)
            {
                return this->array[get_index(index)];
            }
        };

        friend reader;
        friend writable;

        ring_buffer<T, SIZE> buffer;
        ring_buffer<std::atomic<uint64_t>, SIZE> readable_flag;

        alignas(128) std::atomic<uint64_t> writable_limit;
        alignas(128) std::atomic<uint64_t> reader_count;

        reader reader_list[READER_NUM];

        bool min_index(size_t index)
        {
            for (size_t i = 0; i < this->reader_count.load(); i++)
            {
                if (index >= this->reader_list[i].limit.load() + SIZE)
                    return false;
            }
            return true;
        }

    public:
        ring_queue() : writable_limit(0), reader_count(0)
        {
        }

        reader* make_reader()
        {
            assert(this->reader_count.load() != READER_NUM);
            auto index = this->reader_count.fetch_add(1);

            return new (&reader_list[index]) reader(this);
        }

        std::unique_ptr<writable> make_writable()
        {
            return std::make_unique<writable>(this);
        }
    };
} // namespace RQ