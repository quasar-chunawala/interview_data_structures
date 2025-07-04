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

// Compiler Explorer: https://godbolt.org/z/z5oYqjdv5
namespace dev {

/**
 * @brief A standard container that offers constant-time
 * access to individual elements in any order.
 */
template<typename T>
class vector;

/**
 * @brief @a vector::Iterator<T> satisfies the contiguous iterator
 * concept. It is a light wrapper over a pointer-to-T. With random
 * access iterators, you can jump around your collection in
 * arbitrary-sized steps in constant time. it += n is a constant-time
 * operation. Contiguous iterators provide the additional guarantee
 * that, the elements of the collection are stored contiguously in
 * memory. it + n == std::to_address(*it) + n.
 */
template<typename T>
struct Iterator
{
    using iterator_concept = std::contiguous_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using pointer_to_const = const T*;
    using size_type = std::size_t;
    friend vector<T>;
    friend Iterator<const T>;
    friend Iterator<std::remove_const_t<T>>;

    /**
     * @brief Default constructor for Iterator.
     */
    Iterator()
      : m_ptr{ nullptr }
    {
    }

    /**
     * @brief Constructs an Iterator from a raw pointer.
     * @param ptr Pointer to the element.
     */
    explicit Iterator(pointer ptr)
      : m_ptr(ptr)
    {
    }

    /**
     * @brief Constructs an Iterator from another Iterator.
     * @tparam U The type of the other Iterator.
     * @param other The other Iterator to copy from.
     */
    template<typename U>
        requires std::is_convertible_v<std::remove_const_t<U>*, T*>
    Iterator(const Iterator<U>& other)
      : m_ptr{ const_cast<T*>(other.m_ptr) }
    {
    }

    /**
     * @brief Pre-increment operator.
     * @return Reference to the incremented Iterator.
     */
    Iterator& operator++()
    {
        ++m_ptr;
        return *this;
    }

    /**
     * @brief Post-increment operator.
     * @return Copy of the Iterator before incrementing.
     */
    Iterator operator++(int)
    {
        auto temp = *this;
        ++m_ptr;
        return temp;
    }

    /**
     * @brief Pre-decrement operator.
     * @return Reference to the decremented Iterator.
     */
    Iterator& operator--()
    {
        --m_ptr;
        return *this;
    }

    /**
     * @brief Post-decrement operator.
     * @return Copy of the Iterator before decrementing.
     */
    Iterator operator--(int)
    {
        auto temp = *this;
        --m_ptr;
        return (*this);
    }

    /**
     * @brief Adds an offset to the Iterator.
     * @param n The offset to add.
     * @return A new Iterator pointing to the new position.
     */
    auto operator+(difference_type n) const { return Iterator(m_ptr + n); }
    friend auto operator+(difference_type n, Iterator j) { return (j + n); }

    /**
     * @brief Adds an offset to the Iterator and assigns it.
     * @param n The offset to add.
     * @return Reference to the updated Iterator.
     */
    Iterator& operator+=(size_type n)
    {
        m_ptr += n;
        return *this;
    }

    /**
     * @brief Subtracts an offset from the Iterator.
     * @param n The offset to subtract.
     * @return A new Iterator pointing to the new position.
     */
    auto operator-(size_type n) const { return Iterator(m_ptr - n); }
    friend auto operator-(difference_type n, Iterator j) { return (j - n); }

    /**
     * @brief Subtracts an offset from the Iterator and assigns it.
     * @param n The offset to subtract.
     * @return Reference to the updated Iterator.
     */
    Iterator& operator-=(size_type n)
    {
        m_ptr -= n;
        return *this;
    }

    /**
     * @brief Accesses the element at the given index.
     * @param i The index to access.
     * @return Reference to the element at the index.
     */
    reference operator[](size_type i) const { return m_ptr[i]; }

    /**
     * @brief Dereferences the Iterator.
     * @return Reference to the element pointed to by the Iterator.
     */
    reference operator*() { return *m_ptr; }
    const_reference operator*() const { return *m_ptr; }

    /**
     * @brief Accesses the member of the element pointed to by the Iterator.
     * @return Pointer to the element.
     */
    pointer operator->() { return m_ptr; }
    pointer_to_const operator->() const { return m_ptr; }

    /**
     * @brief Computes the distance between two Iterators.
     * @param other The other Iterator.
     * @return The distance between the two Iterators.
     */
    difference_type operator-(const Iterator& other) const { return m_ptr - other.m_ptr; }

    /**
     * @brief Compares two Iterators using the spaceship operator.
     * @param lhs The left-hand side Iterator.
     * @param rhs The right-hand side Iterator.
     * @return Result of the comparison.
     */
    [[nodiscard]] friend auto operator<=>(const Iterator& lhs, const Iterator& rhs)
    {
        return lhs.m_ptr <=> rhs.m_ptr;
    }

    /**
     * @brief Checks if two Iterators are equal.
     * @param lhs The left-hand side Iterator.
     * @param rhs The right-hand side Iterator.
     * @return True if the Iterators are equal, false otherwise.
     */
    friend bool operator==(const Iterator& lhs, const Iterator& rhs)
    {
        return lhs.m_ptr == rhs.m_ptr;
    }

    /**
     * @brief Moves the element pointed to by the Iterator.
     * @param it The Iterator.
     * @return The moved element.
     */
    friend auto&& iter_move(Iterator it) { return std::move(*it); }

  private:
    pointer m_ptr;
    pointer get() { return m_ptr; }
};

template<typename T>
class vector
{
  public:
    using value_type = T;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;

  private:
    pointer m_elements{ nullptr };
    size_type m_size{ 0 };
    size_type m_capacity{ 0 };
    friend iterator;

    /**
     * @brief Checks if the container is full.
     * @return True if the container is full, false otherwise.
     */
    [[nodiscard]] bool full() { return size() == capacity(); }

    /**
     * @brief Grows the container to twice its current capacity.
     */
    void grow() { reserve(capacity() ? capacity() * 2 : 16); }

    /**
     * @brief Cleans up the range [begin(), last) in case of an exception.
     * @param last The end of the range to clean up.
     */
    void cleanup_on_fail_aux(Iterator<T> last)
    {
        auto p{ begin() };
        for (; p != last; ++p)
            std::destroy_at(p.m_ptr);

        ::operator delete(m_elements);
        throw;
    }

    /**
     * @brief Fills the raw memory block %[begin(), begin() + size) with %n copies
     * of %init.
     */
    template<typename U>
        requires std::is_convertible_v<U, T>
    void copy_aux(const U& init, size_t size)
    {
        auto i{ begin() };
        try {
            for (; i != begin() + size; ++i)
                std::construct_at(i.m_ptr, init);
        } catch (...) {
            cleanup_on_fail_aux(i);
        }
    }

    /**
     * @brief Copies values from the range %[first,last)
     * to %[begin(),begin()+std::distance(first,last)).
     */
    template<typename It>
    void copy_rng_aux(It first, It last)
    {
        static_assert(std::forward_iterator<decltype(first)>);
        static_assert(std::forward_iterator<decltype(last)>);
        auto i{ begin() };
        auto j{ first };
        try {
            for (; j != last; ++i, ++j)
                std::construct_at(i.m_ptr, *j);
        } catch (...) {
            cleanup_on_fail_aux(i);
        }
    }

    /**
     * @brief Allocates raw memory for the container.
     * @param new_capacity The new capacity of the container.
     * @return Pointer to the allocated memory.
     */
    pointer allocate_aux(size_t new_capacity)
    {
        auto p = static_cast<pointer>(::operator new(sizeof(value_type) * new_capacity));
        return p;
    }

    /**
     * @brief Copies elements from old storage to new.
     * @param ptr_to_new_storage_block Pointer to the new storage block.
     */
    void copy_old_storage_to_new(pointer ptr_to_new_storage_block)
    {
        iterator d_first = iterator(ptr_to_new_storage_block);

        if constexpr (std::is_nothrow_move_constructible_v<T>) {
            std::uninitialized_move(begin(), end(), d_first);
        } else {
            try {
                std::uninitialized_copy(begin(), end(), d_first);
            } catch (...) {
                ::operator delete(ptr_to_new_storage_block);
                throw;
            }
        }
    }

    /**
     * @brief helper function to copy/move-construct %value
     * into raw-memory pointed to by %ptr.
     */
    template<typename U>
        requires std::is_convertible_v<U, T>
    void construct_at_addr(pointer ptr, U&& value)
    {
        if constexpr (std::is_nothrow_move_constructible_v<U>) {
            std::construct_at(ptr, std::move(value));
        } else {
            std::construct_at(ptr, value);
        }
    }

  public:
    // Capacity related member functions
    /**
     * @brief Returns the number of elements in the vector
     */
    [[nodiscard]] size_type size() const { return m_size; }

    /**
     * @brief Returns the total number of elements that the vector can hold.
     */
    [[nodiscard]] size_type capacity() const { return m_capacity; }

    /**
     * @brief Returns true if the vector is empty.
     */
    bool empty() const { return m_size == 0; }

    // Constructors
    /**
     * @brief Creates a vector with no elements.
     */
    vector() = default;

    /**
     * @brief Creates a vector with @a n copies of an element.
     * @param n Number of elements to initially create.
     * @param init An element to copy.
     */
    vector(size_type n, const_reference init)
      : m_elements(allocate_aux(n))
      , m_size{ 0 }
      , m_capacity{ n }
    {
        copy_aux(init, n);
        m_size = n;
    }

    /**
     * @brief vector copy constructor
     * @param other A vector of identical element type %T
     */
    explicit vector(const vector& other)
      : m_elements(allocate_aux(other.m_size))
      , m_size{ other.m_size }
      , m_capacity{ other.m_size }
    {
        copy_rng_aux(other.begin(), other.end());
    }

    /**
     * @brief swaps data with another vector.
     * The global %std::swap function is specialized
     * such that %std::swap(v1,v2) will feed to this function.
     */
    void swap(vector& other) noexcept
    {
        using std::swap;
        swap(m_elements, other.m_elements);
        swap(m_size, other.m_size);
        swap(m_capacity, other.m_capacity);
    }

    /**
     * @brief vector copy assignment operator
     */
    vector& operator=(const vector& other)
    {
        vector(other).swap(*this);
        return (*this);
    }

    /**
     * @brief vector move constructor
     * The newly-created vector contains the exact contents of %other.
     * The contents of %other are a valid, but unspecified.
     */
    vector(vector&& other) noexcept
      : m_elements(std::exchange(other.m_elements, nullptr))
      , m_capacity(std::exchange(other.m_capacity, 0))
      , m_size(std::exchange(other.m_size, 0))
    {
    }

    /**
     * @brief vector move assignment operator
     * The contents of %other are moved into this vector(without copying).
     */
    vector& operator=(vector&& other) noexcept
    {
        vector(std::move(other)).swap(*this);
        return (*this);
    }

    /**
     * @brief This constructor fills a vector with copies of
     * the elements in the initializer list
     */
    vector(std::initializer_list<T> src)
      : m_elements{ allocate_aux(src.size()) }
      , m_size{ src.size() }
      , m_capacity{ src.size() }
    {
        copy_rng_aux(src.begin(), src.end());
    }

    /**
     * @brief Destructor
     */
    ~vector()
    {
        std::destroy(begin(), end());
        ::operator delete(m_elements);
    }

    /**
     * @brief Returns a read/write iterator that points to the first element
     * of the vector.
     */
    auto begin(this auto&& self)
    {
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
            return const_iterator(self.m_elements);
        else
            return iterator(self.m_elements);
    }

    /**
     * @brief Returns a read/write iterator that points to one past the last
     * element of the vector.
     */
    auto end(this auto&& self)
    {
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
            return const_iterator(self.m_elements + self.m_size);
        else
            return iterator(self.m_elements + self.m_size);
    }

    /**
     * @brief Read/write reference to the data at index n
     */
    reference at(size_type n)
    {
        if (n >= 0 && n < m_size)
            return m_elements[n];
        else
            throw std::out_of_range("Index out of bounds!");
    }

    /**
     * @brief Read-only reference to the data at index n
     */
    const_reference at(size_type n) const
    {
        if (n >= 0 && n < m_size)
            return m_elements[n];
        else
            throw std::out_of_range("Index out of bounds!");
    }

    /**
     * @brief Array subscript operator.
     * Returns a read/write reference to the element at index %n.
     */
    reference operator[](size_type n) { return m_elements[n]; }

    /**
     * @brief Array subscript operator.
     * Returns a read-only reference to the element at index %n.
     */
    const_reference operator[](size_type n) const { return m_elements[n]; }

    /**
     * @brief Accesses the first element of the container
     */
    template<typename Self>
    decltype(auto) front(this Self&& self)
    {
        return self.m_elements[0];
    }

    /**
     * @brief Accesses the last element of the container
     */
    template<typename Self>
    decltype(auto) back(this Self&& self)
    {
        return self.m_elements[self.m_size - 1];
    }

    // Modifiers
    /**
     * @brief Add data to the end of the vector
     */
    template<typename U>
        requires std::is_convertible_v<U, T>
    void push_back(U&& value)
    {
        pointer p{ nullptr };
        size_t new_capacity = m_capacity ? 2 * m_capacity : 16;

        if (full()) {
            // can throw, if allocation fails
            p = allocate_aux(new_capacity);
            try {
                // can throw if copy c'tor fails
                construct_at_addr(p + m_size, std::forward<U>(value));
            } catch (...) {
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
        } else {
            std::construct_at(begin().get() + m_size, std::forward<U>(value));
            ++m_size;
        }
    }

    /**
     * @brief Removes the last element
     */
    void pop_back()
    {
        std::destroy_at(end().get());
        --m_size;
    }

    /**
     * @brief Construct an element in-place at the end of the
     * vector, using constructor args.
     */
    template<typename... Args>
    reference emplace_back(Args&&... args)
    {
        if (full())
            grow();

        std::construct_at(end().get(), std::forward<Args>(args)...);
        ++m_size;
        return back();
    }

    /**
     * @brief Resize the container to contain %new_size elements.
     * - If count == current size, do nothing.
     * - If count < size(), the container is reduced to the first
     * count elements
     * - If count > size(), additional default contructed elements of
     * T() are appended. If count > capacity, reallocation is triggered.
     * https://en.cppreference.com/w/cpp/container/vector/resize
     */
    void resize(size_type new_size)
    {
        size_t current_size = size();
        if (new_size == current_size)
            return;

        if (new_size < current_size) {
            std::destroy(begin() + new_size, end());
            m_size = new_size;
            return;
        }

        if (new_size > capacity())
            reserve(new_size);

        // Default construct elements at indices
        // [current_size,...,new_size-1]
        for (auto p{ begin() + current_size }; p != begin() + new_size; ++p)
            std::construct_at(p.get(), value_type{});

        m_size = new_size;
    }

    /**
     * @brief Inserts given value into vector before specified iterator.
     * Note that this kind of operation could be expensive for a vector
     * and if it is frequently used, it can trigger reallocation.
     * The user should consider using std::list.
     */
    template<typename U>
        requires std::is_convertible_v<U, T>
    iterator insert(const_iterator position, U&& value)
    {
        // If a reallocation is triggered, all iterators are
        // invalidated and additionally `value` would also become a
        // dangling reference, if it refers to an existing element of
        // the vector.
        size_type index = std::distance(begin(), iterator(position));
        auto pos_ = iterator(position);

        if (full()) {
            size_t new_capacity = m_capacity ? 2 * m_capacity : 16;
            auto ptr_new_blk = allocate_aux(new_capacity);

            try {
                construct_at_addr(ptr_new_blk + index, std::forward<U>(value));
            } catch (...) {
                ::operator delete(ptr_new_blk);
                throw;
            }

            // Copy/move elements from m_data[0..index-1] to
            // ptr_new_blk[0..index-1]
            auto p1{ begin() };
            size_t i{ 0 };

            try {
                for (; p1 != begin() + index; ++p1, ++i) {
                    construct_at_addr(ptr_new_blk + i, std::forward<U>(*p1));
                }
            } catch (...) {
                auto q{ iterator(ptr_new_blk) };
                std::destroy(q, q + i);
                std::destroy_at(q.m_ptr + index);
                ::operator delete(ptr_new_blk);
                throw;
            }

            // Copy/move elements from m_data[index...] to
            // ptr_new_blk[index+1...]
            auto p2{ begin() + index };
            size_t j{ index + 1 };

            try {
                for (; p2 != end(); ++p2, ++j) {
                    construct_at_addr(ptr_new_blk + j, std::forward<U>(*p2));
                }
            } catch (...) {
                auto q{ ptr_new_blk };
                std::destroy(q, q + j);
                ::operator delete(ptr_new_blk);
                throw;
            }

            std::destroy(begin(), end());
            ::operator delete(m_elements);
            m_elements = ptr_new_blk;
            m_capacity = new_capacity;
            pos_ = begin() + index;
            ++m_size;
        } else {
            if constexpr (std::is_nothrow_move_constructible_v<T>) {
                std::uninitialized_move(end() - 1, end(), end());
                std::move_backward(pos_, end(), end());
                *pos_ = std::forward<U>(value);
                ++m_size;
            } else {
                std::uninitialized_copy(end() - 1, end(), end());
                std::copy_backward(pos_, end(), end());
                *pos_ = value;
                ++m_size;
            }
        }

        return pos_;
    }

    /**
     * @brief Inserts a range of elements into the vector before the specified position.
     *
     * This function inserts elements from the range `[first, last)` into the vector
     * before the location specified by `position`. If the range cannot fit into the
     * remaining capacity, a reallocation is triggered.
     *
     * @tparam InputIt The type of the input iterator. Must satisfy the requirements
     * of `std::input_iterator`.
     * @param position The position before which the elements will be inserted.
     * @param first The beginning of the range to insert.
     * @param last The end of the range to insert.
     * @return An iterator pointing to the first of the newly inserted elements.
     *
     * @note If reallocation occurs, all iterators, references, and pointers to
     * elements in the vector are invalidated.
     *
     * @throws std::bad_alloc If memory allocation fails during reallocation.
     * @throws Any exception thrown by the copy c'tor of the element type T.
     */
    template<class InputIt>
    iterator insert(const_iterator position, InputIt first, InputIt last)
    {
        auto index = std::distance(begin(), iterator(position));

        // Algorithm.
        // ----------
        // 1. Determine if the elements in the range [first,last) can
        //    fit into the remaining_capacity = capacity() - size().
        //    If not, a reallocation is triggered.

        // Possible reallocation
        if (std::distance(first, last) > capacity() - size()) {
            size_t new_capacity = size() + std::distance(first, last);
            reserve(new_capacity);
        }

        iterator pos_ = begin() + index;

        //             num_elems_to_shift
        // 2.            |<--------->|
        //   begin()     position    end() capacity |           | |
        //    ===========================================================
        //   |42 |5  |17 |28 |63 |55 |   |   |   |   |   |   |   |   |
        //   |
        //    ===========================================================
        //                           ^-----------------------------------^
        //                                       Raw Storage

        size_t src_len = std::distance(first, last);
        size_t num_elems_to_shift = std::distance(pos_, end());
        iterator d_first = pos_ + src_len;
        iterator d_last = end() + src_len;

        if (src_len >= num_elems_to_shift) {
            // a) If 3 or more elements have to be inserted at pos_,
            //  then the range [position,end) has to be moved/copied
            //  to raw storage.
            if constexpr (std::is_nothrow_move_constructible_v<T>)
                std::uninitialized_move(pos_, end(), d_first);
            else
                std::uninitialized_copy(pos_, end(), d_first);
        } else {
            // b) If less than 3 elements have to be inserted at pos_,
            // then
            //   => a subsequence [end() - src_len, end()) has to be
            //   copied/moved
            //      to raw storage.
            //   => the subsequence [pos_,end() - src_len) has to be
            //   copied/moved
            //      to initialized storage.
            if constexpr (std::is_nothrow_move_constructible_v<T>) {
                std::uninitialized_move(end() - src_len, end(), d_last - src_len);
                std::move_backward(pos_, end() - src_len, end());
            } else {
                std::uninitialized_copy(end() - src_len, end(), d_last - src_len);
                std::copy_backward(pos_, end() - src_len, end());
            }
        }

        // 3. Copy/move elements from src to dest range.
        if (src_len <= num_elems_to_shift) {
            // a) Copy/move the elements from the source range to
            // [pos_,pos+src_len)
            if constexpr (std::is_nothrow_move_constructible_v<T>) {
                std::move(first, last, pos_);
            } else {
                std::copy(first, last, pos_);
            }
        } else {
            // b) (i) Copy/move the elements from
            //     the subsequence [first,first + num_elems_to_shift)
            //     to [pos_,end())
            if constexpr (std::is_nothrow_move_constructible_v<T>) {
                std::move(first, first + num_elems_to_shift, pos_);
            } else {
                std::copy(first, first + num_elems_to_shift, pos_);
            }
            // (ii) Copy/move the elements from
            // [first+num_elems_to_shift,last)
            //      to uninitialized storage [end(),pos_ + src_len)
            if constexpr (std::is_nothrow_move_constructible_v<T>) {
                std::uninitialized_move(first + num_elems_to_shift, last, end());
            } else {
                std::uninitialized_copy(first + num_elems_to_shift, last, end());
            }
        }

        m_size += src_len;
        return pos_;
    }

    // TODO: Add an implementation of insert(const_iterator position, InputIt first,
    // InputIt last) which handles the special case where [first,last) are a subrange in
    // %this vector.

    template<typename It>
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

        auto ptr_new_blk = allocate_aux(new_capacity);

        copy_old_storage_to_new(ptr_new_blk);

        std::destroy(begin(), end());
        ::operator delete(m_elements);
        m_elements = ptr_new_blk;
        m_capacity = new_capacity;
    }
};

// static_assert(std::contiguous_iterator<Iterator<int>>);
// static_assert(std::contiguous_iterator<Iterator<const int>>);

} // namespace dev