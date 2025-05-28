#include<format>
#include<atomic>

namespace dev{
    struct control_block_base{
        std::atomic<unsigned long long> m_ref_count{1};

        void increment(){
            ++m_ref_count;
        }

        auto decrement(){
            return --m_ref_count;
        }

        auto use_count(){
            return m_ref_count.load();
        }

        virtual void release_shared() = 0;
    };

    template<typename T>
    struct control_block : control_block_base{
        T* m_object_ptr;
        explicit control_block(T* data)
        : control_block_base{}
        , m_object_ptr{data}
        {}

        void release_shared() override{
            if(decrement()==0)
            {
                delete m_object_ptr;
                delete this;
            }
        }
    };

    template<typename T>
    struct control_block_with_storage : control_block_base{
        T m_object;
        template<typename... Args>
        explicit control_block_with_storage(Args&&... args)
        : control_block_base{}
        , m_object{T(std::forward<Args>(args)...)}
        {}

        void release_shared() override{
            if(decrement() == 0){
                delete this;
            }
        }

        T* get(){ return &m_object; }
    };

    template<typename T>
    class shared_ptr{
        public:

        /* Default constructor*/
        shared_ptr() = default;

        shared_ptr(T* t, control_block_base* cb)
        : m_raw_underlying_ptr{t}
        , m_control_block_ptr{cb}
        {}

        /* Parametrized constructor : Takes ownership of the pointee */
        shared_ptr(T* ptr) 
        : m_raw_underlying_ptr{ptr}
        {
            try{
                m_control_block_ptr = new control_block(ptr);
            }catch(...){
                delete ptr;
                throw;
            }
        }

        /* Copy constructor : Implements shared co-ownership of the pointee semantics */
        shared_ptr(const shared_ptr& other)
        : m_raw_underlying_ptr{other.m_raw_underlying_ptr}
        , m_control_block_ptr{other.m_control_block_ptr}
        {
            if(m_control_block_ptr)
                m_control_block_ptr->increment();   //Atomic pre-increment
        }

        /* Move constructor : Represents the transfer of ownership */
        shared_ptr(shared_ptr&& other)
        : m_raw_underlying_ptr{ std::exchange(other.m_raw_underlying_ptr, nullptr)}
        , m_control_block_ptr{ std::exchange(other.m_control_block_ptr, nullptr)}
        {}

        /* Swap : Swap two shared_ptr objects member by member */
        void swap(shared_ptr& other){
            using std::swap;
            std::swap(m_raw_underlying_ptr, other.m_raw_underlying_ptr);
            std::swap(m_control_block_ptr, other.m_control_block_ptr);
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
            if(m_control_block_ptr){
                m_control_block_ptr->release_shared();
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
            if(m_control_block_ptr)
                return m_control_block_ptr->use_count();
            else
                return 0;
        }
        
        /* Replaces the managed resource */
        void reset(T* ptr){
            if(m_raw_underlying_ptr != ptr)
                shared_ptr(ptr).swap(*this);
        }

        template<typename U, typename... Args>
        friend shared_ptr<U> make_shared(Args&&... args);

        private:
        T* m_raw_underlying_ptr{nullptr};
        control_block_base* m_control_block_ptr{nullptr};
    };

    template<typename T, typename... Args>
    shared_ptr<T> make_shared(Args&&... args){
        /* Perform a single heap memory allocation */
        control_block_with_storage<T>* cb = new control_block_with_storage<T>(std::forward<Args>(args)...);
        T* data = cb->get();
        return shared_ptr<T>(data, cb);
    }
}