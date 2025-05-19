#include <fstream>
#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>
#include "resource.h"
#include <sstream>
#include <windows.h>


Fl_Text_Buffer *textbuf = nullptr;
Fl_Text_Editor *editor = nullptr;

// Read file
void open_cb(Fl_Widget*, void*) {
    const char* filename = fl_file_chooser("Open File", "*.txt", nullptr);
    if (!filename) return;

    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    textbuf->text(buffer.str().c_str());
}

// Save file
void save_cb(Fl_Widget*, void*) {
    const char* filename = fl_file_chooser("Save File", "*.txt", nullptr);
    if (!filename) return;

    std::ofstream file(filename);
    file << textbuf->text();
}

// End
void quit_cb(Fl_Widget*, void*) {
    exit(0);
}

int main(int argc, char **argv) {
    Fl_Window *win = new Fl_Window(800, 600, "NeraiMemo");
    Fl_Menu_Bar *menu = new Fl_Menu_Bar(0, 0, 800, 25);

    menu->add("File/Open",  FL_CTRL + 'o', open_cb);
    menu->add("File/Save",  FL_CTRL + 's', save_cb);
    menu->add("File/Quit",  FL_CTRL + 'q', quit_cb);

    textbuf = new Fl_Text_Buffer();
    editor = new Fl_Text_Editor(0, 25, 800, 575);
    editor->buffer(textbuf);
    editor->color(fl_rgb_color(170, 255, 255));

    win->end();
    win->show(argc, argv);

    #ifdef _WIN32
        HWND hwnd = fl_xid(win);
        HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
        if (hIcon) {
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SendMessage(hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
        } else {
            MessageBox(NULL, "Failed to load .ico file.", "ERROR", MB_OK);
        }
    #endif

    return Fl::run();
}