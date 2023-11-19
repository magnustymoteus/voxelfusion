#pragma once
#include <queue>

//https://stackoverflow.com/questions/32227321/atomic-operation-on-queuet
template<typename T>
class AtomicQueue
{
public:
    void enqueue(const T& value )
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);
    }

    bool dequeue(T& out)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_queue.empty()) return false;
        out = m_queue.front();
        m_queue.pop();
        return true;
    }

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
};

