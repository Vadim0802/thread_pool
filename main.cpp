#include "thread_pool.h"
#include <iostream>
#include <chrono>

void func1()
{
	//std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << std::this_thread::get_id() << std::endl;
}

int main()
{
	Thread_Pool pool(2);

	std::thread thr([&] {
		for (int i = 0; i < 10000; i++) {
			pool.push_task(func1, i);
		}
	});
	thr.detach();
}