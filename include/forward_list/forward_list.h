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
