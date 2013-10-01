#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/gl/Texture.h"
#include "cinder/Matrix.h"
#include "cinder/Sphere.h"

#include "Finger.h"
#include "Leap.h"

#include <map>
#include <list>

class Hand
{
  public:
	Hand();
	void	update( const Leap::Hand &hand );
	void	draw() const;
	void	drawPalm() const;
	void	drawFingerBones( ci::gl::GlslProg *shader ) const;
	void	drawFingerSplines() const;
	void	drawFingerKnuckles() const;
	void	drawFingerTipGlows( ci::Camera *cam, ci::gl::Texture &tex ) const;
	void	startDying();
	bool	isDead() const;
	
	ci::Vec3f			mPos;
	ci::Vec3f			mDir;
	ci::Vec3f			mRightPos, mLeftPos;
	ci::Vec3f			mBasePos;
	ci::Vec3f			mPalmPos;
	ci::Vec3f			mNormal;
	ci::Vec3f			mAxis;

	float				mPower;
	float				mAlpha;
	bool				mIsDying;
	
	std::map<uint32_t,Finger>		mFingers;
	std::map<uint32_t,Leap::Finger>	mLeapFingers;
};