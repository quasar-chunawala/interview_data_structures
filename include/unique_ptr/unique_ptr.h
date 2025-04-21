#include<format>
#include<concepts>

namespace dev{

    /* Default deleter - single object version */
    template<typename T>
    struct default_deleter{
        void operator()(T* raw_ptr){
            delete raw_ptr;
        }
    };

    /* Default deleter - pointee is an array of objects version */
    template<typename T>
    struct default_deleter<T[]>{
        void operator()(T* raw_ptr){
            delete[] raw_ptr;
        }
    };

    template<typename T, typename D = default_deleter<T>>
    class unique_ptr : D{
        public:
        using deleter_type = D;

        /* Default c'tor */
        unique_ptr() = default;

        unique_ptr(const unique_ptr& ) = delete;
        unique_ptr& operator=(const unique_ptr& ) = delete;

        /* Swap function */
        void swap(unique_ptr& other){
            std::swap(m_underlying_ptr, other.m_underlying_ptr);
        }

        /* Move constructor */
        unique_ptr(unique_ptr&& other)
        : m_underlying_ptr{ std::move(other.m_underlying_ptr)}
        {}

        /* Move assignment */
        unique_ptr& operator=(unique_ptr&& other){
            std::swap(m_underlying_ptr, other.m_underlying_ptr)
            return *this;
        }

        /* Destructor */
        ~unique_ptr(){
            *static_cast<deleter_type*>(this)(m_underlying_ptr);
        }

        // Pointer-like functions
        /* Dereferencing operator */
        T operator*(){
            return *m_underlying_ptr;
        }

        /* Indirection operator*/
        auto operator->(auto member){
            return m_underlying_ptr->member;
        }

        /* IndexOf operator - provides indexed access 
           to the managed array.*/
        T operator[](std::size_t index){
            return m_underlying_ptr[index];
        }

        // Modifiers
        /* Release - returns a pointer to the managed object
           and releases the ownership*/
        T* release(){
            return std::exchange(m_underlying_ptr, nullptr);
        }

        /* Reset - Replaces the managed object */
        


        private:
        T* m_underlying_ptr{nullptr};
    };
}