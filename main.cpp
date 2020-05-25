#include "thread_pool.h"
#include <iostream>
#include <chrono>

void func1()
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "FUNC1\t" << std::this_thread::get_id() << std::endl;
}

void func2()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "FUNC 2 \t" << std::this_thread::get_id() << std::endl;
}

void func3()
{
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::cout << "RESIZE THREAD" << std::this_thread::get_id() << std::endl;
}

int main()
{
	Thread_Pool pool(2);
	
	pool.push_task(func1, 1);
	pool.push_task(func2, 2);
	pool.push_task(func1, 3);
	pool.push_task(func2, 4);
	pool.push_task(func1, 5);
}