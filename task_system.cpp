#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <future>
#include <string>
#include <mutex>
#include <condition_variable>

class Task {
public:
    Task(int id, const std::string& name) : id(id), name(name) {}

    void execute() {
        std::cout << "Executing task " << id << ": " << name << std::endl;
        // Здесь добавляем логику выполнения задачи
        int result = 0;
        for (int i = 0; i <= id; ++i) {
            result += i;
        }
        std::cout << "Result of task " << id << ": " << result << std::endl;
    }

private:
    int id;
    std::string name;
};
class TaskQueue {
public:
    void push(const Task& task) {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push(task);
        cv.notify_one();
    }

    Task pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !tasks.empty(); });
        Task task = tasks.front();
        tasks.pop();
        return task;
    }

private:
    std::queue<Task> tasks;
    std::mutex mtx;
    std::condition_variable cv;
};

void worker(TaskQueue& queue) {
    while (true) {
        Task task = queue.pop();
        task.execute();
    }
}

int main() {
    TaskQueue queue;

    // Запускаем рабочие потоки
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 3; ++i) {
        futures.push_back(std::async(std::launch::async, worker, std::ref(queue)));
    }

    // Добавляем задачи в очередь
    for (int i = 0; i < 10; ++i) {
        queue.push(Task(i, "Task " + std::to_string(i)));
    }

    // Ожидаем завершения всех рабочих потоков
    for (auto& future : futures) {
        future.get();
    }

    return 0;
}
