#include<format>
#include<atomic>

namespace dev{
    template<typename T>
    class shared_ptr{
        public:

        /* Default constructor*/
        shared_ptr() = default;

        /* Parametrized constructor : Takes ownership of the pointee */
        shared_ptr(T* ptr) 
        : m_raw_underlying_ptr{ptr}
        {
            try{
                m_ref_count_ptr = new std::atomic<unsigned long long>(1LL);
            }catch(...){
                delete ptr;
                throw;
            }
        }

        /* Copy constructor : Implements shared co-ownership of the pointee semantics */
        shared_ptr(const shared_ptr& other)
        : m_raw_underlying_ptr{other.m_raw_underlying_ptr}
        , m_ref_count_ptr{other.m_ref_count_ptr}
        {
            if(m_ref_count_ptr)
                ++(*m_ref_count_ptr);   //Atomic pre-increment
        }

        /* Move constructor : Represents the transfer of ownership */
        shared_ptr(shared_ptr&& other)
        : m_raw_underlying_ptr{ std::exchange(other.m_raw_underlying_ptr, nullptr)}
        , m_ref_count_ptr{ std::exchange(other.m_ref_count_ptr, nullptr)}
        {}

        /* Swap : Swap two shared_ptr objects member by member */
        void swap(shared_ptr& other){
            using std::swap;
            std::swap(m_raw_underlying_ptr, other.m_raw_underlying_ptr);
            std::swap(m_ref_count_ptr, other.m_ref_count_ptr);
        }

        friend void swap(shared_ptr& lhs, shared_ptr& rhs){
            lhs.swap(rhs);
        }

        /* Copy assignment operator : Release the current held resource
           and share the ownership of the resource specified by args */
        shared_ptr& operator=(const shared_ptr& other){
            shared_ptr{ other }.swap(*this);
            return *this;
        }

        /* Move assignment : Release the currently held resource
           and transfer the ownership of resource specified in args */
        shared_ptr& operator=(shared_ptr&& other){
            shared_ptr{ std::move(other) }.swap(*this);
            return *this;
        }

        ~shared_ptr(){
            if(m_ref_count_ptr){
                unsigned long long expected = m_ref_count_ptr->load();
                
                // Decrement the reference count
                while(expected > 0 
                    && !m_ref_count_ptr->compare_exchange_weak(expected, expected - 1));

                auto desired = expected - 1;
                if(desired == 0){   // We are the last user of *m_ref_underlying_ptr
                    delete m_raw_underlying_ptr;
                    delete m_ref_count_ptr;
                }
            }
        }

        /* get() - Returns the stored pointer */
        T* get(){
            return m_raw_underlying_ptr;
        }

        const T* get() const{
            return m_raw_underlying_ptr;
        }

        T& operator*() noexcept {
            return *m_raw_underlying_ptr;
        }

        const T& operator*() const noexcept{
            return *m_raw_underlying_ptr;
        }

        T* operator->() noexcept{
            return m_raw_underlying_ptr;
        }
        
        const T* operator->() const noexcept{
            return m_raw_underlying_ptr;
        }

        /* Comparison operator*/
        bool operator==(const shared_ptr& other) const noexcept{
            return (m_raw_underlying_ptr == other.m_raw_underlying_ptr);
        }

        bool operator!=(const shared_ptr& other) const noexcept{
            return !(*this == other);
        }

        unsigned long long use_count() const noexcept{
            if(m_ref_count_ptr)
                return m_ref_count_ptr->load();
            else
                return 0;
        }
        
        /* Replaces the managed resource */
        void reset(T* ptr){
            if(m_raw_underlying_ptr != ptr)
                shared_ptr(ptr).swap(*this);
        }

        private:
        T* m_raw_underlying_ptr{nullptr};
        std::atomic<unsigned long long>* m_ref_count_ptr{nullptr};
    };
}