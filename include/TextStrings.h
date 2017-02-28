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
    drawStrings(alpha, windowWidth, windowHeight, windowHeight/10.0f, m_whereStrings, m_locale);
  }
  void drawWhere3DStrings(float alpha, float windowWidth, float windowHeight) const {
    drawStrings(alpha, windowWidth, windowHeight, windowHeight/10.0f, m_where3DStrings, m_locale);
  }
  void drawWhatStrings(float alpha, float windowWidth, float windowHeight) const {
    drawStrings(alpha, windowWidth, windowHeight, windowHeight/10.0f, m_whatStrings, m_locale);
  }
  void drawHowStrings(float alpha, float windowWidth, float windowHeight) const {
    drawStrings(alpha, windowWidth, windowHeight, windowHeight/10.0f, m_howStrings, m_locale);
  }
  void drawContinueString(float alpha, float windowWidth, float windowHeight) const {
    drawStrings(alpha, windowWidth, windowHeight, windowHeight - windowHeight/20.0f, m_continueString, m_locale, 0.35f);
  }
  void drawFinishString(float alpha, float windowWidth, float windowHeight) const {
    drawStrings(alpha, windowWidth, windowHeight, windowHeight - windowHeight/20.0f, m_finishString, m_locale, 0.35f);
  }

private:

  static void drawStrings(float alpha, float windowWidth, float windowHeight, float drawY, const std::vector<std::wstring>& textStrings, const std::string& locale, float textScale = 1.0f) {
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
    const float DESIRED_TITLE_SIZE = 60.0f/titleScale;

    // set a bunch of constants
    const float centerX = windowWidth/2.0f;
    const float centerY = windowHeight/2.0f;
    const float resScale = windowWidth / DESIRED_WIDTH;
    const float curWidth = 0.55f * windowWidth;
    const float titleSize = textScale * DESIRED_TITLE_SIZE * resScale;
    const float instructionsSize = textScale * DESIRED_INSTRUCTIONS_SIZE * resScale;
    const float lineSpacing = 2.0f * resScale;
    const float rectangleHeight = 3.0f * resScale;
    const ci::Font titleFont("Arial", titleSize);
    const ci::Font instructionsFont("Arial", instructionsSize);
    const ci::ColorA titleColor(0.9f, 0.9f, 0.9f, alpha);
    const ci::ColorA instructionsColor(0.9f, 0.9f, 0.9f, alpha);
    const ci::ColorA lineColor(0.9f, 0.9f, 0.9f, alpha);
    const ci::Vec2f instructionsOffset(0.0f, 0.0f);
    const float height = titleSize + (numStrings-1)*instructionsSize + numStrings*lineSpacing;

    // iterate through the strings and draw them on separate lines
    ci::Vec2f curPos(centerX, drawY);
    for (std::size_t i=0; i<numStrings; i++) {
//    Cinder's ci::toUtf8() function doesn't work properly on Mac, use our own implementation:
      const std::string utf8str = convertWideStringToUTF8String(textStrings[i]);
      if (i == 0) {
        // draw the title string
        ci::gl::drawStringCentered(utf8str, curPos, titleColor, titleFont);
        curPos += ci::Vec2f(0, titleSize);
      } else {
        // draw the instruction strings
        ci::gl::drawStringCentered(utf8str, curPos + instructionsOffset, instructionsColor, instructionsFont);
        curPos += ci::Vec2f(0, instructionsSize);
        curPos += ci::Vec2f(0, lineSpacing);
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
      m_whereStrings.push_back(L"Aqui é onde ele vê");
      m_whereStrings.push_back(L"Acene suas mãos acima do dispositivo");
      m_where3DStrings.push_back(L"Explore o espaço");
      m_whatStrings.push_back(L"Isto é o que ele vê");
      m_whatStrings.push_back(L"Mova a mão sobre o dispositivo");
      m_howStrings.push_back(L"Assim é como se desenha");
      m_howStrings.push_back(L"Para desenhar: mova o dedo para frente");
      m_howStrings.push_back(L"Para parar: mova o dedo para trás");
      m_continueString.push_back(L"Aperte qualquer tecla para continuar");
      m_finishString.push_back(L"Aperte qualquer tecla para concluir");
    }
    else if (m_locale == "es") {
      m_whereStrings.push_back(L"Aquí es donde ve");
      m_whereStrings.push_back(L"Pasa tus manos sobre el dispositivo");
      m_where3DStrings.push_back(L"Explora el espacio");
      m_whatStrings.push_back(L"Esto es lo que ve");
      m_whatStrings.push_back(L"Mueve la mano sobre el dispositivo");
      m_howStrings.push_back(L"Así se dibuja");
      m_howStrings.push_back(L"Para dibujar: mueve el dedo hacia adelante");
      m_howStrings.push_back(L"Para detenerte: mueve el dedo hacia atrás");
      m_continueString.push_back(L"Pulsa cualquier tecla para continuar");
      m_finishString.push_back(L"Pulsa cualquier tecla para terminar");
    }
    else if (m_locale == "de") {
      m_whereStrings.push_back(L"Dies ist sein Sichtfeld");
      m_whereStrings.push_back(L"Bewegen Sie Ihre Hände über dem Gerät");
      m_where3DStrings.push_back(L"Entdecken Sie den Raum");
      m_whatStrings.push_back(L"Dies ist, was es sieht");
      m_whatStrings.push_back(L"Bewegen Sie Ihre Hand über dem Gerät");
      m_howStrings.push_back(L"Auf diese Art zeichnen Sie");
      m_howStrings.push_back(L"Um zu zeichnen: bewegen Sie den Finger nach vorne");
      m_howStrings.push_back(L"Um anzuhalten: bewegen Sie den Finger zurück");
      m_continueString.push_back(L"Drücken Sie eine beliebige Taste, um fortzufahren");
      m_finishString.push_back(L"Drücken Sie eine beliebige Taste, um es zu beenden");
    }
    else if (m_locale == "fr") {
      m_whereStrings.push_back(L"C'est là qu'il voie");
      m_whereStrings.push_back(L"Placez vos mains au-dessus de l'appareil");
      m_where3DStrings.push_back(L"Explorez l'espace");
      m_whatStrings.push_back(L"C'est ce qu'il voie");
      m_whatStrings.push_back(L"Déplacez votre main au-dessus de l'appareil");
      m_howStrings.push_back(L"C'est comme ça que vous tracez");
      m_howStrings.push_back(L"Pour tracer : déplacer votre doigt vers l'avant");
      m_howStrings.push_back(L"Pour arrêter : reculer votre doigt");
      m_continueString.push_back(L"Appuyez sur une touche pour continuer");
      m_finishString.push_back(L"Appuyez sur une touche pour terminer");
    }
    else if (m_locale == "zh") {
      m_whereStrings.push_back(L"这是它看到的地方");
      m_whereStrings.push_back(L"在设备上方摆动您的双手");
      m_where3DStrings.push_back(L"探索太空");
      m_whatStrings.push_back(L"这是它看到的景象");
      m_whatStrings.push_back(L"移动您的手划过设备上方");
      m_howStrings.push_back(L"这是您绘画的方法");
      m_howStrings.push_back(L"绘画：向前移动手指");
      m_howStrings.push_back(L"停止：向后移动手指");
      m_continueString.push_back(L"按下任意键继续");
      m_finishString.push_back(L"按下任意键完成");
    }
    else if (m_locale == "zh_TW") {
      m_whereStrings.push_back(L"這是它看見的範圍");
      m_whereStrings.push_back(L"在裝置上方揮動你的手");
      m_where3DStrings.push_back(L"探索這個空間");
      m_whatStrings.push_back(L"這是它看見的影像");
      m_whatStrings.push_back(L"在裝置上方晃動你的手");
      m_howStrings.push_back(L"這是你畫圖的方法");
      m_howStrings.push_back(L"畫圖：手指前移");
      m_howStrings.push_back(L"停止：手指後移");
      m_continueString.push_back(L"按任意鍵以繼續");
      m_finishString.push_back(L"按任意鍵以結束");
    }
    else if (m_locale == "ja") {
      m_whereStrings.push_back(L"どこを認識しているか");
      m_whereStrings.push_back(L"デバイスの上で手を振ってください");
      m_where3DStrings.push_back(L"空間を探ってください");
      m_whatStrings.push_back(L"何を認識しているか");
      m_whatStrings.push_back(L"デバイスの上で手を動かしてください");
      m_howStrings.push_back(L"描き方");
      m_howStrings.push_back(L"描く: 指を前に動かしてください");
      m_howStrings.push_back(L"止める: 指を後ろに動かしてください");
      m_continueString.push_back(L"続けるにはどれかのキーを押してください");
      m_finishString.push_back(L"終わるにはどれかのキーを押してください");
    }
    else if (m_locale == "ko") {
      m_whereStrings.push_back(L"이는 장치가 바라보는 공간입니다");
      m_whereStrings.push_back(L"장치 위에서 손을 흔드세요");
      m_where3DStrings.push_back(L"공간을 누벼 보세요");
      m_whatStrings.push_back(L"이는 장치가 바라보는 대상입니다");
      m_whatStrings.push_back(L"장치 위로 손을 움직여 보세요");
      m_howStrings.push_back(L"이는 그리기 방법입니다");
      m_howStrings.push_back(L"선 긋기: 손가락을 앞으로 움직임");
      m_howStrings.push_back(L"멈추기: 손가락을 뒤로 움직임");
      m_continueString.push_back(L"계속하시려면 아무 키나 누르세요");
      m_finishString.push_back(L"멈추시려면 아무 키나 누르세요");
    }
    else if (m_locale == "it") {
      m_whereStrings.push_back(L"Questa è l'area di visione");
      m_whereStrings.push_back(L"Muovere le mani sul controller");
      m_where3DStrings.push_back(L"Esplorare lo spazio");
      m_whatStrings.push_back(L"Questo è l'oggetto visto");
      m_whatStrings.push_back(L"Spostare la mano sul dispositivo");
      m_howStrings.push_back(L"Questo è il modo in cui disegnare");
      m_howStrings.push_back(L"Per disegnare: spostare il dito in avanti");
      m_howStrings.push_back(L"Per interrompere: spostare il dito indietro");
      m_continueString.push_back(L"Premere un tasto qualsiasi per continuare");
      m_finishString.push_back(L"Premere un tasto qualsiasi per terminare");
    }
    else { //default to English
      m_whereStrings.push_back(L"This is where it sees");
      m_whereStrings.push_back(L"Wave your hands above the device");
      m_where3DStrings.push_back(L"Explore the space");
      m_whatStrings.push_back(L"This is what it sees");
      m_whatStrings.push_back(L"Move hand over the device");
      m_howStrings.push_back(L"This is how you draw");
      m_howStrings.push_back(L"To draw: move finger forward");
      m_howStrings.push_back(L"To stop: move finger back");
      m_continueString.push_back(L"Press any key to continue");
      m_finishString.push_back(L"Press any key to finish");
    }
  }

  std::vector<std::wstring> m_whereStrings;
  std::vector<std::wstring> m_where3DStrings;
  std::vector<std::wstring> m_whatStrings;
  std::vector<std::wstring> m_howStrings;
  std::vector<std::wstring> m_continueString;
  std::vector<std::wstring> m_finishString;

  std::string m_locale;

};

#endif
