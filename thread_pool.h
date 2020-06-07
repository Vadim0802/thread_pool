#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>
#include <string>
#include <vector>
#include <functional>



class ThreadPool
{
private:
	class Task
	{
	private:
		std::string	          _statusWork;
		std::function<void()> _job;
	public:
		Task(std::function<void()> job)
		{
			this->_job = job;
			this->_statusWork = "inQueue";
		}
		void operator()() { _job(); };
		std::string getStatusWork() { return this->_statusWork; };
		void setStatusWork(std::string status) { this->_statusWork = status; };
	};
	std::mutex	                       _mutex;
	std::condition_variable	           _condition;
	std::vector<std::thread>           _threads;
	std::vector<std::shared_ptr<Task>> _workerQueue;

public:
	auto push_task(std::function<void()> job)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		Task worker(job);
		_workerQueue.push_back(std::make_shared<Task>(std::move(worker)));

		_condition.notify_one();

		return _workerQueue.back();
	}

	std::string getStatus(std::shared_ptr<Task> ptr) { return ptr->getStatusWork(); };

	ThreadPool(unsigned int threadCount)
	{
		int MAX_THREADS = std::thread::hardware_concurrency() - 1;
		if (threadCount > MAX_THREADS)
			threadCount = MAX_THREADS;

		for (int i = 0; i < threadCount; i++)
		{
			_threads.emplace_back([=] {
				while (true)
				{
					for (int j = 0; j < _workerQueue.size(); j++)
					{
						auto Task = _workerQueue[j];

						if (Task->getStatusWork() == "inQueue")
						{
							_mutex.lock();
							(*Task).setStatusWork("inThreading");
							_mutex.unlock();
							(*Task)();
							(*Task).setStatusWork("Completed");
						}
					}

					std::unique_lock<std::mutex> lock(_mutex);
					_condition.wait(lock, [this] {return !_workerQueue.empty(); });

					bool threadEndWork = true;

					for (auto iter : _workerQueue)
					{
						if (iter->getStatusWork() != "Completed")
							threadEndWork = false;
					}
					if (threadEndWork)
						break;
				}
			});
		}
	}
	~ThreadPool()
	{
		for (auto& t : _threads)
		{
			if (t.joinable())
				t.join();
		}
	}
};
