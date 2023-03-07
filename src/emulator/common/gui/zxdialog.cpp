//
// Created by José Hernández on 02/03/2020.
//

#include <cstdint>
#include <cstring>
#include <cstdio>
#include "zxdialog.h"
#include "zxview.h"

ZxDialog::ZxDialog(ZxRect const bounds, const char *title) : ZxGroup(bounds), title(title) {
}

void ZxDialog::drawFrame(uint8_t *buffer, unsigned int x, unsigned int y, unsigned int width, unsigned int height) {

    uint8_t ink = 0xA;
    uint8_t paper = 0xF;

    // Top line
    char line[44 /* characters */ + 1 /* string terminator */] = {0};
    snprintf(line, 44 + 1, "\xAF\xAF %s \xAB", title);
    for (unsigned int x1 = strlen(line); x1 < width; x1++)
        line[x1] = '\xA3';
    line[width - 1] = '\xA7';
    line[width] = 0x00;
    printText(buffer, x, y, ink, paper, reinterpret_cast<char const *>(line));

    // Bottom line
    line[0] = '\xAE';
    for (unsigned int x1 = 1; x1 < width; x1++)
        line[x1] = '\xAC';
    line[width - 1] = '\xAD';
    line[width] = 0x00;
    printText(buffer, x, y + height - 1, ink, paper, line);

    for (unsigned int f=0; f < width; f++)
        line[f] = ' ';
    line[0] = '\xAA';
    line[width - 1] = '\xA5';
    line[width] = '\x00';
    for (unsigned y1 = y + 1; y1 < y + height - 1; y1++) {
        printText(buffer, x, y1, ink, paper, line);
//        printText(buffer, x, y1, ink, paper, "\xAA");
//        printText(buffer, x + width - 1, y1, ink, paper, "\xA5");
    }
}


void ZxDialog::draw(uint8_t *buffer) {
    drawFrame(buffer, m_bounds.getAx(), m_bounds.getAy(), m_bounds.getWidth(), m_bounds.getHeight());
    super::draw(buffer);
}
