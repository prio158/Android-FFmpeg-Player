//
// Created by chen_zi_rui on 2024/4/14.
//

#include <optional>

template<class T>
class SafeQueueNoLock {
public:

    explicit SafeQueueNoLock(size_t capacity) : m_capacity(capacity), data(new T[capacity]) {}

    ~SafeQueueNoLock() {
        delete[] data;
    }

    bool push(const T &v) {
        if (size_.load(std::memory_order_relaxed) >= m_capacity) {
            return false;
        }
        new(data + (back % m_capacity)) T(v);
        ++back;
        size_.fetch_add(1, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {
        if (size_.load(std::memory_order_acquire) == 0) {
            return std::optional<T>(std::nullopt);
        }
        std::optional<T> res(std::move(data[front % m_capacity]));
        data[front % m_capacity].~T();
        ++front;
        size_.fetch_sub(1, std::memory_order_relaxed);
        return res;
    }


private:
    size_t m_capacity;
    T *const data;
    size_t back = 0;
    size_t front = 0;
    std::atomic<size_t> size_;


};
