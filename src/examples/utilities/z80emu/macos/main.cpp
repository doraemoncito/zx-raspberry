#include <QApplication>
#include <QTimer>
#include "window.h"
#include "shared/z80emu.h"
#include "shared/zexall.h"
#include "shared/zx48k_rom.h"

Z80emu z80emu = Z80emu();

Window *global_window = nullptr;

unsigned int elapsed = 0;

void execute() {
    if (elapsed++ > 30000) {
        global_window->updateScreen();
        elapsed = 0;
    }
    z80emu.run();
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window;
    global_window = &window;
    window.show();
    
    // https://stackoverflow.com/questions/36988826/running-code-in-the-main-loop
    auto timer = new QTimer(&window);
    // https://stackoverflow.com/questions/9428038/is-it-possible-to-connect-a-signal-to-a-static-slot-without-a-receiver-instance
    QEventLoop::connect(timer, &QTimer::timeout, execute);
    // z80emu.initialise(zexall_bin, zexall_bin_len);
    z80emu.initialise(zx48k_rom, zx48k_rom_len);
    timer->start();

    return app.exec();
}
