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

template <typename T>
class vector;

template <typename T>
struct Iterator
{
    using iterator_concept = std::contiguous_iterator_tag;
    using difference_type  = std::ptrdiff_t;
    using value_type       = T;
    using reference        = T&;
    using pointer          = T*;
    using size_type        = std::size_t;
    friend vector<T>;
    friend Iterator<const T>;
    friend Iterator<std::remove_const_t<T>>;

    Iterator() = default;

    explicit Iterator(pointer ptr) : m_ptr(ptr)
    {
    }

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
        //                   const T *, if const_iterator
        //                   T*, otherwise
    }

    // compound addition
    Iterator& operator+=(size_type n)
    {
        m_ptr += n;
        return (*this);
    }

    // subtraction
    template <typename Self>
    auto operator-(this Self&& self, size_type n)
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

    [[nodiscard]] friend auto operator<=>(const Iterator& lhs, const Iterator& rhs)
    {
        return lhs.m_ptr <=> rhs.m_ptr;
    }
    // FIXME: I get build errors if I don't explicitly define operator== and operator!=
    // for Iterator<T>. This is unexpected and I would like to understand why it's the
    // case.
    // Comparison operators
    friend bool operator==(const Iterator& lhs, const Iterator& rhs)
    {
        return lhs.m_ptr == rhs.m_ptr;
    }

    friend bool operator!=(const Iterator& lhs, const Iterator& rhs)
    {
        return !(lhs == rhs);
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

template <typename T>
class vector
{
public:
    using value_type      = T;
    using size_type       = std::size_t;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using iterator        = Iterator<T>;
    using const_iterator  = Iterator<const T>;

private:
    pointer   m_elements{nullptr};
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

    // Cleanup helper function in case of copy ctor failure
    void cleanup_on_fail(Iterator<T> it)
    {
        auto p{begin()};
        for (; p != it; ++p)
            std::destroy_at(p.m_ptr);

        ::operator delete(m_elements);
        throw;
    }

    // This helper function accepts a user-supplied parameter `src`, which could be
    // a scalar value or a sequence of values with a pair of (begin(),end()) iterators.
    // It copy constructs T elements from `src` into raw memory underlying this vector.
    template <typename U>
    void copy_helper(const U& src, std::optional<size_t> opt_size)
    {
        auto i{begin()};
        if constexpr (std::is_same_v<U, T>)
        {
            // `src` is a scalar value
            try
            {
                for (; i != begin() + opt_size.value(); ++i)
                    std::construct_at(i.m_ptr, src);
            } catch (...)
            {
                cleanup_on_fail(i);
            }
        }
        else
        {
            // `src` is a range
            auto j{src.begin()};
            try
            {
                for (; j != src.end(); ++i, ++j)
                    std::construct_at(i.m_ptr, *j);
            } catch (...)
            {
                cleanup_on_fail(i);
            }
        }
    }

    // Helper function to allocate a raw memory of size new_capacity
    pointer allocate_new_storage(size_t new_capacity)
    {
        auto p = static_cast<pointer>(::operator new(sizeof(value_type) * new_capacity));
        return p;
    }

    // Helper function to copy old storage to a raw memory block,
    // during reallocation
    void copy_old_storage_to_new(pointer ptr_to_new_storage_block)
    {
        iterator d_first = iterator(ptr_to_new_storage_block);

        if constexpr (std::is_nothrow_move_constructible_v<T>)
        {
            std::uninitialized_move(begin(), end(), d_first);
        }
        else
        {
            try
            {
                std::uninitialized_copy(begin(), end(), d_first);
            } catch (...)
            {
                ::operator delete(ptr_to_new_storage_block);
                throw;
            }
        }
    }

    // Helper function to copy/move-construct `value`
    // into raw-memory pointed to by `ptr`
    template <typename U>
        requires std::is_convertible_v<U, T>
    void construct_at_addr(pointer ptr, U&& value)
    {
        if constexpr (std::is_nothrow_move_constructible_v<U>)
        {
            std::construct_at(ptr, std::move(value));
        }
        else
        {
            std::construct_at(ptr, value);
        }
    }

public:
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

    // Constructors
    // default constructor
    vector() = default;

    // parametrized constructor
    vector(size_type n, const_reference init) :
    m_elements(static_cast<pointer>(::operator new(n * sizeof(value_type)))),
    m_size{0},
    m_capacity{n}
    {
        copy_helper(init, n);
        m_size = n;
    }

    // copy constructor
    explicit vector(const vector& other) :
    m_elements(static_cast<pointer>(::operator new(other.size() * sizeof(value_type)))),
    m_size{other.m_size},
    m_capacity{other.m_size}
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
    vector(vector&& other) noexcept :
    m_elements(std::exchange(other.m_elements, nullptr)),
    m_capacity(std::exchange(other.m_capacity, 0)),
    m_size(std::exchange(other.m_size, 0))
    {
    }

    // move assignment
    vector& operator=(vector&& other) noexcept
    {
        vector(std::move(other)).swap(*this);
        return (*this);
    }

    // constructor that accepts a std::initializer_list<T>{}
    vector(std::initializer_list<T> src) :
    m_elements{static_cast<pointer>(::operator new(src.size() * sizeof(value_type)))},
    m_size{src.size()},
    m_capacity{src.size()}
    {
        copy_helper(src, std::nullopt);
    }

    // Destructor
    ~vector()
    {
        std::destroy(begin(), end());
        ::operator delete(m_elements);
    }

    // Element access member-functions
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

    template <typename Self>
    decltype(auto) at(this Self&& self, size_type n)
    {
        if (n >= 0 && n < self.m_size)
            return self.m_elements[n];
        //         ^------------------
        //         m_elements[n] is T const, if vector is const.
        //         m_elements[n] is T, otherwise.
        else
            throw std::out_of_range("Index out of bounds!");
    }

    // If pos < size is false, we get UB
    template <typename Self>
    decltype(auto) operator[](this Self&& self, size_type n)
    {
        return self.m_elements[n];
    }

    // front() : accesses the first element of the container
    template <typename Self>
    decltype(auto) front(this Self&& self)
    {
        return self.m_elements[0];
    }

    // back() : accesses the last element of the container
    template <typename Self>
    decltype(auto) back(this Self&& self)
    {
        return self.m_elements[self.m_size - 1];
    }

    // Modifiers
    template <typename U>
        requires std::is_convertible_v<U, T>
    void push_back(U&& value)
    {
        pointer p{nullptr};
        size_t  new_capacity = m_capacity ? 2 * m_capacity : 16;

        if (full())
        {
            // can throw, if allocation fails
            p = allocate_new_storage(new_capacity);
            try
            {
                // can throw if copy c'tor fails
                if constexpr (std::is_nothrow_move_constructible_v<U>)
                    std::construct_at(p + m_size, std::forward<U>(value));
                else
                    std::construct_at(p + m_size, value);
            } catch (...)
            {
                ::operator delete(p);
                throw;
            }

            copy_old_storage_to_new(p);

            // Deallocate old storage
            std::destroy(begin(), end());
            ::operator delete(m_elements);

            // Reassign m_elements and m_capacity
            m_elements = p;
            m_capacity = new_capacity;
            ++m_size;
        }
        else
        {
            std::construct_at(begin().get() + m_size, std::forward<U>(value));
            ++m_size;
        }
    }

    void pop_back()
    {
        std::destroy_at(end().get());
        --m_size;
    }

    template <typename... Args>
    reference emplace_back(Args&&... args)
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

    // Inserts the user-supplied value before position
    template <typename U>
        requires std::is_convertible_v<U, T>
    iterator insert(const_iterator position, U&& value)
    {
        // If a reallocation is triggered, all iterators are invalidated
        // and additionally value would also become a dangling reference,
        // if it refers to an existing element of the vector.
        size_type index = std::distance(begin(), iterator(position));
        auto      pos_  = iterator(position);

        if (full())
        {
            size_t new_capacity = m_capacity ? 2 * m_capacity : 16;
            auto   ptr_new_blk  = allocate_new_storage(new_capacity);

            try
            {
                construct_at_addr(ptr_new_blk + index, std::forward<U>(value));
            } catch (...)
            {
                ::operator delete(ptr_new_blk);
                throw;
            }

            // Copy/move elements from m_data[0..index-1] to ptr_new_blk[0..index-1]
            auto   p1{begin()};
            size_t i{0};

            try
            {
                for (; p1 != begin() + index; ++p1, ++i)
                {
                    construct_at_addr(ptr_new_blk + i, std::forward<U>(*p1));
                }
            } catch (...)
            {
                auto q{iterator(ptr_new_blk)};
                std::destroy(q, q + i);
                std::destroy_at(q.m_ptr + index);
                ::operator delete(ptr_new_blk);
                throw;
            }

            // Copy/move elements from m_data[index...] to ptr_new_blk[index+1...]
            auto   p2{begin() + index};
            size_t j{index + 1};

            try
            {
                for (; p2 != end(); ++p2, ++j)
                {
                    construct_at_addr(ptr_new_blk + j, std::forward<U>(*p2));
                }
            } catch (...)
            {
                auto q{ptr_new_blk};
                std::destroy(q, q + j);
                ::operator delete(ptr_new_blk);
                throw;
            }

            std::destroy(begin(), end());
            ::operator delete(m_elements);
            m_elements = ptr_new_blk;
            m_capacity = new_capacity;
            pos_       = begin() + index;
            ++m_size;
        }
        else
        {
            if constexpr (std::is_nothrow_move_constructible_v<T>)
            {
                std::uninitialized_move(end() - 1, end(), end());
                std::move_backward(pos_, end(), end());
                *pos_ = std::forward<U>(value);
                ++m_size;
            }
            else
            {
                std::uninitialized_copy(end() - 1, end(), end());
                std::copy_backward(pos_, end(), end());
                *pos_ = value;
                ++m_size;
            }
        }

        return pos_;
    }

    // We should be able to use pair of iterators from any container
    // as a source of values to insert, which is a very useful property
    // indeed.
    template <class InputIt>
    iterator insert(const_iterator position, InputIt first, InputIt last)
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

        size_t   src_len            = std::distance(first, last);
        size_t   num_elems_to_shift = std::distance(pos_, end());
        iterator d_first            = pos_ + src_len;
        iterator d_last             = end() + src_len;

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

        auto ptr_new_blk = allocate_new_storage(new_capacity);

        copy_old_storage_to_new(ptr_new_blk);

        std::destroy(begin(), end());
        ::operator delete(m_elements);
        m_elements = ptr_new_blk;
        m_capacity = new_capacity;
    }
};
} // namespace dev