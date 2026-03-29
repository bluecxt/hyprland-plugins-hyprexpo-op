#include "ExpoGesture.hpp"

#include "overview.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/helpers/Monitor.hpp>

void CExpoGesture::begin(const ITrackpadGesture::STrackpadGestureBegin& e) {
    ITrackpadGesture::begin(e);

    m_lastDelta   = 0.F;
    m_firstUpdate = true;

    if (!g_pOverview)
        g_pOverview = std::make_unique<COverview>(Desktop::focusState()->monitor()->m_activeWorkspace);
    else {
        g_pOverview->selectHoveredWorkspace();
        g_pOverview->setClosing(true);
    }
}

void CExpoGesture::update(const ITrackpadGesture::STrackpadGestureUpdate& e) {
    if (m_firstUpdate) {
        m_firstUpdate = false;
        return;
    }

    if (!g_pOverview)
        return;

    m_lastDelta += distance(e);

    if (m_lastDelta <= 0.01) // plugin will crash if swipe ends at <= 0
        m_lastDelta = 0.01;

    g_pOverview->onSwipeUpdate(m_lastDelta);
}

void CExpoGesture::end(const ITrackpadGesture::STrackpadGestureEnd& e) {
    if (!g_pOverview)
        return;

    g_pOverview->setClosing(false);
    g_pOverview->onSwipeEnd();
}

extern SDispatchResult onMoveActiveDispatcher(std::string arg);
extern bool            renderingOverview;

void CSwipeGesture::begin(const ITrackpadGesture::STrackpadGestureBegin& e) {
    ITrackpadGesture::begin(e);
    m_delta = 0.F;
    m_dir   = e.direction;

    if (!g_pOverview) {
        renderingOverview = true;
        g_pOverview       = std::make_unique<COverview>(Desktop::focusState()->monitor()->m_activeWorkspace, false, true);
        renderingOverview = false;
    }
}

void CSwipeGesture::update(const ITrackpadGesture::STrackpadGestureUpdate& e) {
    if (g_pOverview) {
        float    d    = distance(e);
        Vector2D move = {0, 0};
        if (m_dir == TRACKPAD_GESTURE_DIR_UP)
            move.y = d;
        else if (m_dir == TRACKPAD_GESTURE_DIR_DOWN)
            move.y = -d;
        else if (m_dir == TRACKPAD_GESTURE_DIR_LEFT)
            move.x = d;
        else if (m_dir == TRACKPAD_GESTURE_DIR_RIGHT)
            move.x = -d;

        g_pOverview->onNavigationSwipeUpdate(move);
    }
}

void CSwipeGesture::end(const ITrackpadGesture::STrackpadGestureEnd& e) {
    if (g_pOverview)
        g_pOverview->onNavigationSwipeEnd();
}
