#ifndef SCREEN_H
#define SCREEN_H


#include <QImage>
#include <QWidget>
#include "shared/SpectrumScreen.h"

class CBcmFrameBuffer;

/* RENAME THIS CLASS TO EmulatorScreen */

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
    bool antialiased = false;

    // QImage image = QImage(296, 192, QImage::Format_Indexed8);
    CSpectrumScreen m_SpectrumScreen;
    CBcmFrameBuffer *bcmFrameBuffer = nullptr;

    QImage image = QImage(352, 296, QImage::Format_RGB32);
};


#endif // SCREEN_H
