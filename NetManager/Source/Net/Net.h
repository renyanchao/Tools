#pragma once

#ifdef _WIN32
#include "Src/Select.h"
#else
#include "Src/Epoll.h"
#endif

class Net
{
public:
    bool Init()
    {
        return m_Selector.Init();
    }

    void Tick()
    {
        m_Selector.Tick();
    }

private:
#ifdef _WIN32
    Select m_Selector;
#else
    Epoll m_Selector;
#endif
};
