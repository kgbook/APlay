#include "thread.hpp"
#include <pthread.h>

namespace aplay {
namespace core {

static const std::size_t kMaxThreadNameLength = 15;

static std::string normalizedName(const std::string& name)
{
    if (name.size() <= kMaxThreadNameLength) {
        return name;
    }
    return name.substr(0, kMaxThreadNameLength);
}

Thread::Thread(const std::string& name)
    : name_(normalizedName(name)),
      running_(false),
      stopRequested_(false),
      paused_(false)
{
}

Thread::~Thread()
{
    stopAndJoin();
}

bool Thread::start()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (running_.load()) {
        return false;
    }
    if (worker_.joinable()) {
        if (worker_.get_id() == std::this_thread::get_id()) {
            return false;
        }
        worker_.join();
    }

    stopRequested_.store(false);
    paused_.store(false);
    running_.store(true);

    try {
        worker_ = std::thread(&Thread::run, this);
    } catch (...) {
        running_.store(false);
        stopRequested_.store(true);
        throw;
    }

    return true;
}

void Thread::stop()
{
    stopRequested_.store(true);
    resume();
}

void Thread::pause()
{
    if (running_.load()) {
        paused_.store(true);
    }
}

void Thread::resume()
{
    paused_.store(false);
    pauseCondition_.notify_all();
}

void Thread::join()
{
    if (!worker_.joinable()) {
        return;
    }

    if (worker_.get_id() == std::this_thread::get_id()) {
        return;
    }

    worker_.join();
}

void Thread::stopAndJoin()
{
    stop();
    join();
}

bool Thread::isRunning() const
{
    return running_.load();
}

bool Thread::isStopRequested() const
{
    return stopRequested_.load();
}

bool Thread::isPaused() const
{
    return paused_.load();
}

void Thread::setName(const std::string& name)
{
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        name_ = normalizedName(name);
    }

    if (running_.load() && worker_.get_id() == std::this_thread::get_id()) {
        applyThreadName();
    }
}

std::string Thread::name() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return name_;
}

bool Thread::prepare()
{
    return true;
}

void Thread::finish()
{
}

void Thread::run()
{
    applyThreadName();

    if (prepare()) {
        while (!stopRequested_.load()) {
            waitIfPaused();
            if (stopRequested_.load() || !runOnce()) {
                break;
            }
        }
    }

    finish();
    running_.store(false);
    paused_.store(false);
}

void Thread::waitIfPaused()
{
    if (!paused_.load()) {
        return;
    }

    std::unique_lock<std::mutex> lock(stateMutex_);
    pauseCondition_.wait(lock, [this] {
        return !paused_.load() || stopRequested_.load();
    });
}

void Thread::applyThreadName() const
{
    std::string threadName = name();
    if (threadName.empty()) {
        return;
    }

#if defined(__APPLE__) || defined(__MACH__)
    pthread_setname_np(threadName.c_str());
#elif defined(__ANDROID__) || defined(__linux__)
    pthread_setname_np(pthread_self(), threadName.c_str());
#endif
}

} // namespace core
} // namespace aplay
