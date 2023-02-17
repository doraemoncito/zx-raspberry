#include "screen.h"
#include "window.h"
#include <QtWidgets>


Window::Window()
{
    screen = new Screen;
    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->addWidget(screen, 0, 0, 1, 1);
    setLayout(mainLayout);

    setWindowTitle(tr("Text drawing using the ZX Spectrum character set"));
}
