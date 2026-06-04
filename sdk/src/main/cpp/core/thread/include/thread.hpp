#ifndef APLAY_CORE_THREAD_HPP
#define APLAY_CORE_THREAD_HPP

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace aplay {
namespace core {

class Thread {
public:
    explicit Thread(const std::string& name = std::string());
    virtual ~Thread();

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    bool start();
    void stop();
    void pause();
    void resume();
    void join();
    void stopAndJoin();

    bool isRunning() const;
    bool isStopRequested() const;
    bool isPaused() const;

protected:
    void setName(const std::string& name);
    std::string name() const;

    virtual bool prepare();
    virtual bool runOnce() = 0;
    virtual void finish();

private:
    void run();
    void waitIfPaused();
    void applyThreadName() const;

    mutable std::mutex stateMutex_;
    std::condition_variable pauseCondition_;
    std::thread worker_;
    std::string name_;
    std::atomic<bool> running_;
    std::atomic<bool> stopRequested_;
    std::atomic<bool> paused_;
};

} // namespace core
} // namespace aplay

#endif // APLAY_CORE_THREAD_HPP
