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
  const std::string& GetDeviceID() const;
  bool IsPongo() const;
  bool IsHOPS() const;
  bool IsEmbedded() const;
private:
  void setupDeviceID( const Leap::Controller& );
private:
  bool m_isConnected;
  Leap::Frame m_frame;
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::string m_deviceID;
};

#endif
