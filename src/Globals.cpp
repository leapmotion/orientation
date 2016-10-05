#include "Globals.h"

#define M_4_PI 12.566370614359172

using namespace ci;

int MAX_PARTICLES	= 600;
int MAX_LIGHTS		= 10;
float DEATH_TIME	= 20.0f;
float SCALE			= 1.0f;
float Y_OFFSET		= -200.0f;

Color cRed			= Color( 1.0f, 0.1f, 0.0f );
Color cBlue			= Color( 0.5f, 0.9f, 1.0f );
Color cOrange		= Color( 1.0f, 0.5f, 0.0f );
Color cYellow		= Color( 1.0f, 1.0f, 0.3f );
Color cGreen    = Color( 0.5f, 1.0f, 0.2f );
Color cWhite    = Color( 1.f, 1.f, 1.f);

Vec3f toVecModified( const Leap::Vector &v )
{
	return ci::Vec3f( v.x, v.y, v.z ) * SCALE + Vec3f( 0.0f, Y_OFFSET, 0.0f );
}

Vec3f toVec( const Leap::Vector &v )
{
	return ci::Vec3f( v.x, v.y, v.z );
}

float getRadiusFromMass( float m )
{
	float r = powf( (3.0f * m )/(float)M_4_PI, 0.3333333f );
	return r;
}

float getMassFromRadius( float r )
{
	float m = ( ( r * r * r ) * (float)M_4_PI ) * 0.33333f;
	return m;
}
