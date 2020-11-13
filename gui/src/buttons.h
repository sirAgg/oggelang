#ifndef BUTTONS_H
#define BUTTONS_H

#include <vector>
#include "window.h"
#include "font.h"

struct Button
{
    int top,bottom,left,right;
    const char* text;

    void (*click_callback)(void*);

    void* callback_data;

    Point text_offset;
    bool is_hovered = false;
};

struct ButtonGroup
{
    Font* font;
    std::vector<Button> buttons;
};

void add_button(ButtonGroup* group, Button b);
void draw_buttons(ButtonGroup* group, Window* window);
void check_click(ButtonGroup* group, Point p);
void check_enter(ButtonGroup* group, Point p);

#endif
