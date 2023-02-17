#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class Screen;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private slots:

private:
    Screen *screen;
};

#endif // WINDOW_H
