#pragma once

#include <cinder/Color.h>
#include <cinder/Vector.h>
#include "Leap.h"

extern int MAX_PARTICLES;
extern int MAX_LIGHTS;
extern float DEATH_TIME;
extern float SCALE;
extern float Y_OFFSET;
extern ci::Color cRed;
extern ci::Color cBlue;
extern ci::Color cOrange;
extern ci::Color cYellow;

extern ci::Vec3f toVecModified( const Leap::Vector &v );
extern ci::Vec3f toVec( const Leap::Vector &v );
extern float getRadiusFromMass( float mass );
extern float getMassFromRadius( float radius );



