#ifndef __TextStrings_h__
#define __TextStrings_h__

#include <string>
#include <vector>
#include <cinder/gl/gl.h>
#include <cinder/Utilities.h>
#include "I18N.h"

class TextStrings {

public:

  TextStrings() {
    std::string syslocale = I18N::getUserLocale();
    if (syslocale != "zh_TW") { // Special-case Traditional Chinese; Use whole locale
      // Get first two characters of string e.g. returns 'en' for English, 'es' for Spanish, 'de' for German, etc.
      syslocale = syslocale.substr(0,2);
    }
    createStrings(syslocale);
  }

  void drawWhereStrings(float alpha, float windowWidth, float windowHeight) const {
    drawStrings(alpha, windowWidth, windowHeight, m_whereStrings, m_locale);
  }
  void drawWhatStrings(float alpha, float windowWidth, float windowHeight) const {
    drawStrings(alpha, windowWidth, windowHeight, m_whatStrings, m_locale);
  }
  void drawHowStrings(float alpha, float windowWidth, float windowHeight) const {
    drawStrings(alpha, windowWidth, windowHeight, m_howStrings, m_locale);
  }

private:

  static void drawStrings(float alpha, float windowWidth, float windowHeight, const std::vector<std::wstring>& textStrings, const std::string& locale) {
    static const float DESIRED_INSTRUCTIONS_SIZE = 24.0f;
    static const float DESIRED_WIDTH = 1000.0f;

    const size_t numStrings = textStrings.size();
    float titleScale = 1.0f;

    // Adjust the font size of the title based on how long the string is:
    if (numStrings >= 1) {
      const std::wstring& titleString = textStrings[0];
      float length = static_cast<float>(titleString.size()); // Determine the number of characters in the title

      if (locale == "ja" || locale == "zh" || locale == "zh_TW") {
        length *= 2; // Assume the strings are using double width characters
      }
      length /= 20.0f; // Strings of length 20 would be nice width font size of 72, so adjust accordingly
      if (length > 1.0f) {
        titleScale = length; // Don't go beyond 72
      }
    }
    const float DESIRED_TITLE_SIZE = 72.0f/titleScale;

    // set a bunch of constants
    const float centerX = windowWidth/2.0f;
    const float centerY = windowHeight/2.0f;
    const float resScale = windowWidth / DESIRED_WIDTH;
    const float curWidth = 0.55f * windowWidth;
    const float titleSize = DESIRED_TITLE_SIZE * resScale;
    const float instructionsSize = DESIRED_INSTRUCTIONS_SIZE * resScale;
    const float lineSpacing = 2.0f * resScale;
    const float rectangleHeight = 3.0f * resScale;
    const Font titleFont("Arial", titleSize);
    const Font instructionsFont("Arial", instructionsSize);
    const ci::ColorA titleColor(0.9f, 0.9f, 0.9f, alpha);
    const ci::ColorA instructionsColor(0.9f, 0.9f, 0.9f, alpha);
    const ci::ColorA lineColor(0.9f, 0.9f, 0.9f, alpha);
    const Vec2f instructionsOffset(15.0f * resScale, 0.0f);
    const float height = titleSize + (numStrings-1)*instructionsSize + numStrings*lineSpacing;

    // iterate through the strings and draw them on separate lines
    Vec2f curPos(centerX - curWidth/2.0f, centerY - height/2.0f);
    for (std::size_t i=0; i<numStrings; i++) {
//    Cinder's ci::toUtf8() function doesn't work properly on Mac, use our own implementation:
      const std::string utf8str = convertWideStringToUTF8String(textStrings[i]);
      if (i == 0) {
        // draw the title string
        ci::gl::drawString(utf8str, curPos, titleColor, titleFont);
        curPos += Vec2f(0, titleSize);

        // draw a rectangle underneath the title
        gl::color(lineColor);
        const Rectf r(centerX - curWidth/2.0f, curPos.y - rectangleHeight - 3.0f*lineSpacing, centerX + curWidth/2.0f, curPos.y - 3.0f*lineSpacing);
        curPos += Vec2f(0, rectangleHeight);
        ci::gl::drawSolidRect(r);
        curPos += Vec2f(0, lineSpacing);
      } else {
        // draw the instruction strings
        ci::gl::drawString(utf8str, curPos + instructionsOffset, instructionsColor, instructionsFont);
        curPos += Vec2f(0, instructionsSize);
        curPos += Vec2f(0, lineSpacing);
      }
    }
  }

  static std::string convertWideStringToUTF8String(const std::wstring& wide) {
    std::string utf8;
    size_t wide_size = wide.size();
    const wchar_t* wide_cstr = wide.c_str();

    try {
      for (size_t i = 0; i < wide_size; i++) {
        unsigned int unicode = static_cast<unsigned int>(*wide_cstr++);

        if (unicode <=  0x7F) {
          utf8 += static_cast<char>(unicode);
        } else if (unicode <= 0x7FF) {
          utf8 += static_cast<char>(0xC0 | (unicode >> 6));
          utf8 += static_cast<char>(0x80 | (unicode & 0x3F));
        } else if (unicode >= 0xD800 && unicode <= 0xDBFF) {
          if (++i >= wide_size) { // Surrogate pairs come in pairs, it must be there
            throw std::exception();
          }
          unsigned int pair = static_cast<unsigned int>(*wide_cstr++);
          if (pair < 0xDC00 || pair > 0xDFFF) {
            throw std::exception(); // And the pair must fall within the surrogate pair range
          }
          // Combine the surrogate pairs
          unicode = 0x10000 + ((unicode - 0xD800) << 10) + (pair - 0xDC00);
          utf8 += static_cast<char>(0xF0 |  (unicode >> 18));
          utf8 += static_cast<char>(0x80 | ((unicode >> 12) & 0x3F));
          utf8 += static_cast<char>(0x80 | ((unicode >>  6) & 0x3F));
          utf8 += static_cast<char>(0x80 |  (unicode        & 0x3F));
        } else if (unicode <= 0xFFFF) {
          utf8 += static_cast<char>(0xE0 |  (unicode >> 12));
          utf8 += static_cast<char>(0x80 | ((unicode >>  6) & 0x3F));
          utf8 += static_cast<char>(0x80 |  (unicode        & 0x3F));
        } else if (unicode <= 0x10FFFF) {
          utf8 += static_cast<char>(0xF0 |  (unicode >> 18));
          utf8 += static_cast<char>(0x80 | ((unicode >> 12) & 0x3F));
          utf8 += static_cast<char>(0x80 | ((unicode >>  6) & 0x3F));
          utf8 += static_cast<char>(0x80 |  (unicode        & 0x3F));
        } else {
          throw std::exception();
        }
      }
    } catch (...) {}
    return utf8;
  }

  void createStrings(const std::string& syslocale) {
    m_locale = syslocale;
    m_whereStrings.clear();
    m_whatStrings.clear();
    m_howStrings.clear();

    if (m_locale == "pt") {
      m_whereStrings.push_back(L"Esse é o raio de visão...");
      m_whereStrings.push_back(L"para começar: mova sua mão sobre o dispositivo");
      m_whereStrings.push_back(L"para continuar: remova suas mãos");
      m_whatStrings.push_back(L"Isso é o que é visto...");
      m_whatStrings.push_back(L"para começar: mova sua mão sobre o dispositivo");
      m_whatStrings.push_back(L"para continuar: remova suas mãos");
      m_howStrings.push_back(L"É assim que você desenha.");
      m_howStrings.push_back(L"para desenhar: mova seu dedo para a frente");
      m_howStrings.push_back(L"para parar: mova seu dedo para trás");
      m_howStrings.push_back(L"para continuar: remova suas mãos");
    }
    else if (m_locale == "es") {
      m_whereStrings.push_back(L"Esto es donde lo ve...");
      m_whereStrings.push_back(L"para comenzar: mueva su mano sobre el dispositivo");
      m_whereStrings.push_back(L"para continuar: elimine sus manos");
      m_whatStrings.push_back(L"Esto es lo que ve...");
      m_whatStrings.push_back(L"para comenzar: mueva su mano sobre el dispositivo");
      m_whatStrings.push_back(L"para continuar: elimine sus manos");
      m_howStrings.push_back(L"Así es cómo dibuja...");
      m_howStrings.push_back(L"para dibujar: mueva su dedo hacia adelante");
      m_howStrings.push_back(L"para detener: mueva su dedo para atrás");
      m_howStrings.push_back(L"para continuar: elimine sus manos");
    }
    else if (m_locale == "de") {
      m_whereStrings.push_back(L"Hier wird angezeigt, wo es sieht...");
      m_whereStrings.push_back(L"Zum Starten: Hand über dem Gerät schwenken");
      m_whereStrings.push_back(L"Zum Fortfahren: Hände entfernen");
      m_whatStrings.push_back(L"Hier wird angezeigt, was es sieht...");
      m_whatStrings.push_back(L"Zum Starten: Hand über das Gerät bewegen");
      m_whatStrings.push_back(L"Zum Fortfahren: Hände entfernen");
      m_howStrings.push_back(L"So zeichnen Sie...");
      m_howStrings.push_back(L"Zum Zeichnen: Finger vorbewegen");
      m_howStrings.push_back(L"Zum Stoppen: Finger zurückbewegen");
      m_howStrings.push_back(L"Zum Fortfahren: Hände entfernens");
    }
    else if (m_locale == "fr") {
      m_whereStrings.push_back(L"Ceci est le point de vision...");
      m_whereStrings.push_back(L"Pour commencer : passez votre main au-dessus du périphérique");
      m_whereStrings.push_back(L"Pour poursuivre : retirez vos mains");
      m_whatStrings.push_back(L"Ceci est ce qu'il voit...");
      m_whatStrings.push_back(L"Pour commencer : passez votre main au-dessus du périphérique");
      m_whatStrings.push_back(L"Pour poursuivre : retirez vos mains");
      m_howStrings.push_back(L"Ceci est la méthode de dessin...");
      m_howStrings.push_back(L"Pour dessiner : déplacez votre doigt vers l'avant");
      m_howStrings.push_back(L"Pour arrêter : faites revenir votre doigt en arrière");
      m_howStrings.push_back(L"Pour poursuivre : retirez vos mains");
    }
    else if (m_locale == "zh") {
      m_whereStrings.push_back(L"这是它所看到的空间...");
      m_whereStrings.push_back(L"开始: 在设备上方挥手");
      m_whereStrings.push_back(L"继续: 移开手");
      m_whatStrings.push_back(L"这是它所看到的内容...");
      m_whatStrings.push_back(L"开始: 将手移动到设备上方");
      m_whatStrings.push_back(L"继续: 移开手");
      m_howStrings.push_back(L"这是你作画的方式...");
      m_howStrings.push_back(L"下笔: 向前移动手指");
      m_howStrings.push_back(L"提笔: 往后移动手指");
      m_howStrings.push_back(L"继续: 移开手");
    }
    else if (m_locale == "zh_TW") {
      m_whereStrings.push_back(L"這是它所看到的空間...");
      m_whereStrings.push_back(L"開始: 在設備上方揮手");
      m_whereStrings.push_back(L"繼續: 移開手");
      m_whatStrings.push_back(L"這是它所看到的內容...");
      m_whatStrings.push_back(L"開始: 將手移動到設備上方");
      m_whatStrings.push_back(L"繼續: 移開手");
      m_howStrings.push_back(L"這是你作畫的方式...");
      m_howStrings.push_back(L"下筆: 向前移動手指");
      m_howStrings.push_back(L"提筆: 往後移動手指");
      m_howStrings.push_back(L"繼續: 移開手");
    }
    else if (m_locale == "ja") {
      m_whereStrings.push_back(L"認識範囲を示します 。。。");
      m_whereStrings.push_back(L"開始するには：デバイスの上方で手を振ります");
      m_whereStrings.push_back(L"続行するには：両手を引いて遠ざけます");
      m_whatStrings.push_back(L"認識対象を示します。。。");
      m_whatStrings.push_back(L"開始するには：デバイスの上方で手を動かします");
      m_whatStrings.push_back(L"続行するには：両手を引いて遠ざけます");
      m_howStrings.push_back(L"描画方法を示します。。。");
      m_howStrings.push_back(L"描画するには：指を前に動かします");
      m_howStrings.push_back(L"停止するには：指を後ろに動かします");
      m_howStrings.push_back(L"続行するには：両手を引いて遠ざけます");
    }
    else { //default to English
      m_whereStrings.push_back(L"this is where it sees...");
      m_whereStrings.push_back(L"to begin: wave hand over device");
      m_whereStrings.push_back(L"to continue: remove hands");
      m_whatStrings.push_back(L"this is what it sees...");
      m_whatStrings.push_back(L"to begin: move hand over device");
      m_whatStrings.push_back(L"to continue: remove hands");
      m_howStrings.push_back(L"this is how you draw...");
      m_howStrings.push_back(L"to draw: move finger forward");
      m_howStrings.push_back(L"to stop: move finger back");
      m_howStrings.push_back(L"to continue: remove hands");
    }
  }

  std::vector<std::wstring> m_whereStrings;
  std::vector<std::wstring> m_whatStrings;
  std::vector<std::wstring> m_howStrings;

  std::string m_locale;

};

#endif
