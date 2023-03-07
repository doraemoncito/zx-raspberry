//
// Created by José Hernández on 02/03/2020.
//

#ifndef ZX_DIALOG_H
#define ZX_DIALOG_H

#include "zxrect.h"
#include "zxgroup.h"

class ZxDialog : public ZxGroup {

    typedef ZxGroup super;

public:
    ZxDialog(ZxRect bounds, const char *title);

    void drawFrame(uint8_t *buffer, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    void draw(uint8_t *buffer) override;

private:
    const char *title;

};


#endif // ZX_DIALOG_H
