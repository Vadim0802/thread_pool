#include "thread_pool.h"
#include <iostream>
#include <chrono>

void func1()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "A";
}

void func2()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	std::cout << "O";
}


int main() 
{
	ThreadPool pool(2);

	std::thread thr([&] {
		for (int i = 0; i < 10; i++) {
			pool.push_task(func1);
			pool.push_task(func2);
		}
	});
	thr.detach();

	auto test = pool.push_task(func1);
	std::cout << test->getStatusWork() << std::endl;
}