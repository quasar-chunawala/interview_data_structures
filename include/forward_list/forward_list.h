#include<cstddef>
#include<algorithm>
#include<utility>
#include<iterator>
#include<initializer_list>
#include<concepts>

namespace dev{


    template<typename T>
    class forward_list{
        public:
        using value_type = T;
        using size_type = std::size_t;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using difference_type = std::ptrdiff_t;
        
        template<typename U>
        class Iterator{
            public:
            using value_type = forward_list::value_type;
            using pointer = forward_list::pointer;
            using reference = forward_list::reference;
            using difference_type = forward_list::difference_type;
            friend class forward_list<T>;

            private:
            ListNode* m_current_node_ptr;

            public:
            Iterator() = default;
            Iterator(ListNode* ptr)
            : m_current_node_ptr{ptr}
            {}

            // Pre-increment 
            Iterator& operator++(){
                m_current_node_ptr = m_current_node_ptr->next;
                return *this;
            }

            // Post-increment
            Iterator operator++(int){
                auto temp = *this;
                m_current_node_ptr = m_current_node_ptr->next;
                return temp;
            }

            bool operator==(const Iterator& other) const{
                return (m_current_node_ptr == other.m_current_node_ptr);
            }

            bool operator!=(const Iterator& other) const{
                return !(*this == other);
            }

            // Pointer-like operations
            U& operator*(){
                return m_current_node_ptr->m_value;
            }

            const U& operator*() const{
                return m_current_node_ptr->m_value;
            }

            U* operator->(){
                return m_current_node_ptr->value;
            }

            const U* operator() const{
                return m_current_node_ptr->value;
            }
        };

        size_type size() const{
            return m_size;
        }

        bool empty() const{
            return (!m_head);
        }

        iterator begin(){
            return iterator(m_head);
        }

        const_iterator begin() const{
            return const_iterator(m_head);
        }

        const_iterator cbegin() const{
            return const_iterator(m_head);
        }

        iterator end(){
            return iterator(nullptr);
        }

        const_iterator end() const{
            return iterator(nullptr);
        }

        const_iterator cend() const{
            return end();
        }

        /* Destroy the containers content */
        void clear() noexcept{
            for(auto p {m_head}; p!=nullptr;){
                auto q = p->next;
                delete p;
                p = q;
            }
            m_size = 0;
        }

        ~forward_list(){
            clear();
        }

        private:
        struct ListNode{
            value_type m_value;
            ListNode* next{nullptr};
            
            ListNode(const_reference value)
            : m_value{value}
            {}

            ListNode(T&& value)
            : m_value{std::move(value)}
            {}
        };

        ListNode* m_head{nullptr};
        size_type m_size{0};

        public:
        using iterator = Iterator<T>;
        using const_iterator = Iterator<const T>;

        /* Constructors */
        forward_list() = default;

        template<std::input_iterator It>
        forward_list(It b, It e){
            if(b == e) return;
            ListNode* m_curr{nullptr};

            try{
                for(auto it{b};it!=e;++it){
                    if(empty())
                    {
                        m_head = new ListNode(*it);
                        ++m_size;
                        m_curr = m_head;
                    }
                    else{
                        m_curr->next = new ListNode(*it);
                        ++m_size;
                        m_curr = m_curr->next;
                    }
                }
            }catch(...){
                clear();
                throw;
            }
        }

        forward_list(const forward_list& other):
        forward_list(other.begin(), other.end())
        {}

        forward_list(std::initializer_list<T> other)
        :forward_list(other.begin(), other.end())
        {}
        
        forward_list(forward_list&& other)
        : m_head{std::exchange(other.m_head, nullptr)}
        , m_size{std::exchange(other.m_size,0)}
        {}

        void swap(forward_list& other){
            using std::swap;
            swap(m_head, other.m_head);
            swap(m_size, other.m_size);
        }

        forward_list& operator=(const forward_list& other){
            forward_list(other).swap(*this);
            return *this;
        }

        forward_list& operator=(forward_list&& other){
            
        }
    };
}
