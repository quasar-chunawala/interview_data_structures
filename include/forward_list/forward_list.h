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


        };

        size_type size() const{
            return m_size;
        }

        bool empty() const{
            return (!m_head);
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
    };
}
