#include <algorithm>
#include <cassert>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

// This design is in part inspired by the impl in
// C++ Memory Management by Patrice Roy, Packt, 2025.
// Compiler Explorer: https://godbolt.org/z/efKWcGsKx

namespace dev
{

template <typename T> class vector;

template <typename T> struct Iterator
{
    using iterator_concept = std::contiguous_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using size_type = std::size_t;
    friend vector<T>;
    friend Iterator<const T>;
    friend Iterator<std::remove_const_t<T>>;

    Iterator() = default;

    explicit Iterator(pointer ptr) : m_ptr(ptr) {}

    template <typename U>
        requires std::is_convertible_v<std::remove_const_t<U>*, T*>
    Iterator(const Iterator<U>& other) : m_ptr{const_cast<T*>(other.m_ptr)}
    {
    }

    // pre-increment
    Iterator& operator++()
    {
        ++m_ptr;
        return (*this);
    }

    // post-increment
    Iterator operator++(int)
    {
        auto temp = *this;
        ++m_ptr;
        return temp;
    }

    // pre-decrement
    Iterator& operator--()
    {
        --m_ptr;
        return (*this);
    }

    // post decrement
    Iterator operator--(int)
    {
        auto temp = *this;
        --m_ptr;
        return (*this);
    }

    // addition
    auto operator+(difference_type n)
    {
        return Iterator(m_ptr + n);
        //                  ^---------
        //                   const T *
        //  if we are working with a const_iterator
    }

    // compound addition
    Iterator& operator+=(size_type n)
    {
        m_ptr += n;
        return (*this);
    }

    // subtraction
    template <typename Self> decltype(auto) operator-(this Self&& self, size_type n)
    {
        return Iterator(self.m_ptr - n);
    }

    // compound subtraction
    Iterator& operator-=(size_type n)
    {
        m_ptr -= n;
        return (*this);
    }

    // array subscript operator
    reference operator[](size_type i)
    {
        return m_ptr[i];
    }

    // dereference operator
    reference operator*()
    {
        return *m_ptr;
    }

    // arrow operator
    pointer operator->()
    {
        return m_ptr;
    }

    // Distance between iterators
    difference_type operator-(const Iterator& other) const
    {
        return m_ptr - other.m_ptr;
    }

    [[nodiscard]] bool operator<=>(const Iterator& other) const
    {
        return m_ptr <=> other.m_ptr;
    }

    [[nodiscard]] bool operator==(const Iterator& other) const
    {
        return m_ptr == other.m_ptr;
    }

    [[nodiscard]] bool operator!=(const Iterator& other) const
    {
        return !(*this == other);
    }

    friend auto iter_move(Iterator it)
    {
        return std::move(*it);
    }

  private:
    pointer m_ptr;

    pointer get()
    {
        return m_ptr;
    }
};

template <typename T> class vector
{
  public:
    using value_type = T;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = Iterator<T>;
    using const_iterator = Iterator<T const>;

    // Capacity related member functions

    size_type size() const
    {
        return m_size;
    }

    size_type capacity() const
    {
        return m_capacity;
    }

    bool empty() const
    {
        return m_size == 0;
    }

  private:
    pointer m_elements{nullptr};
    size_type m_size{0};
    size_type m_capacity{0};
    friend iterator;

    bool full()
    {
        return size() == capacity();
    }

    // grow() - Used to dynamically grow the container when full
    void grow()
    {
        reserve(capacity() ? capacity() * 2 : 16);
    }

    // destroy helper in case copy construction fails
    void destroy_helper(Iterator<T> it)
    {
        auto p{begin()};
        for (; p != it; ++p)
            std::destroy_at(p.m_ptr);

        ::operator delete(m_elements);
        throw;
    }

    template <typename U> void copy_helper(const U& src, std::optional<size_t> opt_size)
    {
        auto i{begin()};
        if constexpr (std::is_same_v<U, T>)
        {
            try
            {
                for (; i != begin() + opt_size.value(); ++i)
                    std::construct_at(i.m_ptr, src);
            }
            catch (...)
            {
                destroy_helper(i);
            }
        }
        else
        {
            auto j{src.begin()};
            try
            {
                for (; j != src.end(); ++i, ++j)
                    std::construct_at(i.m_ptr, *j);
            }
            catch (...)
            {
                destroy_helper(i);
            }
        }
    }

    auto insert_helper(const_iterator position)
    {
        // Track the index of the position, where we'd like to insert
        // the user-supplied value. This is because, if a reallocation
        // is triggered, all iterators are invalidated.
        size_type index = std::distance(begin(), iterator(position));
        if (full())
            grow();

        auto pos_ = iterator(begin() + index);

        if constexpr (std::is_nothrow_move_constructible_v<T>)
        {
            std::uninitialized_move(end() - 1, end(), end());
            // std::move_backward(s_first,s_last,d_last)
            // Moves elements from the range [s_first,s_last) to the range
            // ending [...,d_last).
            std::move_backward(pos_, end() - 1, end());
        }
        else
        {
            std::uninitialized_copy(end() - 1, end(), end());
            std::copy_backward(pos_, end() - 1, end());
        }
        ++m_size;
        return pos_;
    }

  public:
    auto begin(this auto&& self)
    {
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
            return const_iterator(self.m_elements);
        else
            return iterator(self.m_elements);
    }

    auto end(this auto&& self)
    {
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
            return const_iterator(self.m_elements + self.m_size);
        else
            return iterator(self.m_elements + self.m_size);
    }

    // default constructor
    vector() = default;

    // parametrized constructor
    vector(size_type n, const_reference init)
        : m_elements(static_cast<pointer>(::operator new(n * sizeof(value_type)))), m_size{0},
          m_capacity{n}
    {
        copy_helper(init, n);
        m_size = n;
    }

    // copy constructor
    vector(const vector& other)
        : m_elements(static_cast<pointer>(::operator new(other.size() * sizeof(value_type)))),
          m_size{other.m_size}, m_capacity{other.m_size}
    {
        copy_helper(other, std::nullopt);
    }

    // swap function
    void swap(vector& other) noexcept
    {
        using std::swap;
        swap(m_elements, other.m_elements);
        swap(m_size, other.m_size);
        swap(m_capacity, other.m_capacity);
    }

    // copy assignment
    vector& operator=(const vector& other)
    {
        vector(other).swap(*this);
        return (*this);
    }

    // move constructor
    vector(vector&& other) noexcept
        : m_elements(std::exchange(other.m_elements, nullptr)),
          m_capacity(std::exchange(other.m_capacity, 0)), m_size(std::exchange(other.m_size, 0))
    {
    }

    // move assignment
    vector& operator=(vector&& other) noexcept
    {
        vector(std::move(other)).swap(*this);
        return (*this);
    }

    // constructor that accepts a std::initializer_list<T>{}
    vector(std::initializer_list<T> src)
        : m_elements{static_cast<pointer>(::operator new(src.size() * sizeof(value_type)))},
          m_size{src.size()}, m_capacity{src.size()}
    {
        copy_helper(src, std::nullopt);
    }

    // Destructor
    ~vector()
    {
        std::destroy(begin(), end());
        ::operator delete(m_elements);
    }

    /* Element access member-functions */
    template <typename Self> auto& at(this Self&& self, size_type n)
    {
        if (n >= 0 && n < self.m_size)
            return self.m_elements[n];
        //     ^------------------
        //      m_elements[n] on const vector becomes T const.
        else
            throw std::out_of_range("Index out of bounds!");
    }

    // If pos < size is false, we get UB
    template <typename Self> auto& operator[](this Self&& self, size_type n)
    {
        return self.m_elements[n];
    }

    // front() : accesses the first element of the container
    template <typename Self> auto& front(this Self&& self)
    {
        return self.m_elements[0];
    }

    // back() : accesses the last element of the container
    template <typename Self> auto& back(this Self&& self)
    {
        return self.m_elements[self.m_size - 1];
    }

    // Modifiers

    // push_back(const T&) : appends a copy of the value
    // to the end of the container
    void push_back(const_reference value)
    {
        if (full())
            grow();

        std::construct_at(end().get(), value);
        ++m_size;
    }

    void push_back(T&& value)
    {
        if (full())
            grow();

        std::construct_at(end().get(), std::move(value));
        ++m_size;
    }

    void pop_back()
    {
        std::destroy_at(end().get());
        --m_size;
    }

    template <typename... Args> reference emplace_back(Args&&... args)
    {
        if (full())
            grow();

        std::construct_at(end().get(), std::forward<Args>(args)...);
        ++m_size;
        return back();
    }

    // resize(size_t count) - Resize the container to contain count elements
    // - If count == current size, do nothing.
    // - If count < size(), the container is reduced to the first count elements
    // - If count > size(), additional default contructed elements of T() are
    // appended. If count > capacity, reallocation is triggered.
    // https://en.cppreference.com/w/cpp/container/vector/resize

    void resize(size_type new_size)
    {
        size_t current_size = size();
        if (new_size == current_size)
            return;

        if (new_size < current_size)
        {
            std::destroy(begin() + new_size, end());
            m_size = new_size;
            return;
        }

        if (new_size > capacity())
            reserve(new_size);

        // Default construct elements at indices [current_size,...,new_size-1]
        for (auto p{begin() + current_size}; p != begin() + new_size; ++p)
            std::construct_at(p.get(), value_type{});

        m_size = new_size;
    }

    // Inserts a copy of value before position
    iterator insert(const_iterator position, const_reference value)
    {
        iterator pos_ = insert_helper(position);
        *pos_ = value;
        return pos_;
    }

    // Inserts a value before position using move semantics
    iterator insert(const_iterator position, T&& value)
    {
        iterator pos_ = insert_helper(position);
        *pos_ = std::move(value);
        return pos_;
    }

    // We should be able to use pair of iterators from any container
    // as a source of values to insert, which is a very useful property
    // indeed.
    template <class InputIt> iterator insert(const_iterator position, InputIt first, InputIt last)
    {

        auto index = std::distance(begin(), iterator(position));

        // Algorithm.
        // ----------
        // 1. Determine if the elements in the range [first,last) can
        //    fit into the remaining_capacity = capacity() - size().
        //    If not, a reallocation is triggered.

        // Possible reallocation
        if (std::distance(first, last) > capacity() - size())
        {
            size_t new_capacity = size() + std::distance(first, last);
            reserve(new_capacity);
        }

        iterator pos_ = begin() + index;

        //             num_elems_to_shift
        // 2.            |<--------->|
        //   begin()     position    end()                               capacity
        //   |           |           |
        //    ===========================================================
        //   |42 |5  |17 |28 |63 |55 |   |   |   |   |   |   |   |   |   |
        //    ===========================================================
        //                           ^-----------------------------------^
        //                                       Raw Storage

        size_t src_len = std::distance(first, last);
        size_t num_elems_to_shift = std::distance(pos_, end());
        iterator d_first = pos_ + src_len;
        iterator d_last = end() + src_len;

        if (src_len >= num_elems_to_shift)
        {
            // a) If 3 or more elements have to be inserted at pos_,
            //  then the range [position,end) has to be moved/copied to
            //  raw storage.
            if constexpr (std::is_nothrow_move_constructible_v<T>)
                std::uninitialized_move(pos_, end(), d_first);
            else
                std::uninitialized_copy(pos_, end(), d_first);
        }
        else
        {
            // b) If less than 3 elements have to be inserted at pos_,
            // then
            //   => a subsequence [end() - src_len, end()) has to be copied/moved
            //      to raw storage.
            //   => the subsequence [pos_,end() - src_len) has to be copied/moved
            //      to initialized storage.
            if constexpr (std::is_nothrow_move_constructible_v<T>)
            {
                std::uninitialized_move(end() - src_len, end(), d_last - src_len);
                std::move_backward(pos_, end() - src_len, end());
            }
            else
            {
                std::uninitialized_copy(end() - src_len, end(), d_last - src_len);
                std::copy_backward(pos_, end() - src_len, end());
            }
        }

        // 3. Copy/move elements from src to dest range.
        if (src_len <= num_elems_to_shift)
        {
            // a) Copy/move the elements from the source range to [pos_,pos+src_len)
            if constexpr (std::is_nothrow_move_constructible_v<T>)
            {
                std::move(first, last, pos_);
            }
            else
            {
                std::copy(first, last, pos_);
            }
        }
        else
        {
            // b) (i) Copy/move the elements from
            //     the subsequence [first,first + num_elems_to_shift)
            //     to [pos_,end())
            if constexpr (std::is_nothrow_move_constructible_v<T>)
            {
                std::move(first, first + num_elems_to_shift, pos_);
            }
            else
            {
                std::copy(first, first + num_elems_to_shift, pos_);
            }
            // (ii) Copy/move the elements from [first+num_elems_to_shift,last)
            //      to uninitialized storage [end(),pos_ + src_len)
            if constexpr (std::is_nothrow_move_constructible_v<T>)
            {
                std::uninitialized_move(first + num_elems_to_shift, last, end());
            }
            else
            {
                std::uninitialized_copy(first + num_elems_to_shift, last, end());
            }
        }

        m_size += src_len;
        return pos_;
    }

    template <typename It>
        requires(std::is_same_v<It, iterator> || std::is_same_v<It, const_iterator>)
    iterator erase(It position)
    {
        auto pos_ = iterator(position.get());
        if (pos_ == end())
            return pos_;

        std::copy(std::next(pos_), end(), pos_);
        std::destroy_at(std::prev(end()).get());
        --m_size;
        return pos_;
    }

    void reserve(size_type new_capacity)
    {
        if (new_capacity <= capacity())
            return;

        auto p = static_cast<pointer>(::operator new(sizeof(value_type) * new_capacity));
        iterator d_first = iterator(p);

        if constexpr (std::is_nothrow_move_assignable_v<T>)
        {
            std::uninitialized_move(begin(), end(), d_first);
        }
        else
            try
            {
                std::uninitialized_copy(begin(), end(), d_first);
            }
            catch (...)
            {
                ::operator delete(p);
                throw;
            }

        std::destroy(begin(), end());
        ::operator delete(m_elements);
        m_elements = p;
        m_capacity = new_capacity;
    }
};
} // namespace dev