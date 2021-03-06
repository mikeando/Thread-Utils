#ifndef ma_concurrent_queue_h
#define ma_concurrent_queue_h

// Based on code from http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
// Original version by Anthony Williams
// Modifications by Michael Anderson

#include "boost/thread.hpp"
#include <deque>

template<typename Data>
class concurrent_queue
{
private:
    std::deque<Data> the_queue;
    mutable boost::mutex the_mutex;
    boost::condition_variable the_condition_variable;
    bool is_canceled;

    //All threads that might be waiting on this queue
    boost::thread_group * waiting_threads;

public:
    concurrent_queue( boost::thread_group * waiting_thread_group ) : the_queue(), the_mutex(), the_condition_variable(), is_canceled(false), waiting_threads(waiting_thread_group) {}
    ~concurrent_queue() { cancel(); waiting_threads->join_all(); }


    struct Canceled{};
    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if (is_canceled) throw Canceled();
        the_queue.push_back(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }

    bool empty() const
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if (is_canceled) throw Canceled();
        return the_queue.empty();
    }

    bool try_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if (is_canceled) throw Canceled();
        if(the_queue.empty())
        {
            return false;
        }

        popped_value=the_queue.front();
        the_queue.pop_front();
        return true;
    }

    void wait_and_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);

        while(the_queue.empty() && !is_canceled)
        {
            the_condition_variable.wait(lock);
        }
        if (is_canceled) throw Canceled();

        popped_value=the_queue.front();
        the_queue.pop_front();
    }

    std::deque<Data> wait_and_take_all()
    {
        boost::mutex::scoped_lock lock(the_mutex);

        while(the_queue.empty() && !is_canceled)
        {
            the_condition_variable.wait(lock);
        }
        if (is_canceled) throw Canceled();

        std::deque<Data> retval;
        std::swap(retval, the_queue);
        return retval;
    }

    void cancel()
    {
       boost::mutex::scoped_lock lock(the_mutex);
       if (is_canceled) throw Canceled();
       is_canceled = true;
       lock.unlock();
       the_condition_variable.notify_all();
    }

};

#endif
