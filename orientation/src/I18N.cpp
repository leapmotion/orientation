// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "I18N.h"

#if __APPLE__
#include <Foundation/NSArray.h>
#include <Foundation/NSBundle.h>
#include <Foundation/NSString.h>
#elif _WIN32
#include <Windows.h>
#else
#include <stdlib.h>
#endif

std::string I18N::getUserLocale()
{
#if __APPLE__
  NSArray* locales = [[NSBundle mainBundle] preferredLocalizations];
  if ([locales count] > 0) {
    NSString* locale = [locales objectAtIndex:0];
    return std::string([locale UTF8String]);
  }
  return "en";
#elif _WIN32
  const LCID lcid = MAKELCID( GetUserDefaultUILanguage(), SORT_DEFAULT );
  char value[6];
  int len;
  std::string locale;

  if ((len = GetLocaleInfoA(lcid, LOCALE_SNAME,
                            reinterpret_cast<LPSTR>(&value), sizeof(value))) > 0) {
    for (int i = 0; i < len; i++) {
      if (value[i] == '-') {
        value[i] = '_'; // Use the format en_US instead of en-US (which is what these functions return)
      }
    }
  } else if ((len = GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME,
                                   reinterpret_cast<LPSTR>(&value), sizeof(value))) > 0) {
    if (len <= (sizeof(value) - 3)) { // Expecting on 2 characters (plus null) for the ISO-3166 country code
      value[len-1] = '_';
      int ctryLen = GetLocaleInfoA(lcid, LOCALE_SISO3166CTRYNAME,
                                   reinterpret_cast<LPSTR>(&value[len]), sizeof(value) - len);
      if (ctryLen > 0) {
        len += ctryLen;
      } else {
        value[len-1] = '\0';
      }
    }

  } else {
    return "en";
  }
  if (value[len-1] == '\0') {
    len--;
  }
  return std::string(value, len);
#else
  const char* lang = getenv("LANG");
  if (lang) {
    std::string locale(lang);
    size_t pos;

    if ((pos = locale.find('.')) != std::string::npos) {
      locale.erase(pos); // Strip off encoding (e.g., UTF-8)
    }
    return locale;
  } else {
    return "en";
  }
#endif
}
