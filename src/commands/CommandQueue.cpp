#include "CommandQueue.h"

CommandQueue::CommandQueue()
    : worker_([this](std::stop_token st) { workerLoop(std::move(st)); })
{}

void CommandQueue::enqueue(Command cmd) {
    {
        std::lock_guard lock(mutex_);
        queue_.push_back(std::move(cmd));
    }
    cv_.notify_one();
}

void CommandQueue::workerLoop(std::stop_token st) {
    while (!st.stop_requested()) {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, st, [this] { return !queue_.empty(); });
        if (queue_.empty()) break;
        auto cmd = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        cmd();
    }
}
