#ifndef __ParticleDemoApp_h__
#define __ParticleDemoApp_h__

#include <cinder/app/AppBasic.h>
#include <cinder/gl/gl.h>
#include <cinder/Camera.h>
#include <cinder/gl/Fbo.h>
#include <cinder/gl/GlslProg.h>
#include <cinder/ImageIo.h>
#include <cinder/CinderResources.h>
#include <cinder/Thread.h>

#include "Leap.h"
#include <map>
#include <vector>
#include <set>

#include "MSAFluidSolver3D.h"
#include "ParticleSystem.h"
#include "VectorField.h"
#include "CurlNoiseField.h"
#include "MersenneTwister.h"
#include "TextStrings.h"

#include "Resources.h"
#include "HandController.h"
#include "ParticleController.h"
#include "LeapListener.h"
#include "Globals.h"
#include "Utils.h"
#include "Stroke.h"

#if !defined(MSC_VER) && !defined(_DEBUG)
#define IRRKLANG_STATIC
#endif
#include <irrKlang.h>

using namespace ci;
using namespace ci::app;
using namespace std;

class ParticleDemoApp : public AppBasic {

public:
  ParticleDemoApp();

  // cinder overloads
  virtual void setup();
  virtual void resize(ResizeEvent event);
  virtual void keyDown(KeyEvent event);
  virtual void mouseDown( MouseEvent event );
  virtual void mouseDrag( MouseEvent event );
  virtual void mouseWheel( MouseEvent event );
  virtual void update();
  virtual void draw();
  virtual void shutdown();
  virtual Renderer* prepareRenderer();
  virtual void prepareSettings(cinder::app::AppBasic::Settings* settings);

private:

  std::vector<DataSourceRef> m_audioSourceRefs;
  std::set<Stroke> savedStrokes;
  std::map<int, Stroke> activeStrokes;

  irrklang::ISound* createSoundResource(DataSourceRef ref, const char* name);
  
  void drawScene();
  void updateDrawing(const Leap::Frame& frame);
  void drawDrawing() const;
  void runDemoScript();
  void setDemoCamera();
  void drawDemoImage();
  void drawContinueImage();
  bool getDemoStage(bool limitStage, double& curTime, int& stage, float& fadeMult, float& imageFadeMult, float& continueFadeMult);
  void setResolution(int width, int height);
  void getResolution(int& width, int& height);
  void updateCamera(double timeInStage);
  bool isPongo();
  bool isHOPS();
  std::string parseRenderString(const std::string& render_string);
  bool shouldUseFX(const std::string& version_string);

  static const int MOTION_BLUR_LENGTH = 3;
  static const double FADE_TIME;
  static const double MAX_STROKE_TIME_LENGTH;

  // originally from ParticleController.h
  static const int FLUID_DIM;
  static const double VELFIELD_SCALE;
  static const Vector3 LEAP_OFFSET;

  enum { CAMERA_ORTHO, CAMERA_PERSP_FIXED, CAMERA_PERSP_ROTATING };
  enum { IMAGE_PLUG_IN, IMAGE_LOGO, IMAGE_WHERE, IMAGE_WHERE_3D, IMAGE_WHAT, IMAGE_HOW, IMAGE_CONTINUE, IMAGE_FINISH, IMAGE_NONE };
  enum { STAGE_CONNECTING, STAGE_WAITING, STAGE_BEGIN, STAGE_INTRO, STAGE_3D, STAGE_HANDS, STAGE_DRAWING, STAGE_OUTRO, NUM_STAGES };

  // stuff for glow
  gl::Fbo			            m_fboGlow1;
  gl::Fbo			            m_fboGlow2;
  gl::GlslProg            m_shaderGlow;
  gl::Fbo::Format         m_glowFormat;
  float                   m_glowContrast;

  // stuff for motion blur
  int                     m_fboIndex;
  gl::Fbo                 m_fboMotion[MOTION_BLUR_LENGTH];
  gl::GlslProg            m_shaderMotion;
  gl::Fbo::Format         m_motionFormat;

  // controls for individual parts
  bool                    m_draw3DScene;
  bool                    m_drawFluid;
  bool                    m_drawHand;
  bool                    m_drawDrawing;
  bool                    m_drawFingerGlowsOnly;

  // visualizer mode features
  bool                    m_visualizerOnlyMode;
  Vec2i                   m_initialMousePos;
  Vec2i                   m_currentMousePos;
  Vec2i                   m_previousMousePos;
  float                   m_cameraTheta;
  float                   m_cameraPhi;
  float                   m_cameraZoom;

  // loaded images and instructions
  gl::Texture             m_glowTex;
  gl::Texture             m_noiseTex;
  gl::Texture             m_logoTex;
  gl::Texture             m_plugInTex;
  TextStrings             m_textStrings;

  // stuff for scene drawing
  gl::GlslProg            m_fingerShader;
  HandController*         m_handController;
  ParticleController*     m_particleController;
  CameraPersp             m_camera;
  CameraOrtho             m_orthoCamera;
  int                     m_cameraMode;
  int                     m_stage;
  int                     m_curImageNum;
  float                   m_curImageAlpha;
  float                   m_curContinueAlpha;
  int                     m_curContinueNum;
  float                   m_curFadeMult;
  double                  m_lastActivityTime;
  float                   m_frustumLinesAlpha;
  int                     m_curNumHands;
  Vec3f                   m_totalHandPos;
  float                   m_blurMult;
  Vec3f                   m_cameraPos;
  Vec3f                   m_cameraCenter;
  int                     m_originalWidth;
  int                     m_originalHeight;
  bool                    m_useNativeRes;
  bool                    m_useFX;
  std::string             m_renderString;
  bool                    m_skipQueued;
  int                     m_skipStage;

  // Leap
  LeapListener*           m_listener;
  Leap::Controller        m_leap;
  Leap::Frame             m_frame;
  bool                    m_haveNewFrame;

  // audio stuff
  irrklang::ISoundEngine* m_soundEngine;
  irrklang::ISound*       m_bassLoop;
  irrklang::ISound*       m_rotationLoop;
  irrklang::ISound*       m_fluidLoop;
  irrklang::ISound*       m_transitionLoop;
  irrklang::ISound*       m_handLoop;
  irrklang::ISound*       m_drawingLoop;
  irrklang::ISound*       m_glowLoop;

};

#endif
