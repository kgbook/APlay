#ifndef APLAY_CORE_SINGLETON_HPP
#define APLAY_CORE_SINGLETON_HPP

namespace aplay {
namespace core {

template <typename T>
class Singleton {
public:
    static T& instance() {
        static T instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    Singleton() {}
    ~Singleton() {}
};

} // namespace core
} // namespace aplay

#endif // APLAY_CORE_SINGLETON_HPP
