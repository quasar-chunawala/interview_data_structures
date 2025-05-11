#include<cstddef>
#include<algorithm>
#include<initializer_list>
#include<iterator>
#include<type_traits>
#include<format>

namespace dev{
    template<typename T>
    struct Iterator{
        using iterator_concept = std::contiguous_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using size_type = std::size_t;
        
        Iterator() = default;

        Iterator(pointer ptr) : m_ptr(ptr) {}
        
        // pre-increment
        Iterator& operator++(){
            ++m_ptr;
            return (*this);
        }
        
        // post-increment
        Iterator operator++(int){
            auto temp = *this;
            ++m_ptr;
            return temp;
        }

        // pre-decrement
        Iterator& operator--(){
            --m_ptr;
            return (*this);
        }

        // post decrement
        Iterator operator--(int){
            auto temp = *this;
            --m_ptr;
            return (*this);
        }
        
        // addition
        Iterator operator+(size_type n){
            return iterator(m_ptr + n);
        }
        
        // compound addition
        Iterator& operator+=(size_type n){
            m_ptr += n;
            return (*this);
        }

        // subtraction
        Iterator operator-(size_type n){
            return iterator(m_ptr - n);
        }
        
        // compound subtraction
        Iterator& operator-=(size_type n){
            m_ptr -= n;
            return (*this);
        }

        // array subscript operator
        reference operator[](size_type i){
            return m_ptr[i];
        }
        
        // dereference operator
        reference operator*(){
            return *m_ptr;
        }
        
        // arrow operator
        pointer operator->(){
            return m_ptr;
        }

        // Distance between iterators
        difference_type operator-(const Iterator& other) const {
            return m_ptr - other.m_ptr;
        }

        // Equality comparison
        bool operator==(const Iterator& other) const {
            return m_ptr == other.m_ptr;
        }

        // Inequality comparison
        bool operator!=(const Iterator& other) const {
            return !(*this == m_ptr);
        }

        // Relational operators
        bool operator<(const Iterator& other) const {
            return m_ptr < other.m_ptr;
        }

        bool operator<=(const Iterator& other) const {
            return m_ptr <= other.m_ptr;
        }

        bool operator>(const Iterator& other) const {
            return m_ptr > other.m_ptr;
        }

        bool operator>=(const Iterator& other) const {
            return m_ptr >= other.m_ptr;
        }

        private:
        pointer m_ptr;
    };

    template<typename T>
    class vector{
        public:
            using value_type = T;
            using size_type = std::size_t;
            using pointer = T*;
            using const_pointer = const T*;
            using reference = T&;
            using const_reference = const T&;
            using iterator = Iterator<T>;
            using const_iterator = Iterator<const T>;

            // default constructor
            vector() = default;

            // parametrized constructor
            vector(size_type n, const_reference init)
            : m_elements{new value_type[n]()}
            , m_size{n}
            , m_capacity{n}
            {
                try{
                    std::fill(begin(),end(), init);
                }catch(...){
                    delete[] m_elements;
                    throw;
                }
            }

            // copy constructor
            vector(const vector& other)
            : m_elements{ new value_type[other.m_size]{}}
            , m_size{other.m_size}
            , m_capacity{other.m_size}
            {
                try{
                    std::copy(other.begin(), other.end(), begin());
                }catch(...){
                    delete[] m_elements;
                    throw;
                }
            }


            // swap function
            void swap(vector& other) noexcept{
                using std::swap;
                swap(m_elements, other.m_elements);
                swap(m_size, other.m_size);
                swap(m_capacity, other.m_capacity);
            }

            // copy assignment
            vector& operator=(const vector& other){
                vector(other).swap(*this);
                return (*this);
            }

            // move constructor
            vector(vector&& other) noexcept
            : m_elements(std::exchange(other.m_elements, nullptr))
            , m_capacity(std::exchange(other.m_capacity, 0))
            , m_size(std::exchange(other.m_size, 0))
            {}

            // move assignment
            vector& operator=(vector&& other) noexcept{
                vector(std::move(other)).swap(*this);
                return (*this);
            }

            // constructor that accepts a std::initializer_list<T>{}
            vector(std::initializer_list<T> src)
            : m_elements{new value_type[src.size()]}
            , m_size{ src.size()}
            , m_capacity{ src.size()}
            {
                try{
                    std::copy(src.begin(), src.end(), begin());
                }catch(...){
                    delete[] m_elements;
                    throw;
                }
            }

            // Destructor
            ~vector(){
                delete[] m_elements;
            }

            /* Element access member-functions */
            template<typename Self>
            decltype(auto) at(this Self&& self, size_type n){
                if(n >= 0 && n < self.m_size)
                    return self.m_elements[n];
                else
                    throw std::out_of_range("Index out of bounds!");
            }

            // If pos < size is false, we get UB
            template<typename Self>
            decltype(auto) operator[](this Self&& self, size_type n){
                return self.m_elements[n];
            }

            // front() : accesses the first element of the container
            template<typename Self>
            decltype(auto) front(this Self&& self){
                return self.m_elements[0];
            }

            // back() : accesses the last element of the container
            template<typename Self>
            decltype(auto) back(this Self&& self){
                return self.m_elements[self.m_size - 1];
            }

            template<typename Self>
            auto begin(this Self && self){
                return self.m_elements;
            }
            
            template<typename Self>
            auto end(this Self && self){
                return self.m_elements + self.size();
            }

            /* Capacity related member functions */
            size_type size() const{
                return m_size;
            }

            size_type capacity() const{
                return m_capacity;
            }

            bool empty() const{
                return m_size == 0;
            }

            /* Modifiers */
            // push_back(const T&) : appends a copy of the value
            // to the end of the container
            void push_back(const_reference value){
                if(full())
                    grow();

                m_elements[m_size] = value;
                ++m_size;
            }

            void push_back(T&& value){
                if(full())
                    grow();
                
                m_elements[m_size] = std::move(value);
                ++m_size;
            }

            void pop_back(){
                --m_size;
            }

            template<typename... Args>
            reference emplace_back(Args&&... args){
                if(full())
                    grow();

                m_elements[m_size] = value_type(std::forward<Args>(args)...);
                ++m_size;
                return back();
            }
            
            /* resize() - We want to allocate enough memory to cover the needs
            of the new capacity, copying or moving objects from the old memory block
            to the new memory block, then updating the address of the new memory block
            into `m_elements` pointer and updating the storage capacity.
            */
            void resize(size_type new_capacity){
                if(new_capacity <= capacity()) return;

                auto p = new T[new_capacity];
                if constexpr(std::is_nothrow_move_assignable_v<T>){
                    std::move(begin(), end(), p);
                }else{
                    try{
                        std::copy(begin(),end(),p);
                    }catch(...){
                        delete[] p;
                        throw;
                    }
                }

                delete[] m_elements;
                m_elements = p;
                m_capacity = new_capacity;
            }

            /* Inserts a copy of value before position */
            iterator insert(const_iterator position, const_reference value){
                if(full())
                    grow();
                
                size_type index = std::distance(begin(), position);
                std::copy_backward(position, end(), end() + 1);
                m_elements[index] = value;
                return iterator(m_elements[index]);
            }
            
            /* Inserts a value before position using move semantics */
            iterator insert(const_iterator position, T&& value){
                if(full())
                    grow();
                
                size_type index = std::distance(begin(), position);
                std::copy_backward(position, end(), end() + 1);
                m_elements[index] = std::move(value);
                return iterator(m_elements[index]);
            }

            /* 
            We should be able to use pair of iterators from any container
            as a source of values to insert, which is a very useful property
            indeed.
            */
            template<class InputIt>
            iterator insert(const_iterator position, InputIt first, InputIt last){
                iterator pos_ = const_cast<iterator>(position);
                const std::size_t remaining = capacity() - size();
                const std::size_t n = std::distance(first, last);
                if(remaining < n){
                    auto index = std::distance(begin(), pos_);
                    resize(capacity() + n - remaining);
                    pos_ = std::next(begin(), index);
                }

                std::copy_backward(pos_, end(), end() + n);
                std::copy(first, last, pos_);
                m_size += n;
                return pos_;
            }

            iterator erase(const_iterator pos){
                iterator pos_ = const_cast<iterator>(pos);
                if(pos_ == end()) return pos_;

                std::copy(std::next(pos_),end(),pos_);
                *std::prev(end()) = {};
                --m_size;
                return pos_;
            }
            private:
            pointer m_elements{nullptr};
            size_type m_size{0};
            size_type m_capacity{0};

            bool full(){
                return size() == capacity();
            }

            /* grow() - Used to dynamically grow the container when full*/
            void grow(){
                resize(capacity() ? capacity() * 2 : 16);
            }
    };
}


