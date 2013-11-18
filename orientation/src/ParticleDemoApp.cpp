#include "ParticleDemoApp.h"
#if __APPLE__
// for fullscreen resolution modification
#include <CoreGraphics/CGDirectDisplay.h>
#include <Carbon/Carbon.h>
#include <mach-o/dyld.h>
#endif

#ifdef _WIN32
#include "SOP.hpp"
#include <Windows.h>
#endif

#include <cinder/Url.h>
#include <cinder/Base64.h>

#include "CrashReport.h"

static bool DISABLE_MIXPANEL = false;

static std::string transformBytes(const std::string& bytes)
{
  size_t n = bytes.size();
  unsigned char mask[7] = {0xac, 0xd9, 0xc5, 0xb6, 0xeb, 0xf6, 0xbd};
  std::string trans;

  trans.reserve(n);
  for (size_t i = 0; i < n; i++) {
    trans.push_back(bytes[i] ^ mask[i % 7] ^ (i*101 & 0x7F));
  }
  return trans;
}

static std::string getMixPanelToken()
{
#if DEBUG
  //64a624e0f5fd5fec35dff6b08281664e
  return transformBytes("\x9A\x88\xEE\xAF\xCD\xBB\x86\xDF\x97\xFD\xA2\xD8\xFF\xFA\xCF\xD1\xA6\xB6\x95\xEF\xBF\xD3\x95\xE6\xF6\x84\x8C\xAB\x96\x9E\xA7\xE8");
#else
  //77d363605f0470115eb82352f14b2981
  return transformBytes("\x9B\x8B\xEB\xAA\xC9\xBC\xD5\xDF\xC4\xAE\xF4\x88\xFD\xAC\x9B\x83\xA0\xE6\x93\xB1\xEB\xD6\xC2\xE4\xA8\x87\x80\xF8\x92\x91\xAB\xBC");
#endif
}

static void SendMixPanelJSON(const std::string &jsonData)
{
  const std::string mpBaseURL("http://api.mixpanel.com/track/?data=");
  std::string encodedData = cinder::toBase64(jsonData);
  std::string requestString = mpBaseURL + encodedData;
  try {
    cinder::IStreamUrl::create(cinder::Url(requestString));
  } catch(...) { }
}

static void SendMixPanelEvent(const std::string &eventName, const std::string &deviceID, const std::string &data = "")
{
  if (DISABLE_MIXPANEL) {
    return;
  }

  std::string json = "{ \"event\": \"" + eventName + "\", \"properties\": {";
  
  static std::string distinct_id;
#ifdef _WIN32
  
  if( distinct_id.empty() ) {
    HKEY key;
    if( RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\LeapMotion"), &key) == ERROR_SUCCESS ) {
      char val[64];
      val[0] = 0; //if we fail, make this a null string;
      DWORD valLen = 64;
      RegGetValue(key, NULL, TEXT("MixPanelGUID"), NULL, NULL, (LPBYTE)&val, &valLen);
      distinct_id = std::string(val);
    }
  }

  json.append("\"distinct_id\": \"" + distinct_id + "\",");
#elif __APPLE__
  if( distinct_id.empty() ) {
    ci::fs::path guidPath = boost::filesystem::path("/Library/Application Support/Leap Motion/mpguid");
    ci::IStreamFileRef guidFileStream = ci::loadFileStream(guidPath);
    distinct_id = guidFileStream->readLine();
  }

  if( !distinct_id.empty() ) {
    json.append("\"distinct_id\": \"" + distinct_id + "\",");
  }
#endif
  
  json.append("\"token\": \""+ getMixPanelToken() + "\",");

  json.append("\"Device ID\": \"" + deviceID + "\"");

  if( !data.empty() ) {
    json.append(", " + data);
  }

  SendMixPanelJSON( json + "} }");
}

const double ParticleDemoApp::FADE_TIME = Utils::TIME_BETWEEN_PEAKS;
const double ParticleDemoApp::MAX_STROKE_TIME_LENGTH = 7.0;

irrklang::ISound* ParticleDemoApp::createSoundResource(DataSourceRef ref, const char* name) {
  // Obtain the buffer backing the loaded resource:
  if ( m_soundEngine ) {
    auto& buf = ref->getBuffer();

    // Attempt to load the stream from the buffer on the data source:
    auto ss = m_soundEngine->addSoundSourceFromMemory(
                                                      buf.getData(),
                                                      buf.getDataSize(),
                                                      name,
                                                      false
                                                      );

    m_audioSourceRefs.push_back(ref);
    ss->setForcedStreamingThreshold(300000);

    // Done loading the stream, return the source:
    return m_soundEngine->play2D(ss, true, true, true, (bool)irrklang::ESM_NO_STREAMING);
  }

  return NULL;
};

ParticleDemoApp::ParticleDemoApp()
{
#ifdef _WIN32
  // prevents multiple instances of a Windows application from
  // spawning as per: http://stackoverflow.com/questions/8799646/preventing-multiple-instances-of-my-application
  // needs to be done before any window initialization is done.
  // if called from setup and exit done with quit() too much init happens
  // and a window appears before process terminates.
  // this is earlier & faster with no visual side effects.
  CreateSemaphoreA(NULL, TRUE, TRUE, "LeapMotionOrientation");
  if (GetLastError() == ERROR_ALREADY_EXISTS)
  {
    exit(0);
  }
#endif
  m_skipStage = -1;
}

void ParticleDemoApp::setup() {
  // osx path stuff
#ifdef __APPLE__

  char exec_path[PATH_MAX] = {0};
  uint32_t pathSize = sizeof(exec_path);
  if (!_NSGetExecutablePath(exec_path, &pathSize)) {
    char fullpath[PATH_MAX] = {0};
    if (realpath(exec_path, fullpath)) {
      std::string path(fullpath);
      size_t pos = path.find_last_of('/');

      if (pos != std::string::npos) {
        path.erase(pos+1);
      }
      if (!path.empty()) {
        chdir(path.c_str());
      }
      const char* resources = "../Resources";
      if (!access(resources, R_OK)) {
        chdir(resources);
      }
    }
  }

#endif


// work around for enabling nvidia cards on auto-select Optimus machines.
#ifdef _WIN32
  int result = SOP_SetProfile("Leap Motion", "Orientation.exe");
  if (result == SOP_RESULT_CHANGE) {
    // Since the Optimus profiles are only checked once at application start, the
    // application has to be restarted now. Instead of starting the application again,
    // a hint could be displayed to the user that the changes only take effect after
    // the next start.

    // Call the current application without arguments from a new console window
    wchar_t command[1000];
    swprintf_s(command, L" /C %s", L"Orientation.exe");
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    CreateProcess(L"C:\\Windows\\System32\\cmd.exe", command, NULL, NULL, 0, 0, NULL, NULL, &si, &pi);

    exit(0);
  }
#endif

  // initial values for some variables
  m_draw3DScene = true;
  m_drawFluid = true;
  m_drawDrawing = false;
  m_drawHand = false;
  m_drawFingerGlowsOnly = false;
  m_haveNewFrame = false;
  m_fboIndex = 0;
  m_curImageNum = -1;
  m_curFadeMult = 0.0f;
  m_curImageAlpha = 1.0f;
  m_lastActivityTime = 0.0;
  m_frustumLinesAlpha = 0.0f;
  m_curNumHands = 0;
  m_blurMult = 0.25f;
  m_cameraMode = CAMERA_ORTHO;
  m_cameraTheta = 0.0f;
  m_cameraPhi = 0.1f;
  m_cameraZoom = 1.0f;
  m_cameraPos = Vec3f::zero();
  m_useNativeRes = false;

  // set up Leap
  m_listener = new LeapListener();
  m_leap.setPolicyFlags(Leap::Controller::POLICY_BACKGROUND_FRAMES);
  m_leap.addListener(*m_listener);

  // load images from resources
  m_glowTex = gl::Texture(loadImage(loadResource(RES_GLOW_PNG)));
  m_logoTex = gl::Texture(loadImage(loadResource(RES_LOGO_PNG)));
#if _WIN32
  if (isPongo()) {
    // EVENT app was started with embedded Leap
    SendMixPanelEvent("Orientation - App started (embedded)", m_listener->GetDeviceID());
    m_plugInTex = gl::Texture(loadImage(loadResource(RES_PLUG_IN_PNG_PONGO)));
  } else if (isHOPS()) {
    // EVENT app was started with embedded Leap
    SendMixPanelEvent("Orientation - App started (embedded)", m_listener->GetDeviceID());
    m_plugInTex = gl::Texture(loadImage(loadResource(RES_PLUG_IN_PNG_HOPS)));
  } else {
    // EVENT app was started with Leap peripheral
    SendMixPanelEvent("Orientation - App started (peripheral)", m_listener->GetDeviceID());
    m_plugInTex = gl::Texture(loadImage(loadResource(RES_PLUG_IN_PNG)));
  }
#else
  m_plugInTex = gl::Texture(loadImage(loadResource(RES_PLUG_IN_PNG)));
#endif

  // use additive blending and no depth buffer
  gl::enableAlphaBlending();
  gl::enableAdditiveBlending();
  gl::disableDepthRead();
  gl::disableDepthWrite();

  // set up glow FBO format
  m_motionFormat.setColorInternalFormat(GL_RGB8);
  m_glowFormat.enableMipmapping(false);
  m_glowFormat.enableDepthBuffer(false);

  // set up motion blur FBO format
  m_motionFormat.setColorInternalFormat(GL_RGB8);
  m_motionFormat.enableMipmapping(false);
  m_motionFormat.enableDepthBuffer(false);
  m_motionFormat.setMinFilter(GL_NEAREST);
  m_motionFormat.setMagFilter(GL_NEAREST);
  m_motionFormat.setSamples(4);

  // load shaders
  m_useFX = shouldUseFX(std::string((char*)glGetString(GL_VERSION)));

#ifdef __APPLE__
  // OSX uses a custom version of OpenGL 2.1 that supports FBOs.
  m_useFX = true;
#endif

  if (m_useFX) {
    try {
      m_shaderGlow = gl::GlslProg(loadResource(RES_GLOW_VERT), loadResource(RES_GLOW_FRAG));
      m_shaderMotion = gl::GlslProg(loadResource(RES_MOTION_VERT), loadResource(RES_MOTION_FRAG));
      m_fingerShader = gl::GlslProg(loadResource(RES_FINGER_VERT), loadResource(RES_FINGER_FRAG));
    } catch (gl::GlslProgCompileExc e) {
      std::cout << e.what() << std::endl;
      m_useFX = false;
    }
  }

  // set up hands and fluid
  m_handController = new HandController(&m_camera);
  m_handController->setShader( m_useFX ? &m_fingerShader : NULL );
  m_particleController = new ParticleController();

  // set up sounds
  m_soundEngine = irrklang::createIrrKlangDevice();
  if (!m_soundEngine) {
    std::cout << "Error loading sound engine" << std::endl;
    //quit();
  }

  // Create all of our sounds and set the initial volume to zero for all of them
  m_bassLoop = createSoundResource(loadResource(RES_BASS_OGG), "bass.ogg");
  m_rotationLoop = createSoundResource(loadResource(RES_ROTATE_OGG), "rotate.ogg");
  m_fluidLoop = createSoundResource(loadResource(RES_FLUID_OGG), "fluid.ogg");
  m_transitionLoop = createSoundResource(loadResource(RES_WAIT_OGG), "wait.ogg");
  m_handLoop = createSoundResource(loadResource(RES_HAND_OGG), "hand.ogg");
  m_drawingLoop = createSoundResource(loadResource(RES_DRAWING_OGG), "drawing.ogg");
  m_glowLoop = createSoundResource(loadResource(RES_GLOW_OGG), "glow.ogg");

  if ( m_bassLoop ) m_bassLoop->setVolume(0.0f);
  if ( m_rotationLoop ) m_rotationLoop->setVolume(0.0f);
  if ( m_fluidLoop ) m_fluidLoop->setVolume(0.0f);
  if ( m_transitionLoop ) m_transitionLoop->setVolume(0.0f);
  if( m_handLoop ) m_handLoop->setVolume(0.0f);
  if ( m_drawingLoop ) m_drawingLoop->setVolume(0.0f);

  gl::disableVerticalSync();

#if _WIN32
  HANDLE img = LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(RES_APP_ICON), IMAGE_ICON, 16, 16, LR_SHARED);
  SetClassLongPtr(getRenderer()->getHwnd(), GCLP_HICONSM, (LONG)img);
  SetClassLongPtr(getRenderer()->getHwnd(), GCLP_HICON, (LONG)img);
#endif

  std::cout << glGetString(GL_VENDOR) << std::endl;
  std::cout << glGetString(GL_RENDERER) << std::endl;
  std::cout << glGetString(GL_VERSION) << std::endl;
  std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  std::cout << glGetString(GL_EXTENSIONS) << std::endl;

  m_renderString = parseRenderString(std::string((char*)glGetString(GL_RENDERER)));

  DISABLE_MIXPANEL = m_visualizerOnlyMode;
}

void ParticleDemoApp::resize(ResizeEvent event) {
  int width = event.getWidth();
  int height = event.getHeight();

  if (width <= 0 || height <= 0) {
    return;
  }

  int w, h;
  getResolution(w, h);

  std::cout << "resize is called: " << width << ", " << height << std::endl;
  std::cout << "resolution is: " << w << ", " << h << std::endl;

  static const int DOWNSCALE_FACTOR = 2;

  if (m_useFX) {
    // setup our blur Fbo's, smaller ones will generate a bigger blur
    m_fboGlow1 = gl::Fbo(width/DOWNSCALE_FACTOR, height/DOWNSCALE_FACTOR, m_glowFormat);
    m_fboGlow1.bindFramebuffer();
    gl::clear(Color::black());
    m_fboGlow1.unbindFramebuffer();
    m_fboGlow2 = gl::Fbo(width/DOWNSCALE_FACTOR, height/DOWNSCALE_FACTOR, m_glowFormat);
    m_fboGlow2.bindFramebuffer();
    gl::clear(Color::black());
    m_fboGlow2.unbindFramebuffer();

    for (int i=0; i<MOTION_BLUR_LENGTH; i++) {
      m_fboMotion[i] = gl::Fbo(width, height, m_motionFormat);
      m_fboMotion[i].bindFramebuffer();
      gl::clear(Color::black());
      m_fboMotion[i].unbindFramebuffer();
    }
  }

  Utils::genNoiseTexture(m_noiseTex, width, height);
}

void ParticleDemoApp::keyDown(KeyEvent event) {
  char key = event.getChar();
  if (key == 27) {
    // EVENT user exited manually with ESC key
    SendMixPanelEvent("Orientation - User Exited (ESC)", m_listener->GetDeviceID());
    quit();
  }
#if _WIN32
  if (event.isAltDown() && event.getCode() == KeyEvent::KEY_F4) {
    // EVENT user exited manually with ALT+F4 key
    SendMixPanelEvent("Orientation - User Exited (ESC)", m_listener->GetDeviceID());
    quit();
  }
#endif

  if (!m_skipQueued
    && m_stage != m_skipStage
    && m_stage != STAGE_WAITING
    && m_stage != STAGE_BEGIN
    && m_stage != STAGE_OUTRO) {
    m_skipQueued = true;
    m_skipStage = m_stage;
  }
}

void ParticleDemoApp::mouseDown(MouseEvent event) {
  m_currentMousePos = m_previousMousePos = m_initialMousePos = event.getPos();
}

void ParticleDemoApp::mouseDrag(MouseEvent event) {
  static const float CAMERA_SPEED = 0.0035f;

  if (!m_visualizerOnlyMode) {
    return;
  }

  m_previousMousePos = m_currentMousePos;
  m_currentMousePos = event.getPos();
  float dTheta = static_cast<float>(m_currentMousePos.x - m_previousMousePos.x)*CAMERA_SPEED;
  float dPhi = static_cast<float>(m_currentMousePos.y - m_previousMousePos.y)*CAMERA_SPEED;

  m_cameraTheta -= dTheta;
  m_cameraPhi += dPhi;

  if (m_cameraTheta < 0.0f) {
    m_cameraTheta += static_cast<float>(M_PI*2);
  }
  if (m_cameraTheta >= static_cast<float>(M_PI*2)) {
    m_cameraTheta -= static_cast<float>(M_PI*2);
  }
  if (m_cameraPhi < -static_cast<float>(M_PI*0.45)) {
    m_cameraPhi = -static_cast<float>(M_PI*0.45);
  }
  if (m_cameraPhi > static_cast<float>(M_PI*0.45)) {
    m_cameraPhi = static_cast<float>(M_PI*0.45);
  }
}

void ParticleDemoApp::mouseWheel(MouseEvent event) {
  float off = event.getWheelIncrement();
  m_cameraZoom -= 0.1f*event.getWheelIncrement();
  m_cameraZoom = std::min(1.9f, std::max(0.1f, m_cameraZoom));
}

std::string ParticleDemoApp::parseRenderString(const std::string& render_string) {
  // would be cleaner if I used Boost.Spirit
  std::string::const_iterator it = render_string.begin();
  for (; it != render_string.end(); it++) {
    if (isdigit(*it)) {
      break;
    }
  }
  assert(it != render_string.end());
  int model_number = atoi(render_string.substr(it - render_string.begin()).c_str());
  if (render_string.find("Intel HD") != std::string::npos) {
    return "fail";
  }
  if (render_string.find("Intel") != std::string::npos) {
    // Intel GMA or older
    return "fail";
  }

  if (render_string.find("FireGL") != std::string::npos ||
    render_string.find("FirePro") != std::string::npos ||
    render_string.find("Quadro") != std::string::npos) {
      // workstation card, give them a shot at full resolution regardless of the model #
      return "pass";
  }
  if (render_string.find("GeForce") != std::string::npos) {
    if (model_number > 1000) {
      if (render_string.find("M OpenGL") != std::string::npos) {
        // mobile GPU
        if (model_number < 9000) {
          return "fail";
        }
      }
      if (model_number < 6000) {
        // GeForce FX (5000) series or older, DirectX 8
        return "fail";
      }
      // GeForce 6000, 7000 series
      if (model_number < 8000) {
        // GeForce 6000 series first to support DirectX 9
        return "mid-resolution";
      }
    }
    // GeForce 8000 == 9000, 200, 300 (GT200), 400 (Fermi)... 600 or higher series
    return "pass";
  }
  if (render_string.find("Radeon X") != std::string::npos) {
    // Radeon X300, X600, X700, X800, X1000, wide range of all DirectX 9
    return "mid-resolution";
  }
  if (render_string.find("Radeon") != std::string::npos) {
    if (model_number >= 8000 && model_number < 10000) {
      // R200 series has big numbers like 8500LE, 9200SE, 9200, 9250, DirectX 8.1
      return "fail";
    }
    if (model_number < 5000) {
      // R600 (HD2000) series through HD4000 series
      return "mid-resolution";
    }
    // HD5000 and higher
    return "pass";
  }
  // unrecognized video card, err on the middle
  return "mid-resolution (unrecognized)";

  // btw, in practice we should print the choice to stdout so that
  // we can quickly troubleshoot any one user during the beta

  // fail = pop up a dialog box asking the user if they really want to proceed
  // mid-resolution = that 700xsomething we use for the Radeon HD4000
  // pass = full resolution
}

bool ParticleDemoApp::shouldUseFX(const std::string& version_string) {
  if (version_string.empty()) {
    return false;
  }
  if (version_string[0] == '1' || version_string[0] == '2') {
    // OpenGL 2.1 or older
    return false;
  }
  return true;
}

void ParticleDemoApp::update() {
  static bool firstUpdate = true;

  if (m_visualizerOnlyMode && firstUpdate) {
    setAlwaysOnTop(false);
    // EVENT app has begun in Visualizer mode (not Orientation)
    SendMixPanelEvent("Orientation - Visualizer mode", m_listener->GetDeviceID());
    firstUpdate = false;
  }

  if (firstUpdate && m_stage > STAGE_CONNECTING) {
    std::cout << "first update routine" << std::endl;
    if (!m_visualizerOnlyMode) {
      // needed for OSX integrated cards to prevent squashing in full screen.
      setWindowSize(400, 300);

#if _WIN32
      setFullScreen(true);
#endif
      std::cout << glGetString(GL_VENDOR) << std::endl;
      std::cout << glGetString(GL_RENDERER) << std::endl;
      std::cout << glGetString(GL_VERSION) << std::endl;
      std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
      std::cout << glGetString(GL_EXTENSIONS) << std::endl;

      if (m_renderString == "fail") {
        setResolution(800, 600);
        std::cout << "trying to use 800x600" << std::endl;
      } else if (m_renderString == "mid-resolution") {
        setResolution(1280, 720);
        std::cout << "trying to use 1280x720" << std::endl;
      } else {
        std::cout << "trying to use native" << std::endl;
        m_useNativeRes = true;
      }

#if __APPLE__
      // needed for OSX integrated cards to prevent squashing in full screen.
      setWindowSize(400, 300);
      setFullScreen(true);
#endif
      hideCursor();

      // EVENT app has begun in Orientation mode (not Visualizer)
      SendMixPanelEvent("Orientation - Orientation mode", m_listener->GetDeviceID());
    }
    firstUpdate = false;

    return; // don't run draw code until resize is called.
  } else if (m_stage > STAGE_WAITING && !m_visualizerOnlyMode) {
    if ( m_soundEngine ) {
      m_soundEngine->setAllSoundsPaused(false);
    }
  }

  m_haveNewFrame = m_listener->WaitForFrame(m_frame, 1);

  if (!m_frame.hands().isEmpty() && m_particleController->GetSpeedRatio() > 0.2f && m_stage != STAGE_HANDS) {
    m_lastActivityTime = ci::app::getElapsedSeconds();
  }
  if ((!m_frame.pointables().isEmpty() || !m_frame.hands().isEmpty()) && (m_stage == STAGE_HANDS || m_stage == STAGE_DRAWING)) {
    m_lastActivityTime = ci::app::getElapsedSeconds();
  }

  runDemoScript();
  if (m_listener->IsConnected()) {
    const Vector3& offset = ParticleController::LEAP_OFFSET;
    m_handController->update(m_frame.hands());
    //if (m_haveNewFrame) {
      m_particleController->Update(m_frame, m_leap.devices(), m_stage == STAGE_DRAWING);
    //}
    m_particleController->PerformThrottling();
    m_curNumHands = m_frame.hands().count();
    m_totalHandPos = Vec3f::zero();
    for (int i=0; i<m_curNumHands; i++) {
      Leap::Vector handPos = m_frame.hands()[i].palmPosition();
      m_totalHandPos += Vec3f(handPos.x + offset.x(), handPos.y + offset.y(), handPos.z + offset.z());
    }
    if (m_haveNewFrame && m_drawDrawing) {
      updateDrawing(m_frame);
    }
  }

  if ( m_soundEngine ) {
    m_soundEngine->update();
  }
}

void ParticleDemoApp::drawScene() {

  gl::clear(Color(0, 0, 0));

  gl::enableAlphaBlending();
  gl::enableAdditiveBlending();
  gl::disableDepthRead();
  gl::disableDepthWrite();

  // SCREEN SPACE
  gl::setMatricesWindow( getWindowSize() );
  gl::setViewport( getWindowBounds() );

  const float width = static_cast<float>(getWindowWidth());
  const float height = static_cast<float>(getWindowHeight());
  glBegin(GL_QUADS);
  glColor3f(0,0,0);
  glVertex2f(0, 0);
  glVertex2f(width, 0);
  glColor3f(0.04f, 0.07f, 0.12f);
  glVertex2f(width, height);
  glVertex2f(0, height);
  glEnd();

  drawDemoImage();
  drawContinueImage();

  setDemoCamera();

  const Vector3& offset = ParticleController::LEAP_OFFSET;
  if (m_draw3DScene && m_drawDrawing) {
    glPushMatrix();
    glTranslated(offset.x(), offset.y(), offset.z());
    Leap::PointableList pointables = m_frame.pointables();
    for (int i=0; i<pointables.count(); i++) {
      Utils::drawPointable(pointables[i], &m_orthoCamera, m_glowTex);
    }
    drawDrawing();
    glPopMatrix();
  }

  if (m_draw3DScene && !m_leap.devices().isEmpty()) {
    const bool pulsing = (!m_visualizerOnlyMode && m_stage != STAGE_DRAWING);
    glPushMatrix();
    glTranslated(offset.x(), offset.y(), offset.z());
    Utils::drawFrustum(m_leap.devices()[0], m_frustumLinesAlpha, pulsing);
    Utils::drawDevice();
    Utils::drawDeviceSurface(pulsing);
    glPopMatrix();
  }

  if (m_draw3DScene && m_drawHand) {
    if (!m_drawFingerGlowsOnly) {
      m_handController->draw();
    }
    m_handController->drawHands(m_glowTex, m_drawFingerGlowsOnly);
  }

  gl::enableAlphaBlending();
  gl::enableAdditiveBlending();
  gl::disableDepthRead();
  gl::disableDepthWrite();

  if (m_draw3DScene && m_drawFluid) {
    m_particleController->Draw();
  }
}

void ParticleDemoApp::draw() {
  double curTime = ci::app::getElapsedSeconds();
  int width = getWindowWidth();
  int height = getWindowHeight();
  // clear our window
  gl::clear( Color::black() );

  Area viewport = gl::getViewport();

  if (m_useFX) {
    int buf1Idx = m_fboIndex;
    int buf2Idx = (m_fboIndex + 1) % MOTION_BLUR_LENGTH;
    int buf3Idx = (m_fboIndex + 2) % MOTION_BLUR_LENGTH;

    m_fboIndex = (m_fboIndex + 1) % MOTION_BLUR_LENGTH;

    cinder::gl::Fbo& buf1 = m_fboMotion[buf1Idx];
    cinder::gl::Fbo& buf2 = m_fboMotion[buf2Idx];
    cinder::gl::Fbo& buf3 = m_fboMotion[buf3Idx];

    gl::setViewport(buf1.getBounds());
    buf1.bindFramebuffer();
    gl::pushMatrices();
    gl::setMatricesWindow(width, height, false);
    gl::clear( Color::black() );
    drawScene();
    gl::popMatrices();
    buf1.unbindFramebuffer();

    gl::setViewport(buf3.getBounds());
    buf3.bindFramebuffer(); // output frame
    gl::clear( Color::black() );

    if ( m_useFX ) {
      m_shaderMotion.bind();

      buf1.bindTexture(0); // current frame
      m_shaderMotion.uniform("tex0", 0);
      buf2.bindTexture(1); // temporary frame
      m_shaderMotion.uniform("tex1", 1);
      m_shaderMotion.uniform("t", static_cast<float>(curTime));
      m_shaderMotion.uniform("postMult", m_curFadeMult);
      m_noiseTex.bind(2);
      m_shaderMotion.uniform("noiseTex", 2);
      m_shaderMotion.uniform("blurMult", m_blurMult);

      gl::color(Color::white());
      gl::drawSolidRect(Rectf(0, static_cast<float>(height), static_cast<float>(width), 0));

      m_shaderMotion.unbind();
    }

    buf3.unbindFramebuffer();
    buf1.unbindTexture();
    buf2.unbindTexture();

    m_noiseTex.unbind();

    // bind the blur shader
    if ( m_useFX ) {
      m_shaderGlow.bind();
      m_shaderGlow.uniform("tex0", 0); // use texture unit 0
      m_shaderGlow.uniform("glowContrast", m_glowContrast);

      // tell the shader to blur horizontally and the size of 1 pixel
      m_shaderGlow.uniform("sampleOffset", Vec2f(1.0f/m_fboGlow1.getWidth(), 0.0f));

      // copy a horizontally blurred version of our scene into the first blur Fbo
      gl::setViewport( m_fboGlow1.getBounds() );
      m_fboGlow1.bindFramebuffer();
      buf3.bindTexture(0);
      gl::pushMatrices();
      gl::setMatricesWindow(width, height, false);
      gl::clear( Color::black() );
      gl::drawSolidRect( m_fboGlow1.getBounds() );
      gl::popMatrices();
      buf3.unbindTexture();
      m_fboGlow1.unbindFramebuffer();

      // tell the shader to blur vertically and the size of 1 pixel
      m_shaderGlow.uniform("sampleOffset", Vec2f(0.0f, 1.0f/m_fboGlow2.getHeight()));

      // copy a vertically blurred version of our blurred scene into the second blur Fbo
      gl::setViewport( m_fboGlow2.getBounds() );
      m_fboGlow2.bindFramebuffer();
      m_fboGlow1.bindTexture(0);
      gl::pushMatrices();
      gl::setMatricesWindow(width, height, false);
      gl::clear( Color::black() );
      gl::drawSolidRect( m_fboGlow2.getBounds() );
      gl::popMatrices();
      m_fboGlow1.unbindTexture();
      m_fboGlow2.unbindFramebuffer();

      // unbind the shader
      m_shaderGlow.unbind();
    }

    gl::setViewport( viewport );

    // draw our scene with the blurred version added as a blend
    gl::clear( Color::black() );
    gl::color(Color::white() );
    gl::draw(buf3.getTexture(), Rectf(0, static_cast<float>(height), static_cast<float>(width), 0));

    gl::enableAdditiveBlending();
    gl::draw(m_fboGlow2.getTexture(), Rectf(0, static_cast<float>(height), static_cast<float>(width), 0));
    gl::disableAlphaBlending();
  } else {
    drawScene();
  }
}

void ParticleDemoApp::shutdown() {
  if ( m_soundEngine ) {
    m_soundEngine->stopAllSounds();
  }
#ifndef __APPLE__
  if (!m_visualizerOnlyMode && !m_useNativeRes) {
    setResolution(m_originalWidth, m_originalHeight);
  }
#endif
  // EVENT application is exiting in any way except a crash (total time spent is "ci::app::getElapsedSeconds()")
  std::stringstream extraData;
  extraData << "\"Elapsed Time\": \"" << ci::app::getElapsedSeconds();
  SendMixPanelEvent("Orientation - Quit", m_listener->GetDeviceID(), extraData.str());
}

Renderer* ParticleDemoApp::prepareRenderer() {
  return new ci::app::RendererGl(ci::app::RendererGl::AA_NONE);
}

void ParticleDemoApp::getResolution(int& width, int& height) {
#if __APPLE__
  CGRect rect = CGDisplayBounds(CGMainDisplayID());
  width = rect.size.width;
  height = rect.size.height;
#else
  // Get a handle to the desktop window
  const HWND hDesktop = GetDesktopWindow();

  // Get the size of the screen into a rectangle
  RECT rDesktop;
  GetWindowRect( hDesktop, &rDesktop );

  // The top left corner will have coordinates (0, 0)
  // and the bottom right corner will have coordinates
  // (width, height)
  width = rDesktop.right;
  height = rDesktop.bottom;
#endif
}

void ParticleDemoApp::setResolution(int width, int height) {
#if __APPLE__
  CFArrayRef displays = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), NULL);
  CGDisplayModeRef mode;
  int maxResWidth = 0;
  int maxResHeight = 0;
  for (int i = 0; i < CFArrayGetCount(displays); ++i) {
    CGDisplayModeRef displayMode = (CGDisplayModeRef)CFArrayGetValueAtIndex(displays, i);
    int curWidth = (int)CGDisplayModeGetWidth(displayMode);
    int curHeight = (int)CGDisplayModeGetHeight(displayMode);
    if(curWidth <= width && (curWidth > maxResWidth || (curWidth <= maxResWidth && curHeight > maxResHeight))) {
      mode = displayMode;
      maxResWidth = curWidth;
      maxResHeight = curHeight;
    }
  }
  setWindowSize(maxResWidth, maxResHeight);
  CGDisplaySetDisplayMode(CGMainDisplayID(), mode, NULL);
#else
  DEVMODE tempDM;
  DEVMODE mode;
  std::size_t maxWidth = 0;
  std::size_t maxHeight = 0;
  for (int i = 0; EnumDisplaySettings(0, i, &tempDM) != 0; i++) {
	DWORD curWidth = tempDM.dmPelsWidth;
    DWORD curHeight = tempDM.dmPelsHeight;
    if (tempDM.dmBitsPerPel >= 32 && curWidth <= (std::size_t)width && (curWidth > maxWidth || (curWidth == maxWidth && curHeight > maxHeight)) && tempDM.dmBitsPerPel > 16) {
      mode = tempDM;
      maxWidth = curWidth;
      maxHeight = curHeight;
    }
  }
  if (maxWidth > 0) {
    mode.dmFields = mode.dmFields & ~DM_DISPLAYFIXEDOUTPUT;
    mode.dmDisplayFixedOutput = DMDFO_CENTER;
    ChangeDisplaySettings(&mode, CDS_FULLSCREEN);
    setWindowSize(mode.dmPelsWidth, mode.dmPelsHeight);
  }
#endif
}

void ParticleDemoApp::prepareSettings(cinder::app::AppBasic::Settings* settings) {
  const std::vector<std::string>& args = getArgs();
  m_visualizerOnlyMode = false;

  if (args.size() >= 2 && args[1] == "visualizer") {
    m_visualizerOnlyMode = true;
    // forces window to start at top, disable later in update.
    settings->setAlwaysOnTop(true);
  }
  getResolution(m_originalWidth, m_originalHeight);
  settings->setFrameRate(60.0f);
  if (m_originalHeight < 700) {
    settings->setWindowSize(640, 480);
  } else if (m_originalHeight <= 900) {
    settings->setWindowSize(800, 600);
  } else {
    settings->setWindowSize(1024, 768);
  }
  if (m_visualizerOnlyMode) {
    settings->setTitle("Leap Motion Visualizer");
  } else {
    settings->setTitle("Leap Motion Orientation");
  }
}

void ParticleDemoApp::updateDrawing(const Leap::Frame& frame) {
  Leap::PointableList fingers = frame.pointables();
  Leap::HandList hands = frame.hands();

  double curTime = ci::app::getElapsedSeconds();

  static int curStrokeID = 1;

  Vector3 viewDir(m_cameraPos.x - m_cameraCenter.x, m_cameraPos.y - m_cameraCenter.y, m_cameraPos.z - m_cameraCenter.z);

  // update strokes or create new ones
  for (int i=0; i<fingers.count(); i++) {
    int id = fingers[i].id();
    Vector3 tipPos = fingers[i].tipPosition().toVector3<Vector3>();
    if (fingers[i].touchZone() == Leap::Pointable::ZONE_TOUCHING) {
      std::map<int, Stroke>::iterator it = activeStrokes.find(id);
      if (it == activeStrokes.end()) {
        Stroke temp(curTime, curStrokeID++);
        temp.addPoint(tipPos, viewDir, curTime);
        temp.setEndTime(curTime);
        activeStrokes.insert(std::pair<int, Stroke>(id, temp));
      } else {
        it->second.addPoint(tipPos, viewDir, curTime);
        it->second.setEndTime(curTime);
      }
    } else {
      // see if this finger was previously drawing a stroke, if so end the stroke and store it
      std::map<int, Stroke>::iterator it = activeStrokes.find(id);
      if (it != activeStrokes.end()) {
        it->second.setEndTime(curTime);
        savedStrokes.insert(it->second);
        activeStrokes.erase(it);
      }
    }
  }

  // cleanup strokes
  std::set<Stroke>::iterator savedIt = savedStrokes.begin();
  while (savedIt != savedStrokes.end()) {
    if ((curTime - savedIt->endTime()) >= MAX_STROKE_TIME_LENGTH + FADE_TIME) {
      savedStrokes.erase(savedIt++);
    } else {
      ++savedIt;
    }
  }
  std::map<int, Stroke>::iterator it = activeStrokes.begin();
  while (it != activeStrokes.end()) {
    Stroke& stroke = it->second;
    if ((curTime - stroke.endTime()) >= MAX_STROKE_TIME_LENGTH + FADE_TIME) {
      activeStrokes.erase(it++);
    } else if ((curTime - stroke.endTime()) >= 0.1) {
      savedStrokes.insert(it->second);
      activeStrokes.erase(it++);
    } else {
      ++it;
    }
  }
}

void ParticleDemoApp::drawDrawing() const {
  // draw saved and active strokes
  float r, g, b;
  double curTime = ci::app::getElapsedSeconds();
  for (std::set<Stroke>::const_iterator it = savedStrokes.begin(); it != savedStrokes.end(); ++it) {
    const Stroke& stroke = *it;
    Utils::getLabelColors(stroke.id(), r, g, b);
    double curFade = 1.0;
    double timeSinceUpdate = curTime - stroke.endTime();
    if (timeSinceUpdate > MAX_STROKE_TIME_LENGTH) {
      curFade = 1.0 - Utils::smootherStep(std::min(1.0, (timeSinceUpdate-MAX_STROKE_TIME_LENGTH)/FADE_TIME));
    }
    ColorA color(r, g, b, static_cast<float>(curFade));
    stroke.draw(color);
  }
  for (std::map<int, Stroke>::const_iterator it = activeStrokes.begin(); it != activeStrokes.end(); ++it) {
    const Stroke& stroke = it->second;
    Utils::getLabelColors(stroke.id(), r, g, b);
    double curFade = 1.0;
    double timeSinceUpdate = curTime - stroke.endTime();
    if (timeSinceUpdate > MAX_STROKE_TIME_LENGTH) {
      curFade = 1.0 - Utils::smootherStep(std::min(1.0, (timeSinceUpdate-MAX_STROKE_TIME_LENGTH)/FADE_TIME));
    }
    ColorA color(r, g, b, static_cast<float>(curFade));
    stroke.draw(color);
  }
}

#define ANY_KEY_DELAY 6.0f

void ParticleDemoApp::runDemoScript() {
  static double accumTime = ci::app::getElapsedSeconds();
  static double lastTime = ci::app::getElapsedSeconds();

  double curTime = ci::app::getElapsedSeconds();
  double deltaTime = curTime - lastTime;
  lastTime = curTime;

  static int lastStage = -1;
  static double timeInStage = 0;
  float fadeMult, imageFadeMult, continueFadeMult;
  bool doLimit = false;
  if (m_listener->IsConnected()) {
    doLimit = (curTime - m_lastActivityTime) < 0.25
                && m_stage != m_skipStage
                && lastStage != STAGE_WAITING
                && lastStage != STAGE_BEGIN
                && lastStage != STAGE_OUTRO;
    if (!getDemoStage(doLimit, accumTime, m_stage, fadeMult, imageFadeMult, continueFadeMult)) {
      // EVENT orientation completed successfully (total amount of time spent was "curTime")
      std::stringstream extraData;
      extraData << "\"Elapsed Time\": " << ci::app::getElapsedSeconds();
      SendMixPanelEvent("Orientation - Completed Success", m_listener->GetDeviceID(), extraData.str());
      quit();
    }
    accumTime += deltaTime;
  } else {
    accumTime = 0;
    m_stage = STAGE_CONNECTING;
    fadeMult = 1.0f;
    imageFadeMult = 1.0f;
    continueFadeMult = 0.0f;
  }

  if (m_stage != lastStage) {
    if (lastStage > STAGE_WAITING) {
      // EVENT stage changed (amount of time spent was "timeInStage" and current stage is "m_stage")
      std::stringstream extraData;
      extraData << "\"Elapsed Time\": " << timeInStage << ",";
      extraData << "\"New Stage\": " << m_stage;
      SendMixPanelEvent("Orientation - Stage Changed", m_listener->GetDeviceID(), extraData.str());
    }
    timeInStage = 0;
    m_lastActivityTime = curTime;
  } else {
    timeInStage += deltaTime;
  }
  lastStage = m_stage;

  static Utils::FPSCounter fpsCounter;
  fpsCounter.Update(ci::app::getElapsedSeconds());
  float fps = fpsCounter.FPS();

  float targetMult = 0.25f;
  if (m_stage >= STAGE_HANDS) {
    targetMult *= 2.0f;
  }
  static const float TARGET_FRAME_RATE = 60.0f;
  float exponent = TARGET_FRAME_RATE / fps;
  m_blurMult = 1.0f - std::pow(1.0f - targetMult, exponent);

  static Utils::TimedMean<float> fadeSmoother(0.33f);
  if (m_stage == STAGE_CONNECTING) {
    fadeSmoother.Update(0.0f, curTime);
    fadeMult = 1.0f;
    m_glowContrast = 0.0f;
    imageFadeMult = 1.0f;
    continueFadeMult = 0.0f;
  } else if (m_stage == STAGE_WAITING) {
    fadeSmoother.Update(0.0f, curTime);
    fadeMult = 0.0f;
    m_glowContrast = 0.0f;
    imageFadeMult = 0.0f;
    continueFadeMult = 0.0f;
  } else {
    fadeSmoother.Update(fadeMult, curTime);
    fadeMult = fadeSmoother.Mean();
    m_glowContrast = 1.2f;
  }

  const float sinceActivityMult = static_cast<float>(Utils::smootherStep(ci::math<double>::clamp((curTime-1.5 - m_lastActivityTime)/5.0)));
  continueFadeMult = timeInStage < ANY_KEY_DELAY ? 0.0f : std::max(continueFadeMult, std::min(1.0f-imageFadeMult, sinceActivityMult));

  m_draw3DScene = !(m_stage == STAGE_CONNECTING)
               && !(m_stage == STAGE_WAITING)
               && !(m_stage == STAGE_BEGIN)
               && !(m_stage == STAGE_OUTRO);
  m_curFadeMult = fadeMult;
  m_curImageAlpha = static_cast<float>(imageFadeMult);
  m_curContinueAlpha = static_cast<float>(continueFadeMult);

  if (m_visualizerOnlyMode && m_stage > STAGE_CONNECTING) {
    m_curImageAlpha = 0.0f;
    m_curContinueAlpha = 0.0f;
  }

  // decide what to draw based on what stage we're in
  if (m_stage >= STAGE_HANDS) {
    if (m_visualizerOnlyMode) {
      m_particleController->SetMaxSpawnRateRatio(0.1, 0.1);
    } else {
      m_particleController->SetMaxSpawnRateRatio(0, 1.0);
    }
  }
  m_drawHand = (m_stage == STAGE_HANDS);
  m_drawDrawing = (m_stage == STAGE_DRAWING);
  m_drawFingerGlowsOnly = (m_stage == STAGE_DRAWING);

  // set camera depending on stage
  if (m_stage <= STAGE_INTRO) {
    m_cameraMode = CAMERA_ORTHO;
    m_frustumLinesAlpha = 0.0f;
  } else if (m_stage <= STAGE_3D) {
    m_cameraMode = CAMERA_PERSP_ROTATING;
    m_frustumLinesAlpha = 1.0f;
  } else if (m_stage == STAGE_DRAWING) {
    m_cameraMode = CAMERA_ORTHO;
    m_frustumLinesAlpha = 0.0f;
  } else {
    m_cameraMode = CAMERA_PERSP_FIXED;
    m_frustumLinesAlpha = 0.0f;
  }

  // set image depending on stage
  m_curContinueNum = -1;
  if (m_stage == STAGE_CONNECTING) {
    m_curImageNum = IMAGE_PLUG_IN;
  } else if (m_stage == STAGE_BEGIN) {
    m_curImageNum = IMAGE_LOGO;
  } else if (m_stage == STAGE_INTRO) {
    m_curImageNum = IMAGE_WHERE;
    m_curContinueNum = IMAGE_CONTINUE;
  } else if (m_stage == STAGE_3D) {
    m_curImageNum = IMAGE_WHERE_3D;
    m_curContinueNum = IMAGE_CONTINUE;
  } else if (m_stage == STAGE_HANDS) {
    m_curImageNum = IMAGE_WHAT;
    m_curContinueNum = IMAGE_CONTINUE;
  } else if (m_stage == STAGE_DRAWING) {
    m_curImageNum = IMAGE_HOW;
    m_curContinueNum = IMAGE_FINISH;
  } else if (m_stage == STAGE_OUTRO) {
    m_curImageNum = IMAGE_LOGO;
  }

  // modify bass loop
  if ( m_bassLoop ) {
    if (m_stage == STAGE_BEGIN) {
      if (timeInStage < FADE_TIME) {
        m_bassLoop->setVolume(std::min(1.0f, 0.9f*fadeMult));
      }
    } else if (m_stage == STAGE_OUTRO) {
      if (timeInStage > FADE_TIME) {
        m_bassLoop->setVolume(std::min(1.0f, 0.9f*fadeMult));
      }
    }
  }

  if ( m_transitionLoop ) {
    // modify transition loop
    if (m_stage >= STAGE_INTRO) {
      if (m_stage == STAGE_INTRO && timeInStage < FADE_TIME) {
        m_transitionLoop->setVolume(0.0f);
      } else if (m_stage == STAGE_OUTRO && timeInStage > FADE_TIME) {
        m_transitionLoop->setVolume(0.0f);
      } else if (m_stage > STAGE_OUTRO) {
        m_transitionLoop->setVolume(0.0f);
      } else {
        m_transitionLoop->setVolume(std::min(0.9f, (1.0f-fadeMult)*0.5f));
      }
    }
  }

  if ( m_handLoop ) {
    // modify hands loop
    if (m_stage == STAGE_HANDS) {
      static Utils::RollingMean<50> volSmoother;
      volSmoother.Update(static_cast<float>(m_curNumHands));
      float mult = std::min(0.9f, volSmoother.avg*0.4f);
      m_handLoop->setVolume(std::min(1.0f, fadeMult * (mult + 0.1f)));
    } else {
      m_handLoop->setVolume(0);
    }
  }

  // modify drawing loop
  if ( m_drawingLoop && m_glowLoop ) {
    if (m_stage == STAGE_DRAWING) {
      static Utils::RollingMean<30> volSmoother;
      volSmoother.Update(static_cast<float>(activeStrokes.size()));
      float mult = ci::math<float>::clamp(volSmoother.avg*0.6f, 0.15f, 0.9f);
      m_drawingLoop->setVolume(std::min(1.0f, 0.5f * fadeMult));
      m_glowLoop->setVolume(std::min(1.0f, fadeMult * mult));
    } else {
      m_drawingLoop->setVolume(0);
      m_glowLoop->setVolume(0);
    }
  }
  // modify 3D rotation loop
  if ( m_rotationLoop ) {
    if (m_stage == STAGE_3D) {
      float speedRatio = m_particleController->GetSpeedRatio();
      m_rotationLoop->setVolume(std::min(1.0f, fadeMult*(0.5f * speedRatio + 0.2f)));
    } else {
      m_rotationLoop->setVolume(0);
    }
  }

  if ( m_fluidLoop ) {
    // modify fluid loop
    if (m_stage == STAGE_INTRO) {
      float speedRatio = m_particleController->GetSpeedRatio();
      m_fluidLoop->setVolume(std::min(1.0f, fadeMult*(0.5f * speedRatio + 0.2f)));
    } else {
      m_fluidLoop->setVolume(0);
    }
  }

  updateCamera(timeInStage);
}

void ParticleDemoApp::setDemoCamera() {
  if (m_cameraMode == CAMERA_ORTHO) {
    gl::setMatrices(m_orthoCamera);
  } else if (m_cameraMode == CAMERA_PERSP_FIXED) {
    gl::setMatrices(m_camera);
  } else if (m_cameraMode == CAMERA_PERSP_ROTATING) {
    gl::setMatrices(m_camera);
  }
}

void ParticleDemoApp::drawDemoImage() {
  const ci::Vec2f center = getWindowCenter();
  const float width = (const float)getWindowWidth();
  if (m_curImageNum == IMAGE_PLUG_IN || m_curImageNum == IMAGE_LOGO) {
    // draw an actual texture
    float scale = 1.0f;
    gl::Texture* tex = 0;
    if (m_curImageNum == IMAGE_PLUG_IN) {
      scale = 0.75f;
      tex = &m_plugInTex;
    } else {
      scale = 0.7f;
      tex = &m_logoTex;
    }

    // draw the texture
    const ColorA c(1, 1, 1, m_curImageAlpha);
    gl::color(c);
    glEnable(GL_TEXTURE_2D);
    const float logoAspect = tex->getAspectRatio();
    tex->bind();
    const float height = width / logoAspect;
    const Rectf r(center.x - scale*width/2, center.y - scale*height/2, center.x + scale*width/2, center.y + scale*height/2);
    gl::drawSolidRect(r);
    tex->unbind();
    glDisable(GL_TEXTURE_2D);
  } else {
    // draw a text string
    if (m_curImageNum == IMAGE_WHERE) {
      m_textStrings.drawWhereStrings(m_curImageAlpha, width, (float)getWindowHeight());
    } else if (m_curImageNum == IMAGE_WHERE_3D) {
      m_textStrings.drawWhere3DStrings(m_curImageAlpha, width, (float)getWindowHeight());
    } else if (m_curImageNum == IMAGE_WHAT) {
      m_textStrings.drawWhatStrings(m_curImageAlpha, width, (float)getWindowHeight());
    } else if (m_curImageNum == IMAGE_HOW) {
      m_textStrings.drawHowStrings(m_curImageAlpha, width, (float)getWindowHeight());
    } else {
      return;
    }
  }
}

void ParticleDemoApp::drawContinueImage() {
  const ci::Vec2f center = getWindowCenter();
  const float width = (const float)getWindowWidth();
  if (m_curContinueNum == IMAGE_CONTINUE) {
    m_textStrings.drawContinueString(m_curContinueAlpha, width, (float)getWindowHeight());
  } else if (m_curContinueNum == IMAGE_FINISH) {
    m_textStrings.drawFinishString(m_curContinueAlpha, width, (float)getWindowHeight());
  } else {
    return;
  }
}

bool ParticleDemoApp::getDemoStage(bool limitStage, double& curTime, int& stage, float& fadeMult, float& imageFadeMult, float& continueFadeMult) {
  static bool loaded = false;
  static double stageTimes[NUM_STAGES];

  if (!loaded) {
    // set how long each stage of the demo lasts
    if (m_visualizerOnlyMode) {
      stageTimes[STAGE_CONNECTING] = 0.001;
      stageTimes[STAGE_WAITING] = 0;
      stageTimes[STAGE_BEGIN] = 0;
      stageTimes[STAGE_INTRO] = 0;
      stageTimes[STAGE_3D] = 0;
      stageTimes[STAGE_HANDS] = 99999999999;
      stageTimes[STAGE_DRAWING] = 0;
      stageTimes[STAGE_OUTRO] = 0;
    } else {
      stageTimes[STAGE_CONNECTING] = 0.001;
      stageTimes[STAGE_WAITING] = 2;
      stageTimes[STAGE_BEGIN] = 2*FADE_TIME;
      stageTimes[STAGE_INTRO] = 99999999999;
      stageTimes[STAGE_3D] = 99999999999;
      stageTimes[STAGE_HANDS] = 99999999999;
      stageTimes[STAGE_DRAWING] = 99999999999;
      stageTimes[STAGE_OUTRO] = 2*FADE_TIME;
    }
    loaded = true;
  }

  // find the current stage
  double totalTime = 0;
  for (int i=0; i<NUM_STAGES; i++) {
    totalTime += stageTimes[i];
    if (totalTime > curTime) {
      stage = i;
      double curStageTime = curTime - (totalTime - stageTimes[i]);
      imageFadeMult = 1.0f - static_cast<float>(Utils::smootherStep(ci::math<double>::clamp(fabs(curStageTime - 1.5*FADE_TIME)/FADE_TIME)));
      continueFadeMult = 1.0f - static_cast<float>(Utils::smootherStep(ci::math<double>::clamp(fabs((curStageTime-ANY_KEY_DELAY) - 1.5*FADE_TIME)/FADE_TIME)));

      if (m_skipQueued) {
        curTime = totalTime - FADE_TIME;
        m_skipQueued = false;
      }

      if (curStageTime < FADE_TIME) {
        fadeMult = static_cast<float>(Utils::smootherStep(curStageTime/FADE_TIME));
      } else if (stageTimes[i] - curStageTime < FADE_TIME) {
        if (limitStage) {
          fadeMult = 1.0f;
          curTime = totalTime - FADE_TIME;
        } else {
          fadeMult = static_cast<float>(Utils::smootherStep((stageTimes[i] - curStageTime)/FADE_TIME));
        }
      } else {
        fadeMult = 1.0f;
      }
      return true;
    }
  }
  return false; // shouldn't get here
}

void ParticleDemoApp::updateCamera(double timeInStage) {
  static const ci::Vec3f up = Vec3f::yAxis();
  static Utils::TimedMean<Vec3f> avgHandPosSmoother(2.0);
  static Utils::RollingMean<30, Vec3f> expHandPosSmoother;

  double curTime = ci::app::getElapsedSeconds();
  if (m_curNumHands > 0) {
    avgHandPosSmoother.Update(m_totalHandPos/static_cast<float>(m_curNumHands), curTime);
  } else {
    avgHandPosSmoother.Update(Vec3f::zero(), curTime);
  }
  Vec3f handCenter = avgHandPosSmoother.Mean();
  expHandPosSmoother.Update(handCenter);
  handCenter = expHandPosSmoother.avg;

  // modify camera depending on mode
  float aspect = getWindowAspectRatio();
  if (m_cameraMode == CAMERA_ORTHO) {
    if (m_stage == STAGE_DRAWING) {
      Leap::InteractionBox box = m_frame.interactionBox();
      float boxCenterY = box.center().y + ParticleController::LEAP_OFFSET.y();
      float halfWidth = box.width()/2.0f;
      float left = -halfWidth;
      float right = halfWidth;
      float bottom = boxCenterY - halfWidth/aspect;
      float top = boxCenterY + halfWidth/aspect;
      m_orthoCamera.setOrtho(left, right, bottom, top, -3000.0f, 3000.0f);
    } else {
      static const float ORTHO_WIDTH = static_cast<float>(ParticleController::VELFIELD_SCALE*ParticleController::FLUID_DIM/2.0);
      static const float ORTHO_OFFSET = 30;
      float left = -ORTHO_WIDTH;
      float right = ORTHO_WIDTH;
      float bottom = -ORTHO_WIDTH/aspect + ORTHO_OFFSET;
      float top = ORTHO_WIDTH/aspect + ORTHO_OFFSET;
      m_orthoCamera.setOrtho(left, right, bottom, top, -3000.0f, 3000.0f);
    }
    m_cameraPos = ci::Vec3f(0, 0, 1000);
    m_cameraCenter = ci::Vec3f(0, 0, 0);
  } else if (m_cameraMode == CAMERA_PERSP_FIXED) {
    const ci::Vec3f center = Vec3f(0.0f, -50.0f, 0.0f);
    m_camera.setPerspective(50.0f, aspect, 5.0f, 3000.0f);
    float dist = m_cameraZoom * (m_stage == STAGE_HANDS ? 800.0f : 400.0f);
    Vec3f eye;
    eye.x = cosf(m_cameraPhi) * sinf(m_cameraTheta) * dist;
    eye.y = sinf(m_cameraPhi) * dist;
    eye.z = cosf(m_cameraPhi) * cosf(m_cameraTheta) * dist;
    if (m_stage == STAGE_HANDS) {
      m_cameraPos = (eye + handCenter)/2.0f;
      m_cameraCenter = (center + handCenter)/2.0f;
    } else {
      m_cameraPos = eye;
      m_cameraCenter = center;
    }
    m_camera.lookAt(m_cameraPos, m_cameraCenter, up);
  } else if (m_cameraMode == CAMERA_PERSP_ROTATING) {
    m_cameraCenter = Vec3f::zero();
    m_camera.setPerspective(50.0f, aspect, 5.0f, 3000.0f);

    static const double CAM_DIST_TIME = 7.0;
    static const double START_DIST = 700.0;
    static const double END_DIST = 1000.0;

    static const double ORBIT_TIME_X = 4.1;
    static const double ORBIT_TIME_Y = 5.3;
    static const double ORBIT_TIME_Z = 6.5;
    static const double MOVEMENT_RANGE_X = 150;
    static const double MOVEMENT_RANGE_Y = 50;
    static const double MOVEMENT_RANGE_Z = 50;

    static const Vector3 MOVEMENT_DIRECTION = Vector3(0.15f, 0.0f, 1.0f).normalized();
    double progress = ci::math<double>::clamp(timeInStage / CAM_DIST_TIME);
    progress = Utils::smootherStep(1.0 - progress*progress);

    float camDist = static_cast<float>(progress*(END_DIST - START_DIST) + START_DIST);
    m_cameraPos.x = camDist*MOVEMENT_DIRECTION.x() + static_cast<float>(MOVEMENT_RANGE_X*std::sin(timeInStage / ORBIT_TIME_X));
    m_cameraPos.y = camDist*MOVEMENT_DIRECTION.y() + static_cast<float>(MOVEMENT_RANGE_Y*std::sin(timeInStage / ORBIT_TIME_Y));
    m_cameraPos.z = camDist*MOVEMENT_DIRECTION.z() + static_cast<float>(MOVEMENT_RANGE_Z*std::sin(timeInStage / ORBIT_TIME_Z));

    m_camera.lookAt(m_cameraPos, m_cameraCenter, up);
  }
}

bool ParticleDemoApp::isPongo() {
#if _WIN32
  std::string programDataPath = getenv("PROGRAMDATA");
  programDataPath.append("\\Leap Motion\\installtype");

  std::ifstream installTypeFile(programDataPath, std::ios_base::in);
  if( !installTypeFile )
    return false;

  std::string installtype;
  installTypeFile >> installtype;
  if( installtype.find("pongo") == std::string::npos )
    return false;
  else
    return true;
#else
  return false;
#endif
}

bool ParticleDemoApp::isHOPS() {
#if _WIN32
  std::string programDataPath = getenv("PROGRAMDATA");
  programDataPath.append("\\Leap Motion\\installtype");

  std::ifstream installTypeFile(programDataPath, std::ios_base::in);
  if( !installTypeFile )
    return false;

  std::string installtype;
  installTypeFile >> installtype;
  if( installtype.find("hops") == std::string::npos )
    return false;
  else
    return true;
#else
  return false;
#endif
}

#if _WIN32
  //#pragma comment( linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"" )

  void sandboxed_main() {
    CrashReport cr;

    cinder::app::AppBasic::prepareLaunch();
    cinder::app::AppBasic *app = new ParticleDemoApp;
    cinder::app::Renderer *ren = new RendererGl;
    cinder::app::AppBasic::executeLaunch( app, ren, "ParticleDemoApp");
    cinder::app::AppBasic::cleanupLaunch();
  }

  LONG WINAPI HandleCrash(EXCEPTION_POINTERS* pException_) {
     ::MessageBox(0, L"Orientation has crashed. Please make sure you have the latest graphics drivers installed.", L"Error", MB_OK);
      // EVENT crashed
      SendMixPanelEvent("Orientation - Crashed", "");
     return EXCEPTION_EXECUTE_HANDLER;
  }

  int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {
    SetUnhandledExceptionFilter(HandleCrash);
    sandboxed_main();
    return 0;
  }
#else
  //CINDER_APP_BASIC( ParticleDemoApp, RendererGl )

  int main( int argc, char * const argv[] ) {
    CrashReport cr;

    cinder::app::AppBasic::prepareLaunch();
    cinder::app::AppBasic *app = new ParticleDemoApp;
    cinder::app::Renderer *ren = new RendererGl;
    cinder::app::AppBasic::executeLaunch( app, ren, "ParticleDemoApp", argc, argv );
    cinder::app::AppBasic::cleanupLaunch();

    return 0;
  }
#endif

  /*
#else

#pragma comment( linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"" )

int main( int argc, char * const argv[] ) {
  cinder::app::AppBasic::prepareLaunch();
  cinder::app::AppBasic *app = new ParticleDemoApp;
  cinder::app::Renderer *ren = new RendererGl;
  cinder::app::AppBasic::executeLaunch( app, ren, "ParticleDemoApp");
  cinder::app::AppBasic::cleanupLaunch();
  return 0;
}
*/
