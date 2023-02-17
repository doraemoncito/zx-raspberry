#ifndef SCREEN_H
#define SCREEN_H


#include <QImage>
#include <QWidget>
#include "common/zxdisplay.h"

class CBcmFrameBuffer;

class Screen : public QWidget
{
    Q_OBJECT

public:
    Screen(QWidget *parent = 0);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool antiAliased = false;

    // QImage image = QImage(296, 192, QImage::Format_Indexed8);
    ZxDisplay m_zxDisplay;

    // ZX Spectrum video memory image
    QImage zxSpectrumImage = QImage(256, 192, QImage::Format_RGB32);

    // Raspberry Pi framebuffer image
    QImage raspberryPiImage = QImage(352, 272, QImage::Format_RGB32);

    CBcmFrameBuffer *bcmFrameBuffer = nullptr;
};


#endif // SCREEN_H
