#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

class CommandQueue {
public:
    using Command = std::function<void()>;

    CommandQueue();
    ~CommandQueue() = default;

    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;

    void enqueue(Command cmd);

private:
    void workerLoop(std::stop_token st);

    std::deque<Command> queue_;
    std::mutex mutex_;
    std::condition_variable_any cv_;
    std::jthread worker_;
};
