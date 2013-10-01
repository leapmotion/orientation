#include "LeapListener.h"
#include <cinder/app/AppBasic.h>
#include <boost/date_time.hpp>
#include <iostream>

using namespace ci;

LeapListener::LeapListener() : m_isConnected(false) { }

void LeapListener::onInit(const Leap::Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void LeapListener::onConnect(const Leap::Controller& controller) {
  std::cout << "Connected" << std::endl;
}

void LeapListener::onDisconnect(const Leap::Controller& controller) {
  std::cout << "Disconnected" << std::endl;
}

void LeapListener::onFrame(const Leap::Controller& controller) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_isConnected = true;
  m_frame = controller.frame();
  m_condition.notify_all();
}

bool LeapListener::WaitForFrame(Leap::Frame& curFrame, int millisecondsTimeout) {
  std::unique_lock<std::mutex> lock(m_mutex);
  if (curFrame.id() == m_frame.id() &&
    !m_condition.timed_wait(lock, boost::posix_time::milliseconds(millisecondsTimeout))) {
      return false;
  }
  curFrame = m_frame;
  return true;
}

bool LeapListener::IsConnected() const {
  return m_isConnected;
}
