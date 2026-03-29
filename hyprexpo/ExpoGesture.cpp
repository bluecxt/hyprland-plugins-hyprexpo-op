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
    m_delta           = 0.F;
    m_dir             = e.direction;
    m_cumulativeDelta = 0;
    m_avgSpeed        = 0;
    m_speedPoints     = 0;

    if (!g_pOverview) {
        renderingOverview = true;
        g_pOverview       = std::make_unique<COverview>(Desktop::focusState()->monitor()->m_activeWorkspace, false, true);
        renderingOverview = false;
    }
}

void CSwipeGesture::update(const ITrackpadGesture::STrackpadGestureUpdate& e) {
    if (!g_pOverview || !e.swipe)
        return;

    static auto PSWIPEDIST = CConfigValue<Hyprlang::INT>("gestures:workspace_swipe_distance");
    static auto PSWIPEINVR = CConfigValue<Hyprlang::INT>("gestures:workspace_swipe_invert");

    const auto  SWIPEDISTANCE = std::clamp(*PSWIPEDIST, (int64_t)1, (int64_t)UINT32_MAX);

    // Get movement in the direction of the gesture
    float       d = 0;
    if (m_dir == TRACKPAD_GESTURE_DIR_LEFT || m_dir == TRACKPAD_GESTURE_DIR_RIGHT)
        d = e.swipe->delta.x;
    else
        d = e.swipe->delta.y;

    if (*PSWIPEINVR)
        d *= -1.0;

    m_avgSpeed = (m_avgSpeed * m_speedPoints + abs(d)) / (m_speedPoints + 1);
    m_speedPoints++;

    m_cumulativeDelta += d;
    m_cumulativeDelta = std::clamp(m_cumulativeDelta, (double)-SWIPEDISTANCE, (double)SWIPEDISTANCE);

    Vector2D cumulativeVector = {0, 0};
    if (m_dir == TRACKPAD_GESTURE_DIR_LEFT || m_dir == TRACKPAD_GESTURE_DIR_RIGHT)
        cumulativeVector.x = m_cumulativeDelta;
    else
        cumulativeVector.y = m_cumulativeDelta;

    g_pOverview->onNavigationSwipeUpdate(cumulativeVector);
}

void CSwipeGesture::end(const ITrackpadGesture::STrackpadGestureEnd& e) {
    if (!g_pOverview)
        return;

    static auto PSWIPEPERC = CConfigValue<Hyprlang::FLOAT>("gestures:workspace_swipe_cancel_ratio");
    static auto PSWIPEDIST = CConfigValue<Hyprlang::INT>("gestures:workspace_swipe_distance");
    static auto PSWIPEFORC = CConfigValue<Hyprlang::INT>("gestures:workspace_swipe_min_speed_to_force");

    const auto  SWIPEDISTANCE = std::clamp(*PSWIPEDIST, (int64_t)1, (int64_t)UINT32_MAX);

    bool        commit = true;

    if ((abs(m_cumulativeDelta) < SWIPEDISTANCE * *PSWIPEPERC && (*PSWIPEFORC == 0 || m_avgSpeed < *PSWIPEFORC)) || abs(m_cumulativeDelta) < 2) {
        commit = false;
    }

    Vector2D finalDecision = {0, 0};
    if (commit) {
        if (m_dir == TRACKPAD_GESTURE_DIR_LEFT || m_dir == TRACKPAD_GESTURE_DIR_RIGHT)
            finalDecision.x = m_cumulativeDelta > 0 ? 100 : -100; // Sign indicates direction
        else
            finalDecision.y = m_cumulativeDelta > 0 ? 100 : -100;
    }

    g_pOverview->onNavigationSwipeEnd(finalDecision);
}
