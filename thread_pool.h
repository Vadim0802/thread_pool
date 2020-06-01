#pragma once
#include <thread>
#include <functional>
#include <condition_variable>
#include <queue>
#include <vector>
#include <deque>
#include <memory>
#include <future>

class Thread_Pool
{
private:
	class Worker
	{
	private:
		std::function<void()> func;
		uint8_t worker_id;
		bool isInQueue = false;
		bool isPerformed = false;
		bool isCompleted = false;

	public:
		Worker(std::function<void()> worker, int id)
		{
			this->func = worker;
			this->worker_id = id;
			this->isInQueue = true;
		}
		~Worker() {};
		bool get_isInQueue() { return isInQueue; };
		bool get_isPerformed() { return isPerformed; };
		bool get_isCompleted() { return isCompleted; };
		int get_id() { return worker_id; };
		void set_isInQueue(bool set) { isInQueue = set; };
		void set_isPerformed(bool set) { isPerformed = set; };
		void set_isCompleted(bool set) { isCompleted = set; };
		std::function<void()> get_function() { return func; };
	};
	bool _done = false;
	bool _threadEndWork = false;
	std::condition_variable _condition;
	std::mutex _mutex;
	std::vector<std::thread> _threads;
	std::deque<std::shared_ptr<Worker>> _workerQueue;
	std::vector<std::shared_ptr<Worker>> _workerQueue_buffer;

public:
	Thread_Pool(unsigned int threadCount = 1)
	{
		for (int i = 0; i < threadCount; i++)
		{
			_threads.emplace_back([=] {
				while (true)
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock{ _mutex };
						
						_condition.wait(lock, [=] {return !_workerQueue.empty() || _threadEndWork; });

						if (_done && _workerQueue.empty())
							break;

						task = _workerQueue.front()->get_function();
						_workerQueue_buffer.push_back(_workerQueue.front());
						_workerQueue.pop_front();
						_workerQueue_buffer.back()->set_isInQueue(false);
						_workerQueue_buffer.back()->set_isPerformed(true);
					}
					task();
					_workerQueue_buffer.back()->set_isPerformed(false);
					_workerQueue_buffer.back()->set_isCompleted(true);
					if (_workerQueue.empty() && _done )
					{
						_threadEndWork = true;
					}
				}
			});
		}
	}

	~Thread_Pool()
	{

		{
			std::unique_lock<std::mutex> lock(_mutex);
			_done = true;
		}

		_condition.notify_all();

		for (auto& t : _threads)
		{
			if (t.joinable())
				t.join();
		}
	}

	void push_task(std::function<void()> task, int id)
	{
		Worker job(task, id);
		auto func = std::make_shared<Worker>(std::move(job));
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_workerQueue.push_back(func);
		}
		
		_condition.notify_one();
	}

	bool isInQueue(int id)
	{
		for (int i = 0; i < _workerQueue.size(); i++)
		{
			auto tmp = _workerQueue[i];
			auto tmp_2 = *tmp;
			if (tmp_2.get_id() == id)
			{
				return tmp_2.get_isInQueue();
			}
		}
	}

	bool isPerformed(int id)
	{
		for (int i = 0; i < _workerQueue_buffer.size(); i++)
		{
			auto tmp = _workerQueue_buffer[i];
			auto tmp_2 = *tmp;
			if (tmp_2.get_id() == id)
			{
				return tmp_2.get_isPerformed();
			}
		}
	}

	bool isCompleted(int id)
	{
		for (int i = 0; i < _workerQueue_buffer.size(); i++)
		{
			auto tmp = _workerQueue_buffer[i];
			auto tmp_2 = *tmp;
			if (tmp_2.get_id() == id)
			{
				return tmp_2.get_isCompleted();
			}
		}
	}
};