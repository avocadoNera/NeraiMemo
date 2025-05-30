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
#include <string>
#include <windows.h>

using namespace std;

Fl_Text_Buffer *textbuf = nullptr;
Fl_Text_Editor *editor = nullptr;

bool isMaximized = false;
bool isModified = false;
int prevX = 0, prevY = 0, prevW = 0, prevH = 0;
Fl_Window* mainWin     = nullptr;  // ウィンドウへのポインタを保持
Fl_Button* maximizeBtn = nullptr; // ボタンへのポインタを保持
Fl_Box* status_bar = nullptr;
string current_filename = "（無題）";

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

void update_status_bar() {
    string status = isModified ? "未保存" : "保存済み";
    string label = "ファイル: " + current_filename + " | 状態: " + status;
    status_bar->label(label.c_str());
    mainWin->add(status_bar);
    status_bar->redraw();
}

void on_text_changed(int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText, void* cbArg) {
    isModified = true;
    update_status_bar();
}

// Read file
void open_cb(Fl_Widget*, void*) {
    const char* filename = fl_file_chooser("Open File", "*.txt", nullptr);
    if (!filename) return;

    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    textbuf->text(buffer.str().c_str());

    current_filename = filename;
    update_status_bar();
}

// Save file
void save_cb(Fl_Widget*, void*) {
    const char* filename = fl_file_chooser("保存先を選んでね", "*.txt", nullptr);
    if (!filename) return;

    std::ofstream file(filename);
    file << textbuf->text();
    isModified = false;

    current_filename = filename;
    update_status_bar();
}

void quit_cb(Fl_Widget*, void*) {
    if (isModified) {
        int response = fl_ask("未保存の変更があるよ。閉じていい？");
        if (response == 0) return;
    }
    exit(0);
}

void minimize_cb(Fl_Widget*, void*) {
    HWND hwnd = fl_xid(Fl::first_window());
    ShowWindow(hwnd, SW_MINIMIZE);
}

void maximize_cb(Fl_Widget*, void*) {
    HWND hwnd = fl_xid(mainWin);

    if (!isMaximized) {
        // ウィンドウの現在サイズ・位置を保存
        prevX = mainWin->x();
        prevY = mainWin->y();
        prevW = mainWin->w();
        prevH = mainWin->h();

        ShowWindow(hwnd, SW_MAXIMIZE);
        isMaximized = true;
        maximizeBtn->label("元にもどす");
    } else {
        ShowWindow(hwnd, SW_RESTORE);
        mainWin->resize(prevX, prevY, prevW, prevH);
        isMaximized = false;
        maximizeBtn->label("大きくする");
    }
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
    mainWin = win;

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

    maximizeBtn = new Fl_Button(610, 0, 70, 30, "大きくする");
    maximizeBtn->labelsize(16);
    maximizeBtn->callback(maximize_cb);
    maximizeBtn->color(fl_rgb_color(100, 149, 237));
    maximizeBtn->labelcolor(FL_WHITE);
    maximizeBtn->box(FL_FLAT_BOX);

    Fl_Menu_Bar *menu = new Fl_Menu_Bar(0, 30, 800, 25);
    menu->add("ふぁいる！/おーぷん", FL_CTRL + 'o', open_cb);
    menu->add("ふぁいる！/ほぞん", FL_CTRL + 's', save_cb);
    menu->add("ふぁいる！/とじる", FL_CTRL + 'q', quit_cb);

    textbuf = new Fl_Text_Buffer();
    textbuf->add_modify_callback(on_text_changed, nullptr);
    
    Fl_Group* editor_group = new Fl_Group(0, 55, 800, 520);
    editor_group->resizable();

    editor = new Fl_Text_Editor(0, 55, 800, 495);
    editor->buffer(textbuf);
    editor->color(fl_rgb_color(170, 255, 255));

    status_bar = new Fl_Box(0, 550, 800, 25);
    status_bar->labelfont(FL_COURIER);
    status_bar->labelsize(14);
    status_bar->label("ファイル: (無題) | 状態: 保存済み");
    status_bar->box(FL_FLAT_BOX);
    status_bar->color(fl_rgb_color(240, 240, 240));

    editor_group->end();

    win->resizable(editor_group);
    win->end();
    win->show(argc, argv);

    HWND hwnd = fl_xid(win);
    show_in_taskbar(hwnd);
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
    } else {
        MessageBox(NULL, "Failed to load .ico file.", "ERROR", MB_OK);
    }

    return Fl::run();
}