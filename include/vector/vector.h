#include<cstddef>
#include<algorithm>
#include<initializer_list>
#include<iterator>
#include<type_traits>
#include<memory>
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
        template<typename Self>
        decltype(auto) operator+(this Self&& self, size_type n){
            return Iterator(self.m_ptr + n);
        }
        
        // compound addition
        Iterator& operator+=(size_type n){
            m_ptr += n;
            return (*this);
        }

        // subtraction
        template<typename Self>
        decltype(auto) operator-(this Self&& self, size_type n){
            return Iterator(self.m_ptr - n);
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
            return !(m_ptr == other.m_ptr);
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

        pointer get(){
            return m_ptr;
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

            template<typename Self>
            decltype(auto) begin(this Self && self){
                return Iterator(self.m_elements);
            }
            
            template<typename Self>
            decltype(auto) end(this Self && self){
                return Iterator(self.m_elements + self.size());
            }

            // default constructor
            vector() = default;

            // parametrized constructor
            vector(size_type n, const_reference init)
            : m_elements{static_cast<pointer>(std::malloc(n * sizeof(value_type)))}
            , m_size{n}
            , m_capacity{n}
            {
                auto p{begin()};
                try{
                    std::uninitialized_fill(begin(), end(), init);
                }catch(...){
                    std::free(m_elements);
                    throw;
                }
            }

            // copy constructor
            vector(const vector& other)
            : m_elements{ static_cast<pointer>(std::malloc(other.size() * sizeof(value_type))) }
            , m_size{other.m_size}
            , m_capacity{other.m_size}
            {
                try{
                    std::uninitialized_copy(other.begin(), other.end(), begin());
                }catch(...){
                    std::free(m_elements);
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
            : m_elements{ static_cast<pointer>(std::malloc(src.size() * sizeof(value_type))) }
            , m_size{ src.size()}
            , m_capacity{ src.size()}
            {
                try{
                    std::uninitialized_copy(src.begin(), src.end(), begin());
                }catch(...){
                    std::free(m_elements);
                    throw;
                }
            }

            // Destructor
            ~vector(){
                std::destroy(begin(), end());
                std::free(m_elements);
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

                std::construct_at(end().get(), value);
                ++m_size;
            }

            void push_back(T&& value){
                if(full())
                    grow();
                
                std::construct_at(end().get(), std::move(value));
                ++m_size;
            }

            void pop_back(){
                --m_size;
            }

            template<typename... Args>
            reference emplace_back(Args&&... args){
                if(full())
                    grow();

                std::construct_at(end().get(),std::forward<Args>(args)...);
                ++m_size;
                return back();
            }
            
            /* resize() - We want to allocate enough memory to cover the needs
            of the new capacity, copying or moving objects from the old memory block
            to the new memory block, then updating the address of the new memory block
            into `m_elements` pointer and updating the storage capacity.
            */
            void resize(size_type new_capacity){
                if(new_capacity == capacity()) return;

                if(new_capacity < capacity()){
                    std::destroy(begin() + new_capacity, end());
                    m_size = new_capacity;
                    return;
                }


                auto p = static_cast<pointer>(std::malloc(new_capacity * sizeof(value_type)));
                if constexpr(std::is_nothrow_move_assignable_v<T>){
                    std::uninitialized_move(begin(), end(), p);
                }else{
                    try{
                        std::uninitialized_copy(begin(),end(),p);
                    }catch(...){
                        std::free(p);
                        throw;
                    }
                }

                std::uninitialized_fill(p+size(), p+new_capacity, value_type{});
                std::destroy(begin(), end());
                std::free(m_elements);
                m_elements = p;
                m_size = new_capacity;
                m_capacity = new_capacity;
            }

            /* Inserts a copy of value before position */
            template<typename It>
            requires (std::is_same_v<It,iterator> || std::is_same_v<It,const_iterator>)       
            iterator insert(It position, const_reference value){
                if(full())
                    grow();
                
                auto pos_ = const_cast<pointer>(position.get());
                size_type index = std::distance(begin(), pos_);

                if constexpr(std::is_nothrow_move_constructible_v<T>)
                {
                    std::uninitialized_move(end() - 1, end(), end());
                    std::move_backward(pos_, end()-2, end());
                }
                else{
                    std::uninitialized_copy(end() - 1, end(), end());
                    std::copy_backward(pos_, end() - 2, end());
                }
                
                std::fill(pos_, pos_ + 1, value);
            }
            
            /* Inserts a value before position using move semantics */
            template<typename It>
            requires (std::is_same_v<It,iterator> || std::is_same_v<It,const_iterator>)
            iterator insert(It position, T&& value){
                if(full())
                    grow();
                
                auto pos_ = iterator(position.get());
                size_type index = std::distance(begin(), pos_);

                if constexpr(std::is_nothrow_move_constructible_v<T>)
                {
                    std::uninitialized_move(end() - 1, end(), end());
                    std::move_backward(pos_, end() - 2, end());
                }
                else{
                    std::uninitialized_copy(end() - 1, end(), end());
                    std::copy_backward(pos_, end() - 2, end());
                }
                
                std::fill(pos_, pos_ + 1, std::move(value));
                return pos_;
            }

            /* 
            We should be able to use pair of iterators from any container
            as a source of values to insert, which is a very useful property
            indeed.
            */
            template<class InputIt>
            iterator insert(const_iterator position, InputIt first, InputIt last){
                auto pos_ = iterator(position.get());
                const std::size_t remaining = capacity() - size();
                const std::size_t n = std::distance(first, last);
                if(remaining < n){
                    auto index = std::distance(begin(), pos_);
                    resize(capacity() + n - remaining);
                    pos_ = std::next(begin(), index);
                }

                
                /* 1. objects to be displaced from the [pos_,end()) sequence
                   into the raw memory.
                */
                auto num_items_to_displace_from_begin_end_into_raw_memory =
                std::min<std::ptrdiff_t>(end()-pos_,n);
                auto where_to_insert_items_from_begin_end_into_raw_memory = 
                end() + n - num_items_to_displace_from_begin_end_into_raw_memory;

                if constexpr(std::is_nothrow_move_constructible_v<T>)
                {
                        std::uninitialized_move(end()-num_items_to_displace_from_begin_end_into_raw_memory, end(), 
                        where_to_insert_items_from_begin_end_into_raw_memory);
                }
                else{
                        std::uninitialized_copy(end()-num_items_to_displace_from_begin_end_into_raw_memory, end(), 
                        where_to_insert_items_from_begin_end_into_raw_memory);
                }

               /* 2. objects to be inserted from the [first, last) seqeuence into
                  the raw memory.

                  The total number of items to be displaced = n. So, the number of 
                  objects from [first,last) to be inserted into raw memory, is
                  n - num_items_to_displace_from_begin_end_into_raw_memory
               */

                auto num_items_to_insert_from_first_last_into_raw_memory = 
                std::max<std::ptrdiff_t>(0,n - num_items_to_displace_from_begin_end_into_raw_memory );
                std::uninitialized_copy(last-num_items_to_insert_from_first_last_into_raw_memory, last, end());

                /* 3. objects to be copied/moved from [begin,end()) to the same space
                */
                auto num_items_to_backward_displace_from_begin_end = 
                std::max<ptrdiff_t>(0, end() - pos_ - num_items_to_displace_from_begin_end_into_raw_memory);
                if constexpr(std::is_nothrow_move_constructible_v<T>)
                {
                    std::move_backward(pos_, pos_ + num_items_to_backward_displace_from_begin_end, end());
                }
                else{
                    std::copy_backward(pos_, pos_ + num_items_to_backward_displace_from_begin_end, end());
                }

                /* 4. objects to be copied from [first,last) into the space [pos,end())
                */
               auto num_items_to_copy_from_first_last_into_begin_end = 
               std::max<ptrdiff_t>(0, n - num_items_to_insert_from_first_last_into_raw_memory);
               std::copy(first, first + num_items_to_copy_from_first_last_into_begin_end, pos_);

               m_size += n;
               return pos_;
            }

            template<typename It>
            requires (std::is_same_v<It,iterator> || std::is_same_v<It,const_iterator>)            
            iterator erase(It position){
                auto pos_ = iterator(position.get());
                if(pos_ == end()) return pos_;
                
                std::copy(std::next(pos_),end(),pos_);
                std::destroy_at(std::prev(end()).get());
                --m_size;
                return pos_;
            }

            void reserve(size_type new_capacity){
                if(new_capacity <= capacity()) return;

                auto p = static_cast<pointer>(std::malloc(new_capacity * sizeof(value_type)));

                if constexpr(std::is_nothrow_move_assignable_v<T>){
                    std::uninitialized_move(begin(), end(), p);
                }
                else try{
                    std::uninitialized_copy(begin(), end(), p);
                }catch(...){
                    std::free(p);
                    throw;
                }

                std::destroy(begin(), end());
                std::free(m_elements);
                m_elements = p;
                m_capacity = new_capacity;
            }
            private:
            pointer m_elements{nullptr};
            size_type m_size{0};
            size_type m_capacity{0};
            friend iterator;

            bool full(){
                return size() == capacity();
            }

            /* grow() - Used to dynamically grow the container when full*/
            void grow(){
                reserve(capacity() ? capacity() * 2 : 16);
            }
    };
}


