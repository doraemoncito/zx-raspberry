#include "screen.h"
#include "window.h"
#include <QtWidgets>


Window::Window()
{
    screen = new Screen(this);
    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->addWidget(screen, 0, 0, 1, 1);
    setLayout(mainLayout);

    setWindowTitle(tr("ZX Spectrum Emulator"));
}

void Window::updateScreen() {
    screen->repaint();
}