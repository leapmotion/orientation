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
  if (m_deviceID.empty()) {
    if (!controller.devices().isEmpty()) {
      std::string fullString = controller.devices()[0].toString();
      // the string from the API also has some extra text on it, so just retrieve the actual ID
      // keep this in sync with whatever DeviceImplementation::toString() does
      size_t pos = fullString.find(": ");
      if (pos == std::string::npos) {
        m_deviceID = fullString;
      } else {
        m_deviceID = fullString.substr(pos + 2);
      }
    }
  }
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

const std::string& LeapListener::GetDeviceID() const {
  return m_deviceID;
}
