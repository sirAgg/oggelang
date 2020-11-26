#include "bytecode_view.h"

const int LINE_NUM_PADDING = 3;

void ByteCodeView::mouse_scroll_update(int scroll, Point mouse_pos)
{
    if(!this->has_bytecode)
        return;

    this->first_visible_line -= scroll*3; 
    if(this->first_visible_line < 0)
        this->first_visible_line = 0;
    if(this->first_visible_line > this->lines.size()-1)
        this->first_visible_line = (long)this->lines.size()-1;
}

void ByteCodeView::draw(Window* window, Area* area)
{
    auto font = get_monospace_font();

    this->pos = {area->x, area->y};
    int text_xpos = area->x + this->line_num_width + 2*LINE_NUM_PADDING;
    int line_num_xpos = area->x + LINE_NUM_PADDING;
    int y_pos = area->y + font->size*1;

    // fill background
    draw_rect_fill(window, COLOR_DARK, Point{area->x+area->width, area->y+area->height}, Point{area->x, area->y});
    
    // draw line number bar
    draw_rect_fill(window, COLOR_DARK2, Point{text_xpos, area->y + area->height}, Point{area->x, area->y});

    if(this->has_bytecode)
    {
        // draw content
        char num_string[16];
        for(int i=this->first_visible_line; i < this->lines.size(); i++)
        {
            char* line = this->lines[i];
            sprintf(num_string, "%d", this->addresses[i]);
            draw_text(window, font, num_string, Point{line_num_xpos, y_pos}, '\0', COLOR_BRIGHT_BLUE); // draw line number
            draw_text(window, font, line, Point{text_xpos, y_pos}, '\n', COLOR_LIGHT); // draw text
            y_pos += font->size;
        }
    }
    else
    {
        draw_text(window, font, "No project loaded.", Point{text_xpos, y_pos}, '\0', COLOR_LIGHT);
    }
}

void create_bytecode_view(ByteCodeView* view)
{
    view->first_visible_line = 1;
}

void show_bytecode(ByteCodeView* view, ByteCode* byte_code)
{
    if(view->has_bytecode)
    {
        view->lines.clear();
        view->addresses.clear();
        dealloc(view->line_alloc);
    }

    view->line_alloc = create_block_alloc(2048);
    

    auto font = get_monospace_font();

    int n_digits = 0;
    int n_addresses = byte_code->code_size;
    while(n_addresses > 0)
    {
        n_digits++;
        n_addresses/=10;
    }

    char c[] = "0";
    int char_width, char_height;
    get_text_size(font, c, &char_width, &char_height);

    view->line_num_width = n_digits * char_width;

    int pc = 0;
    while(pc < byte_code->code_size)
    {
        char* line = (char*)allocate(view->line_alloc, 32);

        int opcode_size = opcode_to_string((OpCode*)byte_code->data+pc, line);

        view->lines.push_back(line);
        view->addresses.push_back(pc);
        pc += opcode_size;
    }

    view->has_bytecode = true;
}
