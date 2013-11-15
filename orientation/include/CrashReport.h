/******************************************************************************\
* Copyright (C) 2012-2013 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#if USE_CRASH_REPORTING
namespace google_breakpad {
  class ExceptionHandler;
}
#endif

class CrashReport {
public:
  CrashReport();
  ~CrashReport();
#if USE_CRASH_REPORTING
private:
  google_breakpad::ExceptionHandler* m_ExceptionHandler;
#endif
};
