#ifndef WINDOW_H
#define WINDOW_H

#include <QDebug>
#include <QKeyEvent>
#include <QWidget>

class Screen;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private:
    Screen *screen;
};

#endif // WINDOW_H
