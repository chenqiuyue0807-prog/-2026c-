#include "Widget.h"

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    // 例如设置窗口标题、大小等
    setWindowTitle("游戏主窗口");
    resize(1280, 900);
}