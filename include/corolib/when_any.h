/**
 * @file when_any.h
 * @brief
 * when_any waits for any of the async_operation or async_task objects passed to it in its constructor
 * to complete. These objects are placed in a vector m_elements.
 * 
 * For every async_operation or async_task object in m_elements, when_any creates a wait_any_on
 * object and places it in a second vector m_wait_any.
 * It associates each element in m_elements with the corresponding element in m_wait_any.
 * This way the async_operation or async_task object in m_elements can
 * inform when_any via the corresponding element in m_wait_any that it has completed.
 *
 * TODO1: verify instantiation of when_any with an appropriate type using C++20 concepts.
 * TODO2: implement other ways to pass the async_operation or async_task objects.
 *
 * @author Johan Vanslembrouck (johan.vanslembrouck@capgemini.com, johan.vanslembrouck@gmail.com)
 */

#ifndef _WHEN_ANY_AWAITABLE_
#define _WHEN_ANY_AWAITABLE_

#include <vector>
#include <type_traits>

#include "print.h"
#include "when_any_one.h"
#include "async_base.h"

namespace corolib
{
    class when_any
    {
    public:
        /**
         * @brief constructor that takes a variable list of async_base-derived objects and
         * populates the internal vector m_elements with its elements.
         */
        template<typename... AsyncBaseTypes>
        when_any(AsyncBaseTypes&... others)
        {
            make_when_any(others...);
        }

        /**
         * @brief constructor that takes an initializer list and
         * populates the internal vector m_elements with its elements.
         */
        when_any(std::initializer_list<async_base*> async_ops)
        {
            print(PRI2, "%p: when_any::when_any(std::initializer_list<async_base*> async_ops)\n", this);
            for (async_base* async_op : async_ops)
            {
                // Only place the object in m_elements if it has not yet been completed.
                if (!async_op->is_ready())
                {
                    when_any_one* q = new when_any_one();
                    m_wait_any.push_back(q);
                    async_op->setWaitAny(q);
                    m_elements.push_back(async_op);
                }
            }
        }

        /**
         * @brief constructor that takes a pointer to a C-style array of objects and its size
         * and that populates the internal vector m_elements with its elements.
         */
        when_any(async_base* pasync_ops[], int size)
        {
            print(PRI2, "%p: when_any::when_any(async_base* pasync_ops, int size)\n", this);
            for (int i = 0; i < size; i++)
            {
                // Only place the object in m_elements if it has not yet been completed.
                async_base* op = pasync_ops[i];
                if (!op->is_ready())
                {
                    when_any_one* q = new when_any_one();
                    m_wait_any.push_back(q);
                    op->setWaitAny(q);
                    m_elements.push_back(op);
                }
            }
        }

        when_any(const when_any& s) = delete;

        when_any(when_any&& s)
        {
            print(PRI2, "%p: when_any::when_any(when_any&& s)\n", this);
        }

        ~when_any()
        {
            print(PRI2, "%p: when_any::~when_any()\n", this);
            for (std::size_t i = 0; i < m_wait_any.size(); i++)
            {
                m_elements[i]->setWaitAny(nullptr);
                delete m_wait_any[i];
            }
        }

        when_any& operator = (const when_any&) = delete;
        when_any& operator = (when_any&& s) = delete;

        /**
         * @brief start_all starts all lazy start coroutines in m_elements.
         * This should be done only once.
         * An application may/will call co_await several times on a wait_any object,
         * which leads to await_ready being called several times as well.
         * However, the lazy start coroutines should only be started once,
         * i.e. on the first co_await call.
         * To avoid multiple calls of start(), data member m_first is used.
         * This data member is added to when_any instead of to awaiter,
         * because an awaiter object is created for every co_await call.
         */
        void start_all()
        {
            print(PRI2, "%p: when_any:start_all(): m_first = %d\n", this, m_first);
            if (m_first)
            {
                for (std::size_t i = 0; i < m_elements.size(); i++)
                {
                    m_elements[i]->start();
                }
                m_first = false;
            }
        }

        auto operator co_await() noexcept
        {
            class awaiter
            {
            public:
                awaiter(when_any& when_any_)
                    : m_when_any(when_any_)
                {
                    print(PRI2, "%p: when_any::awaiter::awaiter()\n", this);
                }

                bool await_ready()
                {
                    print(PRI2, "%p: when_any::awaiter::await_ready()\n", this);
                    for (std::size_t i = 0; i < m_when_any.m_wait_any.size(); i++)
                    {
                        if (m_when_any.m_wait_any[i]->get_completed())
                        {
                            print(PRI2, "%p: when_any::awaiter::await_ready(): return true for i = %d;\n", this, i);
                            return true;
                        }
                    }
                    print(PRI2, "%p: when_any::awaiter::await_ready(): return false;\n", this);
                    return false;
                }

                void await_suspend(std::coroutine_handle<> awaiting)
                {
                    print(PRI2, "%p: when_any::awaiter::await_suspend(...)\n", this);
                    m_when_any.start_all();    // Will have no effect in case of an eager start
                    for (auto el : m_when_any.m_wait_any)
                    {
                        el->set_awaiting(awaiting);
                    }
                }

                int await_resume()
                {
                    // Find out which one has completed
                    print(PRI2, "%p: when_any::awaiter::await_resume()\n", this);
                    for (std::size_t i = 0; i < m_when_any.m_wait_any.size(); i++)
                    {
                        if (m_when_any.m_wait_any[i]->get_and_reset_completed())
                        {
                            print(PRI2, "%p: when_any::awaiter::await_resume(): return i = %d\n", i);
                            // TODO: possibly remove i-th element from m_wait_any and m_elements
                            // or otherwise mark them as not having to be reconsidered again in await_ready
                            return i;
                        }
                    }

                    print(PRI1, "%p: when_any::awaiter::await_resume(): Error, none has completed, yet the coroutine has been resumed\n", this);
                    return -1;
                }
            private:
                when_any& m_when_any;
            };

            return awaiter{ *this };
        }

    protected:

        template<typename... AsyncBaseTypes>
        void make_when_any(AsyncBaseTypes&... others);

        template<typename T, typename... AsyncBaseTypes,
                 typename std::enable_if<std::is_base_of_v<async_base, T>, int>::type = 0>
        void make_when_any(T& t, AsyncBaseTypes&... others) {
            async_base* async_op = static_cast<async_base*>(&t);
            // Only place the object in m_elements if it has not yet been completed.
            if (!async_op->is_ready())
            {
                when_any_one* q = new when_any_one();
                m_wait_any.push_back(q);
                async_op->setWaitAny(q);
                m_elements.push_back(async_op);
            }
            make_when_any(others...);
        };

        //template<>      // g++:  error: explicit specialization in non-namespace scope �class corolib::when_any�
        void make_when_any() {
        };

    private:
        std::vector<when_any_one*> m_wait_any;
        std::vector<async_base*> m_elements;
        bool m_first{ true };
    };

    /**
    * @brief when_anyT is the original implementation of when_any (which has been renamed to when_anyT).
    * Its implementation is here for historical/backup/reference reasons only.
    * when_anyT is considered to be obsolete.
    */
    template<typename TYPE>
    class when_anyT
    {
    public:
	    /**
         * @brief constructor that takes an initializer list and
		 * populates the internal vector m_elements with its elements.
         */
        when_anyT(std::initializer_list<TYPE*> aws)
        {
            print(PRI2, "%p: when_anyT::when_any(std::initializer_list<TYPE*> aws)\n", this);
            for (TYPE* a : aws)
            {
                // Only place the object in m_elements if it has not yet been completed.
                if (!a->is_ready())
                {
                    when_any_one* q = new when_any_one();
                    m_wait_any.push_back(q);
                    a->setWaitAny(q);
                    m_elements.push_back(a);
                }
            }
        }

        /**
         * @brief constructor that takes a pointer to a C-style array of objects and its size
		 * and that populates the internal vector m_elements with its elements.
         */
        when_anyT(TYPE* aws, int size)
        {
            print(PRI2, "%p: when_any::when_any(TYPE* aws, int size)\n", this);
            for (int i = 0; i < size; i++)
            {
                // Only place the object in m_elements if it has not yet been completed.
                if (!aws[i].is_ready())
                {
                    when_any_one* q = new when_any_one();
                    m_wait_any.push_back(q);
                    aws[i].setWaitAny(q);
                    m_elements.push_back(&aws[i]);
                }
            }
        }

        when_anyT(const when_any& s) = delete;

        when_anyT(when_any&& s)
        {
            print(PRI2, "%p: when_anyT::when_any(when_any&& s)\n", this);
        }

        ~when_anyT()
        {
            print(PRI2, "%p: when_anyT::~when_any()\n", this);
            for (std::size_t i = 0; i < m_wait_any.size(); i++)
            {
                m_elements[i]->setWaitAny(nullptr);
                delete m_wait_any[i];
            }
        }

        when_anyT& operator = (const when_anyT&) = delete;

        when_anyT& operator = (when_anyT&& s)
        {
            print(PRI2, "%p: when_anyT::when_anyT = (when_anyT&& s)\n", this);
            s.coro = nullptr;
            return *this;
        }

        /**
         * @brief start_all starts all lazy start coroutines in m_elements.
         * This should be done only once.
         * An application may/will call co_await several times on a wait_any object,
         * which leads to await_ready being called several times as well.
         * However, the lazy start coroutines should only be started once,
         * i.e. on the first co_await call.
         * To avoid multiple calls of start(), data member m_first is used.
         * This data member is added to when_any instead of to awaiter,
         * because an awaiter object is created for every co_await call.
         */
        void start_all()
        {
            print(PRI2, "%p: when_anyT:start_all(): m_first = %d\n", this, m_first);
            if (m_first)
            {
                for (std::size_t i = 0; i < m_elements.size(); i++)
                {
                    m_elements[i]->start();
                }
                m_first = false;
            }
        }

        auto operator co_await() noexcept
        {
            class awaiter
            {
            public:
                awaiter(when_anyT& when_any_)
                    : m_when_any(when_any_)
                {
                    print(PRI2, "%p: when_anyT::awaiter::awaiter()\n", this);
                }

                bool await_ready()
                {
                    print(PRI2, "%p: when_anyT::awaiter::await_ready()\n", this);
                    for (std::size_t i = 0; i < m_when_any.m_wait_any.size(); i++)
                    {
                        if (m_when_any.m_wait_any[i]->get_completed())
                        {
                            print(PRI2, "%p: when_anyT::awaiter::await_ready(): return true for i = %d;\n", this, i);
                            return true;
                        }
                    }
                    print(PRI2, "%p: when_anyT::awaiter::await_ready(): return false;\n", this);
                    return false;
                }

                void await_suspend(std::coroutine_handle<> awaiting)
                {
                    print(PRI2, "%p: when_anyT::awaiter::await_suspend(...)\n", this);
                    m_when_any.start_all();    // Will have no effect in case of an eager start
                    for (auto el : m_when_any.m_wait_any)
                    {
                        el->set_awaiting(awaiting);
                    }
                }

                int await_resume()
                {
                    // Find out which one has completed
                    print(PRI2, "%p: when_anyT::awaiter::await_resume()\n", this);
                    for (std::size_t i = 0; i < m_when_any.m_wait_any.size(); i++)
                    {
                        if (m_when_any.m_wait_any[i]->get_and_reset_completed())
                        {
                            print(PRI2, "%p: when_anyT::awaiter::await_resume(): return i = %d\n", i);
                            // TODO: possibly remove i-th element from m_wait_any and m_elements
                            // or otherwise mark them as not having to be reconsidered again in await_ready
                            return i;
                        }
                    }
                    
                    print(PRI1, "%p: when_anyT::awaiter::await_resume(): Error, none has completed, yet the coroutine has been resumed\n", this);
                    return -1;
                }
            private:
                when_anyT& m_when_any;
            };

            return awaiter{ *this };
        }

    private:
        std::vector<when_any_one*> m_wait_any;
        std::vector<TYPE*> m_elements;
        bool m_first{true};
    };

}

#endif
