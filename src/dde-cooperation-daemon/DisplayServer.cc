#include "DisplayServer.h"

#include <spdlog/spdlog.h>

#include "Manager.h"

DisplayServer::DisplayServer(Manager *manager)
    : m_manager(manager)
    , m_startEdgeDetection(false) {
}

void DisplayServer::handleScreenSizeChange(int16_t w, int16_t h) {
    m_screenWidth = w;
    m_screenHeight = h;
}

void DisplayServer::handleMotion(int16_t x, int16_t y) {
    do {
        if (m_lastX == x) {
            if (x == 0) {
                // left flow
                spdlog::info("left flow");
                flowOut(FlowDirection::Left, 0, y);
                break;
            }

            if (x == m_screenWidth - 1) {
                // right flow
                spdlog::info("right flow");
                flowOut(FlowDirection::Right, 0, y);
                break;
            }
        }

        if (m_lastY == y) {
            if (y == 0) {
                // top flow
                spdlog::info("top flow");
                flowOut(FlowDirection::Top, x, 0);
                break;
            }

            if (y == m_screenHeight - 1) {
                // bottom flow
                spdlog::info("bottom flow");
                flowOut(FlowDirection::Bottom, x, 0);
                break;
            }
        }
    } while (false);

    m_lastX = x;
    m_lastY = y;
}

void DisplayServer::flowBack(uint16_t direction, uint16_t x, uint16_t y) {
    switch (direction) {
    case 3: {
        // left
        x = m_screenWidth;
        break;
    }
    }

    moveMouse(x, y);
    hideMouse(false);
    startEdgeDetection();
}

void DisplayServer::flowOut(uint16_t direction, uint16_t x, uint16_t y) {
    bool r = m_manager->tryFlowOut(direction, x, y);
    if (r) {
        stopEdgeDetection();
        hideMouse(true);
    }
}