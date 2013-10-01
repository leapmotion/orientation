#ifndef __LeapListener_h__
#define __LeapListener_h__

#include <cinder/Thread.h>
#include "Leap.h"

class LeapListener : public Leap::Listener {
public:
  LeapListener();
  virtual void onInit( const Leap::Controller& );
  virtual void onConnect( const Leap::Controller& );
  virtual void onDisconnect( const Leap::Controller& );
  virtual void onFrame( const Leap::Controller& );
  bool WaitForFrame(Leap::Frame& curFrame, int millisecondsTimeout);
  bool IsConnected() const;
private:
  bool m_isConnected;
  Leap::Frame m_frame;
  std::mutex m_mutex;
  std::condition_variable m_condition;
};

#endif
