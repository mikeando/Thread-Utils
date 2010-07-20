// Based on code from http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
// Original version by Anthony Williams
// Modifications by Michael Anderson

#include "boost/thread.hpp"

template<typename Data>
class concurrent_queue
{
private:
    std::queue<Data> the_queue;
    mutable boost::mutex the_mutex;
    boost::condition_variable the_condition_variable;
    bool is_canceled;

public:
    struct Canceled{};
    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if (is_canceled) throw Canceled();
        the_queue.push(data);
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
        the_queue.pop();
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
        the_queue.pop();
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