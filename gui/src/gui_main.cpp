#include "gui_main.h"

#include <stdio.h>
#include <vector>
#include <string.h>
#include "SDL.h"
#include "stb_image.h"
#include "block_alloc.h"
#include "window.h"
#include "buttons.h"
#include "font.h"
#include "view.h"
#include "view_select.h"
#include "control_bar.h"
#include "buffer_view.h"
#include "bytecode_view.h"
#include "variable_view.h"
#include "error_view.h"
#include "output_view.h"
#include "buffer.h"
#include "float_menu.h"

#include "file_util.h"
#include "util.h"
#include "debug_info.h"
#include "opcodes.h"
#include "compiler.h"

#include "debugger.h"

struct Gui
{
    BlockAlloc alloc;
    Window window;

    ControlBar control_bar;
    ByteCodeView bytecode_view;
    VariableView variable_view;
    ErrorView error_view;
    OutputView output_view;
    View* selectable_views[N_SELECTABLE_VIEWS];
    View* views;

    int main_buffer = -1;

    ByteCode byte_code;
    DebugInfo dbginfo;

    DebugState dbgstate;

    std::vector<Buffer> buffers;
};

Gui gui;

void output_view_print_char(int c)
{
    print_char(&gui.output_view, (char)c);
}

void output_view_print_int(int i)
{
    print_int(&gui.output_view, i);
}

int init_gui(Gui* gui)
{
    gui->alloc = create_block_alloc(1024);

    // init window
    gui->window.width  = 800;
    gui->window.height = 600;

    SDL_Init(SDL_INIT_VIDEO);

    gui->window.window = SDL_CreateWindow(
            "OggeLang",
            SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
            gui->window.width, gui->window.height,
            SDL_WINDOW_RESIZABLE
            );

    if(gui->window.window == nullptr)
    {
        printf("Failed to open window. %s\n", SDL_GetError());
        return 0;
    }

    gui->window.renderer = SDL_CreateRenderer(gui->window.window, -1, 0);
    
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
    SDL_SetRenderDrawBlendMode(gui->window.renderer, SDL_BLENDMODE_BLEND);

    // init fonts
    if(!init_fonts(&gui->window))
        return 0;

    // init views
    create_control_bar(&gui->window, &gui->control_bar);
    create_bytecode_view(&gui->bytecode_view);
    create_variable_view(&gui->variable_view, &gui->byte_code, &gui->dbginfo);
    create_error_view(&gui->error_view);
    create_output_view(&gui->output_view);

    gui->selectable_views[0] = &gui->bytecode_view;
    gui->selectable_views[1] = &gui->variable_view;
    gui->selectable_views[2] = &gui->error_view;
    gui->selectable_views[3] = &gui->output_view;


    ViewSelect* select_right_top = allocate_assign(gui->alloc, ViewSelect());
    ViewSelect* select_right_bottom = allocate_assign(gui->alloc, ViewSelect());
    ViewSelect* select_middle = allocate_assign(gui->alloc, ViewSelect());
    ViewSelect* select_left = allocate_assign(gui->alloc, ViewSelect());
    create_view_select(select_right_top, &gui->variable_view);
    create_view_select(select_right_bottom, &gui->output_view);
    create_view_select(select_middle, &gui->bytecode_view);
    create_view_select(select_left);

    HSplit* split4 = allocate_assign(gui->alloc, HSplit((View*)select_right_top, (View*)select_right_bottom, 0.5f));
    VSplit* split3 = allocate_assign(gui->alloc, VSplit((View*)select_middle, (View*)split4, 0.5f));
    VSplit* split2 = allocate_assign(gui->alloc, VSplit((View*)select_left, (View*)split3, 0.333f));
    HSplit* split1 = allocate_assign(gui->alloc, HSplit((View*)&gui->control_bar, (View*)split2, CONTROL_BAR_HEIGHT));

    gui->views = (View*)split1;


    gui->dbgstate.print_char_func = output_view_print_char;
    gui->dbgstate.print_int_func = output_view_print_int;


    print_int(&gui->output_view, -1000);



    return 1;
}

void compile(int main_buffer_idx)
{
    clear_errors(&gui.error_view);
    clear(&gui.output_view);

    compile_program(&gui.byte_code, gui.buffers[main_buffer_idx].filepath, true, &gui.dbginfo);
    show_bytecode(&gui.bytecode_view, &gui.byte_code);
    print_opcodes(gui.byte_code, &gui.dbginfo);
}

int gui_main()
{
    if(!init_gui(&gui))
        return 0;
        
    Buffer buffer1;
    //buffer_from_source_file("test_programs/long_test.ogge", &buffer1);
    //set_buffer(&gui.buffer_views[0], &buffer1);
    //set_buffer(&gui.buffer_views[1], &buffer1);

    //int tex_width, tex_height, tex_channels;
    //unsigned char* data = stbi_load("run.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    //
    //SDL_Texture* texture;
    //SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data, tex_width, tex_height, 32, 4*tex_width, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    //texture = SDL_CreateTextureFromSurface(window.renderer, surface);
    //SDL_Rect texture_rect;
    //texture_rect.x = 2;
    //texture_rect.y = 2;
    //texture_rect.w = tex_width;
    //texture_rect.h = tex_height;

    //stbi_image_free(data);

    SDL_Event event;
    bool running = true;

    Point mouse_pos = {0,0};

    while(running)
    {
        int n_reloaded_buffers = 0;
        for(Buffer& b: gui.buffers)
            n_reloaded_buffers += reload_buffer(&b);

        if(n_reloaded_buffers > 0)
            compile(gui.main_buffer);


        SDL_SetRenderDrawColor(gui.window.renderer, 40,40,40,0);
        SDL_RenderClear(gui.window.renderer);

        //draw_buffer_view(&window, &buffer_view, &area2);
        Area window_area = Area{0,0,gui.window.width, gui.window.height};
        gui.views->draw(&gui.window, &window_area);
        if(is_float_menu_open())
            draw_float_menu(&gui.window);
        //SDL_RenderCopy(window.renderer, texture, nullptr, &texture_rect);

        SDL_RenderPresent(gui.window.renderer);
        SDL_WaitEvent(&event);

        switch(event.type)
        {
            case SDL_QUIT: running = false; break;
            case SDL_KEYDOWN:
               {
                   switch(event.key.keysym.sym)
                   {
                       case SDLK_ESCAPE: running = false; break;
                       case SDLK_o: 
                         {
                            if(event.key.keysym.mod & KMOD_CTRL)
                                open_file();
                         } break;
                   }
               } break;
            case SDL_MOUSEBUTTONDOWN:
               {
                   if(is_float_menu_open())
                   {
                       float_menu_mouse_click(mouse_pos);
                   }
                   else switch(event.button.button)
                   {
                       case SDL_BUTTON_LEFT:  gui.views->mouse_left_click(mouse_pos); break;
                       case SDL_BUTTON_RIGHT: gui.views->mouse_right_click(mouse_pos); break;
                   }
               } break;
            case SDL_MOUSEBUTTONUP:
               { 
                   switch(event.button.button)
                   {
                       case SDL_BUTTON_LEFT:  gui.views->mouse_left_release(mouse_pos); break;
                       case SDL_BUTTON_RIGHT: gui.views->mouse_right_release(mouse_pos); break;
                   }
               }
            case SDL_MOUSEWHEEL:
               {
                   gui.views->mouse_scroll_update(event.wheel.y, mouse_pos);
               } break;
            case SDL_MOUSEMOTION:
               {
                   mouse_pos = {event.button.x, event.button.y};
                   if(is_float_menu_open())
                       float_menu_mouse_enter(mouse_pos);
                   else
                       gui.views->mouse_enter(mouse_pos);
               } break;
            case SDL_WINDOWEVENT:
               {
                   switch(event.window.event)
                   {
                       case SDL_WINDOWEVENT_RESIZED: 
                           {
                               gui.window.width  = event.window.data1;
                               gui.window.height = event.window.data2;
                           } break;
                   }
               } break;
        }


    }
    
    for(auto b: gui.buffers)
        close_buffer(&b);
    SDL_DestroyRenderer(gui.window.renderer);
    SDL_DestroyWindow(gui.window.window);
    SDL_Quit();

    close(&gui.error_view);
    close(&gui.output_view);
    dealloc(gui.dbginfo.symbol_names_alloc);
    dealloc(gui.alloc);
    return 0;
}


Buffer& get_buffer(int buffer_idx)
{
    return gui.buffers[buffer_idx];
}
std::vector<Buffer>& get_buffers()
{
    return gui.buffers;
}

Buffer* get_main_buffer()
{
    if(gui.main_buffer >= 0)
        return &get_buffer(gui.main_buffer);
    return nullptr;
}

void open_file()
{
    char filepath[1024];
    if(!get_open_file_path(&gui.window, filepath, 1024))
        return;

    convert_to_unix_file_path(filepath);

    // check if file is already in a buffer
    unsigned long filepath_hash = hash_djb2(filepath);
    int i = 0;
    for(Buffer& b : gui.buffers)
    {
        if(b.filepath_hash == filepath_hash)
        {
            set_buffer(get_selected_buffer_view(), i); 
            return;
        }
        i++;
    }

    int filepath_len = strlen(filepath)+1;

    printf("opend new file\n");

    char* filen = (char*)allocate(gui.alloc, filepath_len);
    memcpy(filen, filepath, filepath_len);

    Buffer new_buffer;
    buffer_from_source_file(&new_buffer, filen);
    
    gui.buffers.push_back(new_buffer);
    int new_buffer_idx = gui.buffers.size()-1;

    if( gui.main_buffer < 0 )
    {
        gui.main_buffer = new_buffer_idx;
        compile(new_buffer_idx);
    }

    set_buffer(get_selected_buffer_view(), new_buffer_idx); 


    char* filename;
    int line_num;
    linenumber_from_instruction_addr(&gui.dbginfo, 2, &filename, &line_num);
    printf("filename: %s line_num: %d\n", filename, line_num);
}

void run_program()
{
    start_debug(&gui.dbgstate, &gui.byte_code, &gui.dbginfo);
    run(&gui.dbgstate);    
}

void step_program()
{
    if(!gui.dbgstate.is_running)
        start_debug(&gui.dbgstate, &gui.byte_code, &gui.dbginfo);

    step_instruction(&gui.dbgstate);
}

void stop_program()
{

}

int get_pc()
{
    return gui.dbgstate.pc;
}

View** get_selectable_views()
{
    return (View**)gui.selectable_views;
}
