#include <concepts>
#include <format>

// Ref: C++ Memory Management by Patrice Roy
namespace dev {

// Default deleter - single object version
template <typename T> struct default_deleter {
    void operator()(T* raw_ptr) {
        delete raw_ptr;
    }
};

// Default deleter - pointee is an array of objects version
template <typename T> struct default_deleter<T[]> {
    void operator()(T* raw_ptr) {
        delete[] raw_ptr;
    }
};

// Single object version
template <typename T, typename D = default_deleter<T>> class unique_ptr : public D {
  public:
    using deleter_type = D;

    // Default c'tor
    unique_ptr() = default;

    // Copy c'tor and copy assignment are delete'd.
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    // Parameteric constructor
    unique_ptr(T* ptr) : m_underlying_ptr{ptr} {}

    // swap function
    template <typename U, typename OtherDeleter>
        requires std::is_convertible_v<U, T>
    void swap(unique_ptr<U, OtherDeleter>& other) noexcept {
        std::swap(m_underlying_ptr, other.m_underlying_ptr);
    }

    /* Move constructor */
    unique_ptr(unique_ptr&& other)
        : m_underlying_ptr{std::exchange(other.m_underlying_ptr, nullptr)} {}

    /* Move assignment */
    unique_ptr& operator=(unique_ptr&& other) {
        std::swap(m_underlying_ptr, other.m_underlying_ptr);
        return *this;
    }

    /* Destructor */
    ~unique_ptr() {
        deleter_type* deleter = static_cast<deleter_type*>(this);
        (*deleter)(m_underlying_ptr);
    }

    // Pointer-like functions
    /* Dereferencing operator */
    T operator*() {
        return *m_underlying_ptr;
    }

    /* Indirection operator*/
    T* operator->() {
        return m_underlying_ptr;
    }

    /* get() - get the raw underlying pointer*/
    T* get() const {
        return m_underlying_ptr;
    }

    // Modifiers
    /* Release - returns a pointer to the managed object
       and releases the ownership*/
    T* release() {
        return std::exchange(m_underlying_ptr, nullptr);
    }

    /* Reset - Replaces the managed object */
    void reset(T* other) {
        if (m_underlying_ptr != other) {
            deleter_type* deleter = static_cast<deleter_type*>(this);
            (*deleter)(m_underlying_ptr);

            m_underlying_ptr = other;
        }
    }

    void reset(std::nullptr_t) {
        deleter_type* deleter = static_cast<deleter_type*>(this);
        (*deleter)(m_underlying_ptr);
        m_underlying_ptr = nullptr;
    }

    explicit operator bool() const {
        return (m_underlying_ptr == nullptr);
    }

    friend bool operator==(const unique_ptr& lhs, const unique_ptr& rhs) {
        return (lhs.get() == rhs.get());
    }

    friend bool operator==(const unique_ptr& lhs, std::nullptr_t rhs) {
        return (lhs.m_underlying_ptr == nullptr);
    }

    friend bool operator!=(unique_ptr& lhs, unique_ptr& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator!=(unique_ptr& lhs, std::nullptr_t) {
        return !(lhs == nullptr);
    }

    friend void swap(unique_ptr& lhs, unique_ptr& rhs) {
        lhs.swap(rhs);
    }

  private:
    T* m_underlying_ptr{nullptr};
};

/* Array version*/
template <typename T, typename D> class unique_ptr<T[], D> : public D {
  public:
    using deleter_type = D;

    /* Default c'tor */
    unique_ptr() = default;

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    /* Parameteric constructor */
    unique_ptr(T* ptr) : m_underlying_ptr{ptr} {}

    /* Swap function */
    void swap(unique_ptr& other) noexcept {
        std::swap(m_underlying_ptr, other.m_underlying_ptr);
    }

    /* Move constructor */
    unique_ptr(unique_ptr&& other) : m_underlying_ptr{std::move(other.m_underlying_ptr)} {}

    /* Move assignment */
    unique_ptr& operator=(unique_ptr&& other) {
        std::swap(m_underlying_ptr, other.m_underlying_ptr);
        return *this;
    }

    /* Destructor */
    ~unique_ptr() {
        deleter_type* deleter = static_cast<deleter_type*>(this);
        (*deleter)(m_underlying_ptr);
    }

    // Pointer-like functions
    /* Dereferencing operator */
    T operator*() {
        return *m_underlying_ptr;
    }

    /* Indirection operator*/
    T* operator->() {
        return m_underlying_ptr;
    }

    /* IndexOf operator - provides indexed access
       to the managed array.*/
    T operator[](std::size_t index) {
        return m_underlying_ptr[index];
    }

    /* get() - get the raw underlying pointer*/
    T* get() const {
        return m_underlying_ptr;
    }

    // Modifiers
    /* Release - returns a pointer to the managed object
       and releases the ownership*/
    T* release() {
        return std::exchange(m_underlying_ptr, nullptr);
    }

    /* Reset - Replaces the managed object */
    void reset(T* other) {
        if (m_underlying_ptr != other) {
            deleter_type* deleter = static_cast<deleter_type*>(this);
            (*deleter)(m_underlying_ptr);

            m_underlying_ptr = other;
        }
    }

    void reset(std::nullptr_t) {
        deleter_type* deleter = static_cast<deleter_type*>(this);
        (*deleter)(m_underlying_ptr);
        m_underlying_ptr = nullptr;
    }

    explicit operator bool() const {
        return (m_underlying_ptr == nullptr);
    }

    friend bool operator==(const unique_ptr& lhs, const unique_ptr& rhs) {
        return (lhs.get() == rhs.get());
    }

    friend bool operator==(const unique_ptr& lhs, std::nullptr_t rhs) {
        return (lhs.m_underlying_ptr == nullptr);
    }

    friend bool operator!=(unique_ptr& lhs, unique_ptr& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator!=(unique_ptr& lhs, std::nullptr_t) {
        return !(lhs == nullptr);
    }

    friend void swap(unique_ptr& lhs, unique_ptr& rhs) {
        lhs.swap(rhs);
    }

  private:
    T* m_underlying_ptr{nullptr};
};
} // namespace dev