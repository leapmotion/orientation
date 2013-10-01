#include "Stroke.h"
#include "Utils.h"
#include <cinder/Vector.h>

Stroke::Stroke(double startTime, int id)
  : m_startTime(startTime)
  , m_id(id)
{ }

void Stroke::draw(const ci::ColorA& color) const {
  if (m_interp1.size() < 2) {
    return;
  }
  ci::gl::color(color);

  Vector3 diskCenter = (m_interp1.front() + m_interp2.front())/2.0;
  double diskRadius = (m_interp1.front() - diskCenter).norm();
  glPushMatrix();
  glTranslated(diskCenter.x(), diskCenter.y(), diskCenter.z());
  gluDisk(Utils::quadric(), 0.0, diskRadius, 30, 1);
  glPopMatrix();

  glPolygonMode(GL_FRONT, GL_FILL);
	ci::gl::enableAlphaBlending();
  glBegin(GL_QUAD_STRIP);
  for (size_t i=0; i<m_interp1.size(); i++) {
    glTexCoord2f(0, 0);
    glVertex3fv(m_interp1[i].data());
    glTexCoord2f(0, 1);
    glVertex3fv(m_interp2[i].data());
  }
  glEnd();

  diskCenter = (m_interp1.back() + m_interp2.back())/2.0;
  diskRadius = (m_interp1.back() - diskCenter).norm();
  glPushMatrix();
  glTranslated(diskCenter.x(), diskCenter.y(), diskCenter.z());
  gluDisk(Utils::quadric(), 0.0, diskRadius, 30, 1);
  glPopMatrix();

	ci::gl::enableAdditiveBlending();
}

void Stroke::addPoint(const Vector3& point, const Vector3& viewDir, double curTime) {
  static const double INTERP_INC = 1.0 / static_cast<double>(NUM_INTERP_POINTS);
  m_orig.push_back(point);
  m_times.push_back(curTime);
  if (m_orig.size() > 3) {
    double mu = 0;
    int size = static_cast<int>(m_orig.size());
    for (int i=0; i<NUM_INTERP_POINTS; i++) {
      Vector3 result = Utils::interpolateCatmullRom<Vector3>(m_orig[size-4],
                                                             m_orig[size-3],
                                                             m_orig[size-2],
                                                             m_orig[size-1],
                                                             mu);
      double width = Utils::interpolateCatmullRom<double>(widthForTime(m_times[size-4]),
                                                          widthForTime(m_times[size-3]),
                                                          widthForTime(m_times[size-2]),
                                                          widthForTime(m_times[size-1]),
                                                          mu);
      mu += INTERP_INC;
      Vector3 perp = (result - m_last).cross(viewDir).normalized();
      m_interp1.push_back(result - (width/2.0)*perp);
      m_interp2.push_back(result + (width/2.0)*perp);
      m_last = result;
    }
  } else {
    m_last = point;
  }
}

void Stroke::cleanup(double curTime, double maxTimeLength) {
  while (!m_times.empty() && curTime - m_times.front() > maxTimeLength) {
    m_startTime = m_times.front();
    m_times.pop_front();
    m_orig.pop_front();
    for (int i=0; i<NUM_INTERP_POINTS; i++) {
      m_interp1.pop_front();
      m_interp2.pop_front();
    }
  }
}

double Stroke::widthForTime(double time) const {
  static const double START_RAMP_TIME = 0.4;
  static const double MAX_STROKE_WIDTH = 4.0;
  return MAX_STROKE_WIDTH*ci::math<double>::clamp((time-m_startTime)/START_RAMP_TIME, 0.075);
}
