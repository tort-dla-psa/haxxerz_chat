#include "ui.h"

#include <GL/glew.h>
#include <SDL.h>
#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_sdl.h"
#include "imgui/examples/imgui_impl_opengl3.h"

ui::queue_t& ui::queue_send(){
    return m_write_q;
}
ui::queue_t& ui::queue_recv(){
    return m_read_q;
}

ui_imgui::ui_imgui()
    :gl_context(NULL)
{
    if (SDL_Init(SDL_INIT_VIDEO |
        SDL_INIT_TIMER |
        SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        abort();
    }

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = glewInit() != GLEW_OK;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        abort();
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void ui_imgui::start(){
    bool done = false;
    std::string message(32, '\0');
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    while(!done){
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT){
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
            {
                done = true;
            }
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        auto drawList = ImGui::GetBackgroundDrawList();

        ImGui::Begin("UI");
        static bool needs_scroll;
        static bool msg_sent;
        ImGui::Text("Time per frame %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        {
            std::string mes;
            while(m_read_q.try_dequeue(mes)){
                m_msgs.emplace_back(mes);
                needs_scroll = true;
            }
        }
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_HorizontalScrollbar;
        ImVec2 child_size = ImVec2(
            ImGui::GetWindowContentRegionWidth(),
            ImGui::GetTextLineHeightWithSpacing() * 18);

        ImGui::BeginChild("Messages", child_size, false, window_flags);
        struct Funcs {
            static int MyResizeCallback(ImGuiInputTextCallbackData* data) {
                if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                    auto my_str = (std::string*)data->UserData;
                    IM_ASSERT(my_str->data() == data->Buf);
                    my_str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
                    data->Buf = my_str->data();
                }
                return 0;
            }

            static bool MyInputTextMultiline(const char* label, std::string* my_str,
                const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
            {
                IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
                return ImGui::InputTextMultiline(label, my_str->data(), my_str->size(), size,
                    flags | ImGuiInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
            }
        };

        static std::string my_str;
        if (my_str.empty()){
            my_str.push_back(0);
        }

        for(auto &mes:m_msgs){
            auto str = mes;
            ImGui::Text("%s", str.c_str());
        }
        if(needs_scroll){
            ImGui::SetScrollHereY(1.f); // 0.0f:top, 0.5f:center, 1.0f:bottom
            needs_scroll = false;
        }
        ImGui::EndChild();

        msg_sent = (Funcs::MyInputTextMultiline("Message", &my_str, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight()*4),
            ImGuiInputTextFlags_CtrlEnterForNewLine |
            ImGuiInputTextFlags_EnterReturnsTrue));
        if(msg_sent) {
            m_write_q.enqueue(my_str);
            my_str.clear();
        }
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
