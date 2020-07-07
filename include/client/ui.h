#pragma once
#include <deque>
#include "datatypes.hpp"

class ui{
protected:
    std::deque<message> m_msgs;
public:
    virtual ~ui(){};
    virtual void start()=0;
    void add_recieved_msg(const message &mes);
};

#include <SDL.h>
class ui_imgui:public ui{
     SDL_GLContext gl_context;
     SDL_Window* window;
public:
    ui_imgui();
    virtual void start()override;
};
