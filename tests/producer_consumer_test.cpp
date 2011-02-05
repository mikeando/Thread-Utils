#include "producer_consumer_queue.hpp"
#include <iostream>


void consumer( concurrent_queue<int> * queue )
{
  try
  {
  while(true)
  {
    int i;
    std::cout<<"About to wait"<<std::endl;
    queue->wait_and_pop(i);
    std::cout<<"Got me an "<<i<<std::endl;
  }
  }
  catch( const concurrent_queue<int>::Canceled & e )
  {
    return;
  }
}



int main()
{
  {
    boost::thread_group workers;
    concurrent_queue<int> queue( &workers) ;

    workers.add_thread( new boost::thread(consumer, &queue) );

    queue.push(7);

    sleep(1); // This gives the other thread a chance to wait again..
  }
}
