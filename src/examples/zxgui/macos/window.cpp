#include "screen.h"
#include "window.h"
#include <QtWidgets>


Window::Window()
{
    auto *mainLayout = new QGridLayout;
    screen = new Screen(this);

    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->addWidget(screen, 0, 0, 1, 1);
    setLayout(mainLayout);

    setWindowTitle(tr("ZX Raspberry menu system test application"));
    screen->setFocus();
}
