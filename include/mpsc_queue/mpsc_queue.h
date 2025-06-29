#include <atomic>
#include <concepts>
#include <iostream>
#include <optional>
#include <thread>
#include <type_traits>
#include <vector>

//TODO: I am thinking about how to write the try_push() and try_pop() functions.
// This class is still begin designed.
namespace dev
{

template <typename T>
concept Queueable = std::default_initializable<T> && std::move_constructible<T>;

/**
 * @brief The `mpsc_queue` class provides a single-reader, multi-writer
 * fifo queue.
 */
template <Queueable T, std::size_t N>
class mpsc_queue
{
private:
    using size_type  = std::size_t;
    using value_type = T;
    using reference  = T&;

    static constexpr std::size_t m_capacity{1 << N};
    T                            m_buffer[m_capacity];
    alignas(std::hardware_destructive_interference_size) std::atomic<std::size_t> m_read_index{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<std::size_t> m_write_index{0};

public:
    mpsc_queue()                             = default;
    mpsc_queue(const mpsc_queue&)            = delete;
    mpsc_queue& operator=(const mpsc_queue&) = delete;
    mpsc_queue(mpsc_queue&&)                 = delete;
    mpsc_queue& operator=(mpsc_queue&&)      = delete;

    /**
     * @brief pushes an element onto the ringbuffer.
     * @param `element` will be pushed to the queue unless the queue is not full
     */
    template <typename U>
        requires std::is_convertible_v<T, U>
    bool try_push(U&& element)
    {
        std::size_t write_index = m_write_index.load(std::memory_order_acquire);

        do
        {
            if (write_index == (m_read_index.load(std::memory_order_acquire) + m_capacity))
                return false;

            std::size_t next_write_index             = (write_index + 1);
            m_buffer[write_index & (m_capacity - 1)] = std::forward<U>(element);
        } while (!m_write_index.compare_exchange_strong(write_index, next_write_index, std::memory_order_release))

            return true;
    }

    std::optional<T> try_pop()
    {
        std::optional<T>  result{std::nullopt};
        const std::size_t read_index = m_read_index.load(std::memory_order_relaxed);

        if (read_index == m_write_index.load(std::memory_order_acquire))
            return result;

        result = std::move(m_buffer[read_index]);
        m_read_index.store((read_index + 1) & (m_capacity - 1), std::memory_order_release);
        return result;
    }
};
} // namespace dev