// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#if !defined(__I18N_h__)
#define __I18N_h__

#include <string>

class I18N {
  public:
    static std::string getUserLocale();

  private:
    I18N();
    ~I18N();
};

#endif // __I18N_h__
