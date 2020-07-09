#pragma once
#include <deque>
#include "datatypes.hpp"
#include "readerwriterqueue.h"

class ui{
public:
    using queue_t = moodycamel::BlockingReaderWriterQueue<message>;
protected:
    std::deque<message> m_msgs;
    queue_t m_read_q, m_write_q;
public:
    virtual ~ui(){};
    virtual void start()=0;
    queue_t& queue_send();
    queue_t& queue_recv();
};

#include <SDL.h>
class ui_imgui:public ui{
     SDL_GLContext gl_context;
     SDL_Window* window;
public:
    ui_imgui();
    virtual void start()override;
};
