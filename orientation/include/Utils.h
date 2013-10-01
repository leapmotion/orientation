#ifndef __Utils_h__
#define __Utils_h__

#include <cinder/app/AppBasic.h>
#include <cinder/gl/gl.h>
#include <cinder/gl/Fbo.h>
#include <cinder/gl/GlslProg.h>
#include <cinder/ImageIo.h>
#include <cinder/Camera.h>
#include "Leap.h"
#include "DataTypes.h"
#include <deque>

#ifdef __APPLE__
#include <OpenGL/GLU.h>
#else
#include <gl/GLU.h>
#endif

#include "DataTypes.h"
#include "MersenneTwister.h"

namespace Utils {

static const double TIME_BETWEEN_PEAKS = 3.44;

static void drawFrustum(const Leap::Device& device, float edgesAlpha, bool withPulsing) {
  double curTime = ci::app::getElapsedSeconds();
  static const double PI = 3.14159;
  float mult = 1.0f;
  if (withPulsing) {
    mult = static_cast<float>(0.2*std::sin(curTime*PI/(0.5*TIME_BETWEEN_PEAKS)) + 0.8);
  }
  ci::ColorA centerColor(0.25f, 0.45f, 0.65f, 0.4f);
  centerColor *= mult;
  ci::ColorA outsideColor(0.0f, 0.0f, 0.0f, 0.0f);
  glPushMatrix();

  double range = 1.0*device.range();
  double faceRange = 1.5*device.range();
  double shortRange = 0.01*device.range();
  double x = std::tan(device.horizontalViewAngle() * 0.5);
  double y = std::tan(device.verticalViewAngle() * 0.5);

  Vector3 viewPoint(Vector3::Zero());
  Vector3 upperLeft(x, 1.0, y);
  Vector3 lowerLeft(x, 1.0, -y);
  Vector3 upperRight(-x, 1.0, y);
  Vector3 lowerRight(-x, 1.0, -y);
  Vector3 left = (upperLeft + lowerLeft)/2.0;
  Vector3 right = (upperRight + lowerRight)/2.0;
  Vector3 top = (upperLeft + upperRight)/2.0;
  Vector3 bottom = (lowerLeft + lowerRight)/2.0;

  static const int NUM_FACE_POINTS = 15;
  static const double FACE_INCREMENT = 1.0 / static_cast<double>(NUM_FACE_POINTS);
  static const int NUM_LINE_POINTS = 50;
  static const double LINE_INCREMENT = 1.0 / static_cast<double>(NUM_LINE_POINTS);
  double blend;

  // draw top face
  blend = 0;
  glBegin(GL_TRIANGLE_STRIP);
  for (int i=0; i<=NUM_FACE_POINTS; i++) {
    Vector3 pos = blend*upperLeft + (1.0-blend)*upperRight;
    ci::gl::color(centerColor);
    glVertex3d(shortRange*pos[0], shortRange*pos[1], shortRange*pos[2]);
    pos.normalize();
    ci::gl::color(outsideColor);
    glVertex3d(faceRange*pos[0], faceRange*pos[1], faceRange*pos[2]);
    blend += FACE_INCREMENT;
  }
  glEnd();

  // draw left face
  blend = 0;
  glBegin(GL_TRIANGLE_STRIP);
  for (int i=0; i<=NUM_FACE_POINTS; i++) {
    Vector3 pos = blend*lowerLeft + (1.0-blend)*upperLeft;
    ci::gl::color(centerColor);
    glVertex3d(shortRange*pos[0], shortRange*pos[1], shortRange*pos[2]);
    pos.normalize();
    ci::gl::color(outsideColor);
    glVertex3d(faceRange*pos[0], faceRange*pos[1], faceRange*pos[2]);
    blend += FACE_INCREMENT;
  }
  glEnd();

  // draw bottom face
  blend = 0;
  glBegin(GL_TRIANGLE_STRIP);
  for (int i=0; i<=NUM_FACE_POINTS; i++) {
    Vector3 pos = blend*lowerRight + (1.0-blend)*lowerLeft;
    ci::gl::color(centerColor);
    glVertex3d(shortRange*pos[0], shortRange*pos[1], shortRange*pos[2]);
    pos.normalize();
    ci::gl::color(outsideColor);
    glVertex3d(faceRange*pos[0], faceRange*pos[1], faceRange*pos[2]);
    blend += FACE_INCREMENT;
  }
  glEnd();

  // draw right face
  blend = 0;
  glBegin(GL_TRIANGLE_STRIP);
  for (int i=0; i<=NUM_FACE_POINTS; i++) {
    Vector3 pos = blend*upperRight + (1.0-blend)*lowerRight;
    ci::gl::color(centerColor);
    glVertex3d(shortRange*pos[0], shortRange*pos[1], shortRange*pos[2]);
    pos.normalize();
    ci::gl::color(outsideColor);
    glVertex3d(faceRange*pos[0], faceRange*pos[1], faceRange*pos[2]);
    blend += FACE_INCREMENT;
  }
  glEnd();

  if (edgesAlpha > 0.01f) {
    glLineWidth(5);
    ci::gl::color(centerColor * edgesAlpha);

    // draw top edge
    blend = 0;
    glBegin(GL_LINE_STRIP);
    glVertex3d(viewPoint[0], viewPoint[1], viewPoint[2]);
    for (int i=0; i<=NUM_LINE_POINTS; i++) {
      Vector3 pos = blend*upperLeft + (1.0-blend)*upperRight;
      pos = range * pos.normalized();
      glVertex3d(pos[0], pos[1], pos[2]);
      blend += LINE_INCREMENT;
    }
    glEnd();

    // draw left edge
    blend = 0;
    glBegin(GL_LINE_STRIP);
    glVertex3d(viewPoint[0], viewPoint[1], viewPoint[2]);
    for (int i=0; i<=NUM_LINE_POINTS; i++) {
      Vector3 pos = blend*lowerLeft + (1.0-blend)*upperLeft;
      pos = range * pos.normalized();
      glVertex3d(pos[0], pos[1], pos[2]);
      blend += LINE_INCREMENT;
    }
    glEnd();

    // draw bottom edge
    blend = 0;
    glBegin(GL_LINE_STRIP);
    glVertex3d(viewPoint[0], viewPoint[1], viewPoint[2]);
    for (int i=0; i<=NUM_LINE_POINTS; i++) {
      Vector3 pos = blend*lowerRight + (1.0-blend)*lowerLeft;
      pos = range * pos.normalized();
      glVertex3d(pos[0], pos[1], pos[2]);
      blend += LINE_INCREMENT;
    }
    glEnd();

    // draw right edge
    blend = 0;
    glBegin(GL_LINE_STRIP);
    glVertex3d(viewPoint[0], viewPoint[1], viewPoint[2]);
    for (int i=0; i<=NUM_LINE_POINTS; i++) {
      Vector3 pos = blend*upperRight + (1.0-blend)*lowerRight;
      pos = range * pos.normalized();
      glVertex3d(pos[0], pos[1], pos[2]);
      blend += LINE_INCREMENT;
    }
    glEnd();

    // draw range horizontal edge
    blend = 0;
    glBegin(GL_LINE_STRIP);
    for (int i=0; i<=NUM_LINE_POINTS; i++) {
      Vector3 pos = blend*left + (1.0-blend)*right;
      pos = range * pos.normalized();
      glVertex3d(pos[0], pos[1], pos[2]);
      blend += LINE_INCREMENT;
    }
    glEnd();

    // draw range vertical edge
    blend = 0;
    glBegin(GL_LINE_STRIP);
    for (int i=0; i<=NUM_LINE_POINTS; i++) {
      Vector3 pos = blend*top + (1.0-blend)*bottom;
      pos = range * pos.normalized();
      glVertex3d(pos[0], pos[1], pos[2]);
      blend += LINE_INCREMENT;
    }
    glEnd();
  }

  glPopMatrix();
}

static void genNoiseTexture(ci::gl::Texture& tex, int width, int height) {
  ci::gl::Texture::Format format;
  format.setInternalFormat(GL_RGB);
  format.setMinFilter(GL_NEAREST);
  format.setMagFilter(GL_NEAREST);
  int numValues = width*height*3;
  unsigned char* data = new unsigned char[numValues];
  MersenneTwister twister;
  for (int i=0; i<numValues; i++) {
    data[i] = static_cast<char>(255*twister.GetDouble());
  }
  tex = ci::gl::Texture(data, GL_RGB, width, height, format);
  delete[] data;
}

static void drawSolidBox(const Vector3& center, const Vector3& size) {
  glPushMatrix();
  glTranslated(center.x(), center.y(), center.z());
  glScaled(size.x(), size.y(), size.z());
  glTranslated(-0.5f, -0.5f, 0.5f);
  glBegin(GL_QUADS);
  // front
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);
  // back
  glVertex3f(0.0f, 0.0f, -1.0f);
  glVertex3f(1.0f, 0.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glVertex3f(0.0f, 1.0f, -1.0f);
  // right
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  // left
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, -1.0f);
  glVertex3f(0.0f, 1.0f, -1.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);
  // top
  glVertex3f(0.0f, 1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glVertex3f(0.0f, 1.0f, -1.0f);
  // bottom
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, -1.0f);
  glVertex3f(0.0f, 0.0f, -1.0f);
  glEnd();
  glPopMatrix();
}

static GLUquadric* quadric() { 
  static GLUquadric* gluQuadric = gluNewQuadric();
  return gluQuadric;
}

static void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b) {
  float hh, p, q, t, ff;
  long i;
  if (s <= 0.0f) {
    r = 0.0f;
    g = 0.0f;
    b = 0.0f;
    return;
  }
  hh = h;
  if (hh >= 360.0f) {
    hh = 0.0f;
  }
  hh /= 60.0f;
  i = (long)hh;
  ff = hh - i;
  p = v * (1.0f - s);
  q = v * (1.0f - (s * ff));
  t = v * (1.0f - (s * (1.0f - ff)));
  switch (i) {
  case 0: r = v; g = t; b = p; break;
  case 1: r = q; g = v; b = p; break;
  case 2: r = p; g = v; b = t; break;
  case 3: r = p; g = q; b = v; break;
  case 4: r = t; g = p; b = v; break;
  case 5:
  default: r = v; g = p; b = q; break;
  }
}

static void getLabelColors(int idx, float& r, float& g, float& b) {
  static bool loaded = false;
  static const int MAX_LABEL_COLORS = 200;
  static const float DEFAULT_COLOR_SATURATION = 0.6f;
  static const float DEFAULT_COLOR_VALUE = 0.9f;
  static std::vector<float> labelColorsR, labelColorsG, labelColorsB;
  if (!loaded) {
    MersenneTwister twister;
    const double inc = 0.2;
    double curAdd = 0;
    double curVal;
    for (int i=0; i<MAX_LABEL_COLORS; i++) {
      curVal = inc*twister.GetDouble() + curAdd;
      if (curVal > 1.0) {
        curVal -= 1.0;
      }
      float hue = static_cast<float>(360 * curVal);
      HSVtoRGB(hue, DEFAULT_COLOR_SATURATION, DEFAULT_COLOR_VALUE, r, g, b);
      labelColorsR.push_back(r);
      labelColorsG.push_back(g);
      labelColorsB.push_back(b);
      curAdd += inc;
      if (curAdd > 1.0) {
        curAdd -= 1.0;
      }
    }
    loaded = true;
  }
  r = labelColorsR[idx % MAX_LABEL_COLORS];
  g = labelColorsG[idx % MAX_LABEL_COLORS];
  b = labelColorsB[idx % MAX_LABEL_COLORS];
}

static float ratioToAlpha(float ratio, float falloffWidth) {
  ratio = std::min(std::max(ratio, 0.0f), 1.0f);
  float invFalloff = 1.0f - falloffWidth;
  if (ratio > invFalloff) {
    return 1.0f - (ratio - invFalloff)/falloffWidth;
  } else {
    return 1.0f;
  }
}

static void drawDevice() {
  static const float WIDTH = 30.0f;
  static const float HEIGHT = 12.0f;
  static const float LENGTH = 80.0f;
  static const float RADIUS = 5.0f;
  static const int SIDES = 20;

  glColor3f(1.0f, 1.0f, 1.0f);
  glTranslated(0, 9.5, 0);
  drawSolidBox(Vector3::Zero(), Vector3(LENGTH-2*RADIUS, HEIGHT, WIDTH));
  drawSolidBox(Vector3::Zero(), Vector3(LENGTH, HEIGHT, WIDTH-2*RADIUS));
  glRotated(90, 1, 0, 0);

  glPushMatrix();
  glTranslated(LENGTH/2-RADIUS, 10, -HEIGHT/2);
  gluCylinder(quadric(), RADIUS, RADIUS, HEIGHT, SIDES, 1);
  gluDisk(quadric(), 0, RADIUS, SIDES, 1);
  glPopMatrix();

  glPushMatrix();
  glTranslated(-(LENGTH/2-RADIUS), 10, -HEIGHT/2);
  gluCylinder(quadric(), RADIUS, RADIUS, HEIGHT, SIDES, 1);
  gluDisk(quadric(), 0, RADIUS, SIDES, 1);
  glPopMatrix();

  glPushMatrix();
  glTranslated(LENGTH/2-RADIUS, -10, -HEIGHT/2);
  gluCylinder(quadric(), RADIUS, RADIUS, HEIGHT, SIDES, 1);
  gluDisk(quadric(), 0, RADIUS, SIDES, 1);
  glPopMatrix();

  glPushMatrix();
  glTranslated(-(LENGTH/2-RADIUS), -10, -HEIGHT/2);
  gluCylinder(quadric(), RADIUS, RADIUS, HEIGHT, SIDES, 1);
  gluDisk(quadric(), 0, RADIUS, SIDES, 1);
  glPopMatrix();
}

static inline double smootherStep(double x) {
  // x is blending parameter between 0 and 1
  return x*x*x*(x*(x*6 - 15) + 10);
}

static void getRotation(const Leap::Vector& oldDir, const Leap::Vector& newDir, Leap::Vector& axis, float& angle) {
  axis = oldDir.cross(newDir);
  angle = std::acos(oldDir.dot(newDir));
}

template <typename T>
static inline T interpolateCatmullRom(const T& y0, const T& y1, const T& y2, const T& y3, float mu) {
  T a0, a1, a2;
  a0 = -0.5*y0 + 1.5*y1 - 1.5*y2 + 0.5*y3;
  a1 = y0 - 2.5*y1 + 2.0*y2 - 0.5*y3;
  a2 = -0.5*y0 + 0.5*y2;
  return ((a0*mu + a1)*mu + a2)*mu + y1;
}

static void drawPointable(const Leap::Pointable& pointable, ci::Camera* cam, ci::gl::Texture& glowTex) {
  static const float LENGTH = 50.0f;
  static const float RADIUS = 4.0f;
  static const int SIDES = 20;
  static const float APPEAR_TIME = 0.75f;
  Leap::Vector tip = pointable.stabilizedTipPosition();
  Leap::Vector dir = pointable.direction();
  Leap::Vector vel = pointable.tipVelocity();
  float touchDist = pointable.touchDistance();

  float alpha = smootherStep(std::min(1.0f, pointable.timeVisible()/APPEAR_TIME));

  ci::Vec3f right, up;
  cam->getBillboardVectors(&right, &up);

  static const float TOUCH_DIST_RAMP = 0.5f;
  ci::ColorA color(0.5f, 0.7f, 0.9f, alpha*(1.0f - std::min(1.0f, touchDist/TOUCH_DIST_RAMP)));

  ci::Vec3f tipVec(tip.x, tip.y, tip.z);
  ci::Vec3f tipVel(vel.x, vel.y, vel.z);
  ci::Vec3f viewDir = cam->getViewDirection();
  tipVel -= tipVel.dot(viewDir)*viewDir;
  float tipVelNorm = tipVel.length();
  tipVel /= tipVelNorm;

  Leap::Vector axis;
  float angle;
  getRotation(Leap::Vector(right.x, right.y, right.z), Leap::Vector(tipVel.x, tipVel.y, tipVel.z), axis, angle);

  double radius = 8*ci::math<double>::clamp(touchDist, 0.0, 0.7) + 2.5;
  double borderRadius = std::min(radius, 0.5 + std::max(0.0, 5.0 - radius));

  ci::Matrix44f rot = ci::Matrix44f::createRotation(ci::Vec3f(axis.x, axis.y, axis.z), angle);
  if (touchDist <= 0) {
    color.a = 1.0f;
  }

  ci::gl::color(color);
  glPushMatrix();
  glTranslated(tip.x, tip.y, tip.z);
  ci::gl::multModelView(rot);
  float xScale = (1.0f + tipVelNorm/1000.0f);
  glScalef(xScale, std::sqrt(1.0f/xScale), 1.0f);
  gluDisk(quadric(), radius-borderRadius, radius, 40, 1);
  glPopMatrix();

  if (touchDist < 0) {
    alpha = std::min(1.0f, static_cast<float>(fabs(touchDist)/TOUCH_DIST_RAMP));
    color.a = alpha;
    ci::gl::color(color);
    glowTex.enableAndBind();
	  ci::gl::drawBillboard(tipVec, ci::Vec2f( 90.0f, 90.0f ), 0.0f, right, up );
    glowTex.disable();
    glowTex.unbind();
  }
	ci::gl::disable( GL_TEXTURE_2D );
}

template <int n, class T = float>
struct RollingMean {
  RollingMean() : first(true) { }
  inline void Update(const T& value) {
    if (first) {
      avg = value;
      first = false;
    } else {
      avg = (2*value + (n-1)*avg) / (n+1);
    }
  }
  bool first;
  T avg;
};

struct FPSCounter {
  FPSCounter() : lastTime(ci::app::getElapsedSeconds()) {
    deltaTimes.avg = 1.0f / 60.0f;
  }
  inline float FPS() { return 1.0f / deltaTimes.avg; }
  inline void Update(double time) {
    deltaTimes.Update(static_cast<float>(time - lastTime));
    lastTime = time;
  }
  RollingMean<10> deltaTimes;
  double lastTime;
};

template <class T>
struct TimedMean {
  TimedMean(float window = 1.0) : timeWindow(window), mostRecentTime(0) { }
  void Update(const T& val, double time) {
    if (data.empty()) { sum = val; } else { sum += val; }
    data.push_back(Entry(val, time));
    if (time >= mostRecentTime) {
      mostRecentTime = time;
      while (!data.empty() && mostRecentTime - data.front().time > timeWindow) {
        sum -= data.front().val;
        data.pop_front();
      }
    }
  }
  void Reset() {
    data.clear();
    mostRecentTime = 0;
  }
  inline void SetTimeWindow(double window) { timeWindow = window; }
  inline size_t Size() const { return data.size(); }
  inline T Mean() const { return sum / static_cast<float>(data.size()); }
private:
  struct Entry {
    Entry(const T& _val, double _time) : val(_val), time(_time) { }
    T val;
    double time;
  };
  std::deque<Entry, Eigen::aligned_allocator<Entry> > data;
  T sum;
  double timeWindow;
  double mostRecentTime;
};

}

#endif
