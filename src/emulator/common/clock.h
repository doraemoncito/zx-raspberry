/*
 * Copyright (c) 2020-2023 Jose Hernandez
 *
 * This file is part of ZxRaspberry.
 *
 * ZxRaspberry is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ZxRaspberry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ZxRaspberry.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef ZXRASPBERRY_CLOCK_H
#define ZXRASPBERRY_CLOCK_H


#include <memory>
#include "common/hardware/zxhardwaremodel.h"


class Clock {

private:
    Clock() : m_spectrumModel(nullptr), m_tstates(0), m_frames(0) {};

    ZxHardwareModel *m_spectrumModel;
    uint32_t m_tstates;
    uint32_t m_frames;

public:
    static Clock &getInstance() {

        static Clock INSTANCE = Clock();
        return INSTANCE;
    }


    void setSpectrumModel(ZxHardwareModel *spectrumModel) {

        m_spectrumModel = spectrumModel;
        reset();
    }


    [[nodiscard]] uint32_t getTstatesPerScreenFrame() const {

        return m_spectrumModel->tStatesPerScreenFrame();
    }


    [[nodiscard]] uint32_t getTstatesPerScreenLine() const {

        return m_spectrumModel->tStatesPerScreenLine();
    }


    [[nodiscard]] uint32_t getTstates() const {

        return m_tstates;
    }


    void setTstates(uint32_t states) {

        m_tstates = (states > m_spectrumModel->tStatesPerScreenFrame()) ? 0 : states;
        m_frames = 0;
    }


    void addTstates(uint32_t states) {

        m_tstates += states;
    }


    [[nodiscard]] long getFrames() const {

        return m_frames;
    }


    void endFrame() {
        assert(m_tstates >= m_spectrumModel->tStatesPerScreenFrame());
        m_frames++;
        m_tstates -= m_spectrumModel->tStatesPerScreenFrame();
    }


    [[nodiscard]] uint64_t getAbsTstates() const {

        return m_frames * m_spectrumModel->tStatesPerScreenFrame() + m_tstates;
    }


    void reset() {

        m_frames = m_tstates = 0;
    }

};


#endif //ZXRASPBERRY_CLOCK_H
