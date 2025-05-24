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

class TitleBar : public Fl_Box {
public:
    TitleBar(int X, int Y, int W, int H, const char* L = nullptr)
        : Fl_Box(X, Y, W, H, L) {
        box(FL_FLAT_BOX);
        color(fl_rgb_color(100, 149, 237));  // コーンフラワーブルー
        labelsize(14);
        align(FL_ALIGN_CENTER);
    }

protected:
    int handle(int event) override {
        static int offset_x = 0;
        static int offset_y = 0;

        Fl_Window *win = window();
        if (!win) return 0;

        switch (event) {
            case FL_PUSH:
                offset_x = Fl::event_x_root() - win->x();
                offset_y = Fl::event_y_root() - win->y();
                return 1;
            case FL_DRAG:
                win->position(Fl::event_x_root() - offset_x,
                             Fl::event_y_root() - offset_y);
                return 1;
        }
        return Fl_Box::handle(event);
    }
};

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

void quit_cb(Fl_Widget*, void*) {
    exit(0);
}

void minimize_cb(Fl_Widget*, void*) {
#ifdef _WIN32
    HWND hwnd = fl_xid(Fl::first_window());
    ShowWindow(hwnd, SW_MINIMIZE);
#endif
}

void maximize_cb(Fl_Widget*, void*) {
#ifdef _WIN32
    HWND hwnd = fl_xid(Fl::first_window());

    // モニターのワークエリア（タスクバーを除いた領域）を取得
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    // FLTKのウィンドウをフルスクリーンに見えるサイズに変更
    Fl::first_window()->resize(workArea.left, workArea.top,
                               workArea.right - workArea.left,
                               workArea.bottom - workArea.top);
#endif
}

void show_in_taskbar(HWND hwnd) {
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_APPWINDOW;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

    // 再表示してスタイル変更を反映させる
    ShowWindow(hwnd, SW_HIDE);
    ShowWindow(hwnd, SW_SHOW);
}

int main(int argc, char **argv) {
    Fl_Window *win = new Fl_Window(800, 600, "NeraiMemo");
    win->border(0);

    TitleBar *titlebar = new TitleBar(0, 0, 800, 30, "NeraiMemo");

    Fl_Button *close_btn = new Fl_Button(730, 0, 70, 30, "閉じちゃう");
    close_btn->labelsize(16);
    close_btn->callback(quit_cb);
    close_btn->color(fl_rgb_color(220, 20, 60));  // crimson
    close_btn->labelcolor(FL_WHITE);
    close_btn->box(FL_FLAT_BOX);

    Fl_Button *minimize_btn = new Fl_Button(680, 0, 50, 30, "かくす");
    minimize_btn->labelsize(16);
    minimize_btn->callback(minimize_cb);
    minimize_btn->color(fl_rgb_color(100, 149, 237));
    minimize_btn->labelcolor(FL_WHITE);
    minimize_btn->box(FL_FLAT_BOX);

    Fl_Button *maximize_btn = new Fl_Button(610, 0, 70, 30, "大きくする");
    maximize_btn->labelsize(16);
    maximize_btn->callback(maximize_cb);
    maximize_btn->color(fl_rgb_color(100, 149, 237));
    maximize_btn->labelcolor(FL_WHITE);
    maximize_btn->box(FL_FLAT_BOX);

    Fl_Menu_Bar *menu = new Fl_Menu_Bar(0, 30, 800, 25);
    menu->add("File/Open", FL_CTRL + 'o', open_cb);
    menu->add("File/Save", FL_CTRL + 's', save_cb);
    menu->add("File/Quit", FL_CTRL + 'q', quit_cb);

    textbuf = new Fl_Text_Buffer();
    editor = new Fl_Text_Editor(0, 55, 800, 575);
    editor->buffer(textbuf);
    editor->color(fl_rgb_color(170, 255, 255));
    win->resizable(editor);
    win->end();
    win->show(argc, argv);

    #ifdef _WIN32
        HWND hwnd = fl_xid(win);
        show_in_taskbar(hwnd);
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