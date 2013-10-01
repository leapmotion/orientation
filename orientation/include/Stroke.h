#ifndef __Stroke_h__
#define __Stroke_h__

#include <deque>
#include "DataTypes.h"
#include <cinder/gl/gl.h>
#include <cinder/gl/Texture.h>

class Stroke {

public:

  Stroke(double startTime = 0.0, int id = 0);

  void draw(const ci::ColorA& color) const;
  void addPoint(const Vector3& point, const Vector3& viewDir, double curTime);
  void cleanup(double curTime, double maxTimeLength);

  inline int id() const { return m_id; }
  inline void setEndTime(double endTime) { m_endTime = endTime; }
  inline double endTime() const { return m_endTime; }
  inline bool isEmpty() const { return m_times.empty(); }
  bool operator<(const Stroke& other) const { return m_id < other.m_id; }

private:

  double widthForTime(double time) const;

  typedef std::deque<Vector3, Eigen::aligned_allocator<Vector3> > PointDeque;

  static const int NUM_INTERP_POINTS = 5;

  double              m_startTime;
  double              m_endTime;
  std::deque<double>  m_times;
  PointDeque          m_interp1;
  PointDeque          m_interp2;
  PointDeque          m_orig;
  int                 m_id;
  Vector3             m_last;

};

#endif
