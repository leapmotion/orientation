#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/GlslProg.h"

#include "Leap.h"

class Hand;

class Finger {
  public:
	Finger();
	Finger( Hand *hand, int id, const ci::Vec3f &initPos );
	void	update( const ci::Vec3f &basePos, const Leap::Finger &finger );
	void	drawBones( ci::gl::GlslProg *shader ) const;
	void	drawSplines() const;
	void	drawKnuckles() const;
	void	startDying();
	bool	isDead() const;
	
	Hand					*mHand;

	int						mId;
	ci::Vec3f				mPos0, mPos1, mPos2, mPos3;
	ci::Vec3f				mVel;
	ci::Vec3f				mDir0, mDir1, mDir2;
	float					mLength0, mLength1, mLength2;
	float					mRadius0, mRadius1, mRadius2;
	
	int						mLen;
	std::vector<ci::Vec3f>	mPs;
	std::vector<ci::Vec3f>	mVs;

	float					mAlpha;
	bool					mIsDying;
};