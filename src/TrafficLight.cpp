#include <iostream>
#include <random>
#include <thread>
#include <future>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	std::unique_lock<std::mutex> uLock(_mutex);
  	_cond.wait(uLock, [this] { return !_queue.empty(); });
  	T message = std::move(_queue.back());
  	_queue.pop_back();
//   	_queue.clear();
  	return message;
}

template <typename T>
void MessageQueue<T>::send(T &&message)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::this_thread::sleep_for(std::chrono::milliseconds(100));
  	std::lock_guard<std::mutex> uLock(_mutex);
  	_queue.push_back(std::move(message));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _messages = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto msg = _messages->receive();
        if (msg == TrafficLightPhase::green) 
          return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  	std:: random_device rand;
  	std::mt19937 rng(rand());
  	std::uniform_int_distribution<std::mt19937::result_type> distribution(4000, 6000);
  	long cycleDuration = distribution(rng);
  	std::chrono::time_point<std::chrono::system_clock> currTime;
  	currTime = std::chrono::system_clock::now();
  
  	while(true){
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      long timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - currTime).count();
      
      if(timeTaken >= cycleDuration){
        if(_currentPhase == TrafficLightPhase::red){
          _currentPhase = TrafficLightPhase::green;
        }
        else{
          _currentPhase = TrafficLightPhase::red;
        }
        auto ftr = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _messages, std::move(_currentPhase));
        ftr.wait();
		
        currTime = std::chrono::system_clock::now();
        cycleDuration = distribution(rng);
      }
    }
}

