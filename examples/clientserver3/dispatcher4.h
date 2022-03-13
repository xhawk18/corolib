/**
 * @file dispatcher4.h
 * @brief
 *
 * @author Johan Vanslembrouck (johan.vanslembrouck@altran.com, johan.vanslembrouck@gmail.com)
 */
#ifndef _DISPATCHER4_H_
#define _DISPATCHER4_H_

#include <functional>

#include <corolib/commservice.h>
#include <corolib/async_operation.h>
#include <corolib/async_task.h>
#include <corolib/print.h>

using handleRequest =
    std::function<async_task<int>(std::string)>;

struct dispatch_table
{
    std::string     str;
    handleRequest    op;
};

class Dispatcher : public CommService
{
public:
    Dispatcher()
    {
    }
    
    void registerFunctor(std::string tx, handleRequest op)
    {
        print(PRI1, "registerFunctor(%s, op)\n", tx.c_str());        
        int index = get_free_index();
        async_operation<std::string> ret{ this, index };
    
        m_dispatch_table[index].str = tx;
        m_dispatch_table[index].op = op;
    }

    std::string getHeader(std::string str) {
        while (str.size()) {
            int index = str.find(':');
            if (index != std::string::npos) {
                return (str.substr(0, index));
            }
        }
        return "";
    }

    async_task<int> dispatch(std::string str)
    {
        print(PRI2, "Dispatcher::dispatch(<%s>), m_index = %d\n", str.c_str(), m_index);
        
        std::string header = getHeader(str);

        for(int i = 0; i < m_index+1; i++)
        {
            print(PRI2, "Dispatcher::dispatch(): m_dispatch_table[%d].str = <%s>\n", i, m_dispatch_table[i].str.c_str());
            if (m_dispatch_table[i].str.compare(header) == 0)
            {
                print(PRI1, "Dispatcher::dispatch(): found match at index %d\n", i);
                async_task<int> t = m_dispatch_table[i].op(str);
                co_await t;
                break;
            }
        }
        co_return 0;
    }

protected:
    dispatch_table m_dispatch_table[NROPERATIONS];
};

#endif
