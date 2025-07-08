#include <atomic>
#include <format>
#include <memory>
#include <utility>

namespace dev {

template<typename T>
class shared_ptr_base
{
  public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

  protected:
    struct destroy_wrapper // RAII class
    {
        template<typename Deleter>
        destroy_wrapper(Deleter deleter)
          : m_destroyer_ptr{ new destroyer<Deleter>(deleter) }
        {
        }

        void operator()(pointer ptr)
        {
            (*m_destroyer_ptr)(ptr); // Virtual polymorphism
        }

        struct destroyer_base
        {
            virtual void operator()(pointer ptr) = 0;
            virtual ~destroyer_base() = default;
        };

        template<typename Deleter>
        struct destroyer : public destroyer_base
        {
            destroyer(Deleter deleter)
              : destroyer_base()
              , m_deleter{ deleter }
            {
            }

            // The default move and copy constructors will copy/move each member

            void operator()(pointer ptr) override { (m_deleter)(ptr); }

            Deleter m_deleter;
        };

        ~destroy_wrapper() { delete m_destroyer_ptr; }
        destroyer_base* m_destroyer_ptr;
    };

    struct control_block_base
    {
        std::atomic<unsigned long long> m_ref_count{ 1u };
        destroy_wrapper m_destroy_wrapper;

        /**
         * @brief Default constructor
         */
        control_block_base()
          : m_destroy_wrapper{ std::default_delete<T>() }
        {
        }

        /**
         * @brief Constructor that accepts a destroyer.
         */
        control_block_base(destroy_wrapper wrapper)
          : m_destroy_wrapper{ wrapper }
        {
        }

        /**
         * @brief helper function to increment the object reference count
         */
        void increment() { m_ref_count.fetch_add(1u); }

        /**
         * @brief helper function to decrement the object reference count
         */
        auto decrement()
        {
            auto result = m_ref_count.fetch_sub(1u);
            return result;
        }

        // There is no use-case for copying control blocks of a shared_ptr<T>
        // instance. For safety, I delete these functions.
        control_block_base(const control_block_base&) = delete;
        control_block_base& operator=(const control_block_base&) = delete;
        control_block_base(control_block_base&&) = delete;
        control_block_base& operator=(control_block_base&&) = delete;

        /**
         * @brief Return the object reference count
         */
        auto use_count() { return m_ref_count.load(); }

        virtual void release_shared() = 0;
        virtual ~control_block_base() {}
    };

    struct control_block : public control_block_base
    {
        pointer m_object_ptr;

        explicit control_block(pointer data)
          : control_block_base{}
          , m_object_ptr{ data }
        {
        }

        explicit control_block(pointer data, destroy_wrapper wrapper)
          : control_block_base{ wrapper }
          , m_object_ptr{ data }
        {
        }

        void release_shared() override
        {
            if (control_block_base::decrement() == 0) {
                control_block_base::m_destroy_wrapper(m_object_ptr);
                delete this;
            }
        }

        ~control_block() {}
    };

    /**
     * @brief This is a specialized version of %control_block_base, where the managed
     * object resides within the control block itself.
     */
    struct control_block_with_storage : public control_block_base
    {
        T m_object;

        /**
         * @brief Instantiates the managed object by perfectly forwarding
         * the constructor args.
         */
        template<typename... Args>
        explicit control_block_with_storage(Args&&... args)
          : control_block_base{}
          , m_object(std::forward<Args>(args)...)
        {
        }

        /**
         * @brief C'tor that accepts a destroyer object and perfectly
         * forwards the constructor args
         */
        template<typename... Args>
        explicit control_block_with_storage(destroy_wrapper wrapper, Args&&... args)
          : control_block_base{ wrapper }
          , m_object(std::forward<Args>(args)...)
        {
        }

        void release_shared() override
        {
            if (control_block_base::decrement() == 0) {
                delete this;
            }
        }

        T* get() { return &m_object; }
    };

  public:
    /**
     * @brief Default constructor
     */
    shared_ptr_base()
      : m_raw_underlying_ptr{ nullptr }
      , m_control_block_ptr{ nullptr }
    {
    }

    /**
     * @brief Constructor that takes a raw pointer. Takes
     * ownership of the pointee.
     */
    explicit shared_ptr_base(pointer ptr)
      : m_raw_underlying_ptr{ ptr }
      , m_control_block_ptr{ new control_block(ptr) }
    {
    }

    /**
     * @brief Constructor that takes a raw pointer and a custom deleter. Takes
     * ownership of the pointee.
     */
    template<typename Deleter>
    explicit shared_ptr_base(T* ptr, Deleter deleter)
      : m_raw_underlying_ptr{ ptr }
      , m_control_block_ptr{ nullptr }
    {
        try {
            control_block_base* cb = new control_block(ptr, destroy_wrapper(deleter));
            m_control_block_ptr = cb;
        } catch (std::exception& ex) {
            deleter(ptr);
            throw ex;
        }
    }

    explicit shared_ptr_base(T* ptr, control_block_base* cb)
      : m_raw_underlying_ptr{ ptr }
      , m_control_block_ptr{ cb }
    {
    }

    /**
     * @brief Copy constructor. Models shared co-ownership of the resource
     * semantics.
     */
    shared_ptr_base(const shared_ptr_base& other)
      : m_raw_underlying_ptr{ other.m_raw_underlying_ptr }
      , m_control_block_ptr{ other.m_control_block_ptr }
    {
        if (m_control_block_ptr)
            m_control_block_ptr->increment(); // Atomic pre-increment
    }

    /**
     * @brief Move constructor. Represents transfer of ownership.
     */
    shared_ptr_base(shared_ptr_base&& other) noexcept
      : m_raw_underlying_ptr{ std::exchange(other.m_raw_underlying_ptr, nullptr) }
      , m_control_block_ptr{ std::exchange(other.m_control_block_ptr, nullptr) }
    {
    }

    /**
     * @brief Swaps two shared-pointer objects member-by-member.
     */
    void swap(shared_ptr_base& other)
    {
        std::swap(m_raw_underlying_ptr, other.m_raw_underlying_ptr);
        std::swap(m_control_block_ptr, other.m_control_block_ptr);
    }

    friend void swap(shared_ptr_base& lhs, shared_ptr_base& rhs) { lhs.swap(rhs); }

    /**
     * @brief Copy assignment operator. Release the currently held resource
     * and become a shared co-owner of the resource specified by user-supplied
     * argument @a other.
     */
    shared_ptr_base& operator=(const shared_ptr_base& other)
    {
        shared_ptr_base{ other }.swap(*this);
        return *this;
    }

    /**
     * @brief Move assignment operator. Release the currently held resource.
     * Transfer ownership of the resource.
     */
    shared_ptr_base& operator=(shared_ptr_base&& other)
    {
        shared_ptr_base{ std::move(other) }.swap(*this);
        return *this;
    }

    /**
     * @brief Destructor
     */
    ~shared_ptr_base()
    {
        if (m_control_block_ptr) {
            m_control_block_ptr->release_shared();
            m_raw_underlying_ptr = nullptr;
        }
    }

    // Pointer-like functions
    /**
     * @brief Returns the raw underlying pointer.
     */
    [[nodiscard]] pointer get() { return m_raw_underlying_ptr; }

    /**
     * @brief Returns a %pointer-to-const
     */
    [[nodiscard]] const_pointer get() const { return m_raw_underlying_ptr; }

    /**
     * @brief Returns a reference to the managed object
     */
    [[nodiscard]] reference operator*() noexcept { return *m_raw_underlying_ptr; }

    /**
     * @brief Returns a reference-to-const.
     */
    [[nodiscard]] const_reference operator*() const noexcept
    {
        return *m_raw_underlying_ptr;
    }

    /**
     * @brief Implementation of the indirection operator
     */
    [[nodiscard]] pointer operator->() noexcept { return m_raw_underlying_ptr; }

    /**
     * @brief Implementation of the indirection operator
     */
    [[nodiscard]] const_pointer operator->() const noexcept
    {
        return m_raw_underlying_ptr;
    }

    /**
     * @brief Spaceship operator.
     */
    friend auto operator<=>(const shared_ptr_base& lhs, const shared_ptr_base& rhs)
    {
        return lhs.m_raw_underlying_ptr <=> rhs.m_raw_underlying_ptr;
    }

    /**
     * @brief Equality comparison operator
     */
    [[nodiscard]] bool operator==(const shared_ptr_base& other) const noexcept
    {
        return (m_raw_underlying_ptr == other.m_raw_underlying_ptr);
    }

    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept
    {
        return m_raw_underlying_ptr == nullptr;
    }

    /**
     * @brief Returns the reference count of the managed object
     */
    [[nodiscard]] unsigned long long use_count() const noexcept
    {
        if (m_control_block_ptr)
            return m_control_block_ptr->use_count();
        else
            return 0;
    }

    /**
     * @brief Replaces the managed object.
     */
    void reset(T* ptr)
    {
        if (m_raw_underlying_ptr != ptr)
            shared_ptr_base(ptr).swap(*this);
    }

    /**
     * @brief %make_shared is a utility function that accepts constructor
     * args and perfoms a single heap memory allocation for both the managed resource
     * and the control block.
     */
    template<typename... Args>
    friend shared_ptr_base<T> // Single-object version
    make_shared(Args&&... args)
    {
        /* Perform a single heap memory allocation */
        control_block_with_storage* cb = new control_block_with_storage(
          destroy_wrapper(std::default_delete<T>()), (std::forward<Args>(args))...);
        // T* data = cb->get();
        return shared_ptr_base<T>(cb->get(), cb);
    }

  private:
    T* m_raw_underlying_ptr;
    control_block_base* m_control_block_ptr;
};

template<typename T>
class shared_ptr : public shared_ptr_base<T>
{
  public:
    explicit shared_ptr(T* ptr)
      : shared_ptr_base<T>(ptr)
    {
    }

    template<typename Deleter = std::default_delete<T>>
    explicit shared_ptr(T* ptr, Deleter deleter)
      : shared_ptr_base<T>(ptr, deleter)
    {
    }
};

template<typename T>
class shared_ptr<T[]> : public shared_ptr_base<T>
{
  public:
    explicit shared_ptr(T* ptr)
      : shared_ptr_base<T>(ptr)
    {
    }

    template<typename Deleter = std::default_delete<T[]>>
    explicit shared_ptr(T* ptr, Deleter deleter)
      : shared_ptr_base<T>(ptr, deleter)
    {
    }
};
} // namespace dev