#pragma once

#include <hyprland/src/managers/input/trackpad/gestures/ITrackpadGesture.hpp>

class CExpoGesture : public ITrackpadGesture {
  public:
    CExpoGesture()          = default;
    virtual ~CExpoGesture() = default;

    virtual void begin(const ITrackpadGesture::STrackpadGestureBegin& e);
    virtual void update(const ITrackpadGesture::STrackpadGestureUpdate& e);
    virtual void end(const ITrackpadGesture::STrackpadGestureEnd& e);

  private:
    float m_lastDelta   = 0.F;
    bool  m_firstUpdate = false;
};

class CSwipeGesture : public ITrackpadGesture {
  public:
    CSwipeGesture()          = default;
    virtual ~CSwipeGesture() = default;

    virtual void begin(const ITrackpadGesture::STrackpadGestureBegin& e);
    virtual void update(const ITrackpadGesture::STrackpadGestureUpdate& e);
    virtual void end(const ITrackpadGesture::STrackpadGestureEnd& e);

  private:
    float                     m_delta      = 0.F;
    eTrackpadGestureDirection m_dir        = TRACKPAD_GESTURE_DIR_NONE;
    double                    m_cumulativeDelta = 0;
    double                    m_avgSpeed   = 0;
    int                       m_speedPoints = 0;
};
