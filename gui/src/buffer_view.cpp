#include "buffer_view.h"

#include <stdio.h>
#include <stdlib.h>

void BufferView::mouse_scroll_update(int scroll, Point mouse_pos)
{
    this->first_visible_line -= scroll*3; 
    if(this->first_visible_line < 0)
        this->first_visible_line = 0;
    if(this->first_visible_line > this->buffer->lines.size()-1)
        this->first_visible_line = (long)this->buffer->lines.size()-1;
}
void BufferView::draw(Window* window, Area* area)
{
    int text_xpos = area->x + this->line_num_width + 2*LINE_NUM_PADDING;
    int line_num_xpos = area->x + LINE_NUM_PADDING;
    int y_pos = area->y + this->font->size*2;
    
    // draw line number bar
    draw_rect_fill(window, COLOR_DARK2, Point{text_xpos, area->y + area->height}, Point{area->x, area->y});
    
    // draw top bar
    draw_rect_fill(window, COLOR_DARK2, Point{area->x + area->width, area->y + this->font->size}, Point{area->x, area->y});

    if(this->buffer == nullptr)
    {
        draw_text(window, this->font, "No file is open", Point{text_xpos, y_pos}, '\0', COLOR_LIGHT); // draw text
    }
    else
    {
        // draw filename
        draw_text(window, this->font, this->buffer->filename, Point{area->x, area->y + this->font->size - 3});

        // draw content
        char num_string[16];
        for(int i=this->first_visible_line; i < this->buffer->lines.size(); i++)
        {
            char* line = this->buffer->lines[i];
            sprintf(num_string, "%d", i+1);
            draw_text(window, this->font, num_string, Point{line_num_xpos, y_pos}, '\0', COLOR_BRIGHT_BLUE); // draw line number
            draw_text(window, this->font, line, Point{text_xpos, y_pos}, '\n', COLOR_LIGHT); // draw text
            y_pos += this->font->size;
        }
    }
}

void create_buffer_view(BufferView* view, Font* font)
{
    view->font = font;
    view->buffer = nullptr;
    view->line_num_width = 0;
}

void set_buffer(BufferView* view, Buffer* buffer)
{
    view->buffer = buffer;

    // calculate the maximum width of the largest line number
    int n_digits = 0;
    int n_lines = buffer->lines.size();
    while(n_lines > 0)
    {
        n_digits++;
        n_lines /= 10;
    }
    char c[] = "0"; 
    int char_width, char_height;
    get_text_size(view->font, c, &char_width, &char_height);
    
    view->line_num_width = n_digits * char_width;
    view->first_visible_line = 0;
}

void draw_buffer_view(Window* window, BufferView* view, Area* area)
{
}
