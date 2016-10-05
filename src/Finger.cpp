#include <cinder/gl/gl.h>
#include <cinder/Utilities.h>
#include "Hand.h"
#include "Finger.h"
#include "Globals.h"

using namespace ci;

Finger::Finger()
{
}

Finger::Finger( Hand *hand, int id, const Vec3f &initPos )
	: mHand( hand ), mId( id ), mPos0( initPos )
{
	mAlpha		= 0.0f;
	mLen		= 8;
	mIsDying	= false;
	
	for( int i=0; i<mLen; i++ ){
		mPs.push_back( initPos );
		mVs.push_back( Vec3f::zero() );
	}
	
	mLength0	= 23.0f;
	mLength1	= 24.0f;
	mLength2	= 25.0f;
	
	mRadius0	= 1.0f;
	mRadius1	= 1.0f;
	mRadius2	= 1.0f;
}

void Finger::update( const Vec3f &basePos, const Leap::Finger &finger )
{
	Vec3f prevPos	= mPos0;
	mPos0			= toVecModified( finger.tipPosition() );
	mDir0			= toVec( finger.direction() );
	mVel			= mPos0 - prevPos;
	
	mPos1			= mPos0 + mDir0 * -mLength0;
	Vec3f handDir	= ( mPos1 - basePos ).normalized();
	mDir1			= ( mDir0 + handDir ).normalized();
	mPos2			= mPos1 + mDir1 * -mLength1;
	handDir			= ( mPos2 - basePos ).normalized();
	mDir2			= ( mDir1 + handDir ).normalized();
	mPos3			= mPos2 + mDir2 * -mLength2;
	
	for( int i=mLen-1; i>0; i-- ){
		mPs[i] = mPs[i-1];
		mVs[i] = mVs[i-1];
	}
	mPs[0] = mPos0;
	mVs[0] = mVel;
	
//	mAngleToPalm	=
	
	mAlpha -= ( mAlpha - 1.0f ) * 0.1f;
}

void Finger::drawKnuckles() const
{
	if( mAlpha > 0.0f ){
		// SPHERES
		gl::color( ColorA( cYellow, mAlpha ) );
		gl::drawSphere( mPos0, 1.5f );
		gl::color( ColorA( cOrange, mAlpha ) );
		gl::drawSphere( mPos1, 1.5f );
		gl::drawSphere( mPos2, 1.5f );
		gl::drawSphere( mPos3, 1.5f );
		
		Vec3f dir;
		Quatf q;
		
		// KNUCKLE RINGS
		gl::pushModelView();
		dir = ( mPos1 - mPos0 ).normalized();
		q = Quatf( Vec3f::zAxis(), dir );
		gl::translate( mPos0 + dir * 2.0f );
		gl::rotate( q );
		gl::color( ColorA( cBlue, 0.3f ) );
		gl::drawStrokedCircle( Vec2f::zero(), 2.5f * mAlpha );
		gl::popModelView();
		
		gl::pushModelView();
		dir = ( mPos1 - mPos0 ).normalized();
		q = Quatf( Vec3f::zAxis(), dir );
		gl::translate( mPos1 - dir * 2.0f );
		gl::rotate( q );
		gl::color( ColorA( cBlue, 0.3f ) );
		gl::drawStrokedCircle( Vec2f::zero(), 3.5f * mAlpha );
		gl::popModelView();
		
		gl::pushModelView();
		dir = ( mPos2 - mPos1 ).normalized();
		q = Quatf( Vec3f::zAxis(), dir );
		gl::translate( mPos1 + dir * 3.0f );
		gl::rotate( q );
		gl::color( ColorA( cBlue, 0.3f ) );
		gl::drawStrokedCircle( Vec2f::zero(), 3.5f * mAlpha );
		gl::popModelView();
		
		gl::pushModelView();
		dir = ( mPos2 - mPos1 ).normalized();
		q = Quatf( Vec3f::zAxis(), dir );
		gl::translate( mPos2 - dir * 3.0f );
		gl::rotate( q );
		gl::color( ColorA( cBlue, 0.3f ) );
		gl::drawStrokedCircle( Vec2f::zero(), 4.5f * mAlpha );
		gl::popModelView();
		
		gl::pushModelView();
		dir = ( mPos3 - mPos2 ).normalized();
		q = Quatf( Vec3f::zAxis(), dir );
		gl::translate( mPos2 + dir * 3.0f );
		gl::rotate( q );
		gl::color( ColorA( cBlue, 0.3f ) );
		gl::drawStrokedCircle( Vec2f::zero(), 4.5f * mAlpha );
		gl::popModelView();
		
		gl::pushModelView();
		dir = ( mPos3 - mPos2 ).normalized();
		q = Quatf( Vec3f::zAxis(), dir );
		gl::translate( mPos3 - dir * 3.0f );
		gl::rotate( q );
		gl::color( ColorA( cBlue, 0.3f ) );
		gl::drawStrokedCircle( Vec2f::zero(), 5.5f * mAlpha );
		gl::popModelView();
	}
}

void Finger::drawSplines() const
{	
	if( mAlpha > 0.0f ){
		gl::color( ColorA( Color::white(), mAlpha ) );
		gl::drawLine( mPos1, mPos0 );
		gl::drawLine( mPos2, mPos1 );
		gl::drawLine( mPos3, mPos2 );
	}

	// TIP TO SPHERE
	if( mHand->mFingers.size() > 0 && mAlpha > 0.0f ){
		gl::color( ColorA( cYellow, 0.3f ) );
		gl::drawLine( mPos3, mHand->mPos );
	}
}

void Finger::drawBones( gl::GlslProg *shader ) const
{
	if( mAlpha > 0.0f && shader ){
		Quatf quat;
		Matrix44f mat;
		float inset = 2.5f;
		
		mat.setToIdentity();
		mat.translate( mPos0 - mDir0 * inset );
		quat = Quatf( Vec3f::yAxis(), -mDir0 );
		mat *= quat.toMatrix44();
		shader->uniform( "mMatrix", mat );
		shader->uniform( "alpha", mAlpha );
		gl::drawCylinder( 0.5, mRadius0, mLength0 - inset * 2.0f );
		
		mat.setToIdentity();
		mat.translate( mPos1 - mDir1 * inset );
		quat = Quatf( Vec3f::yAxis(), -mDir1 );
		mat *= quat.toMatrix44();
		shader->uniform( "mMatrix", mat );
		shader->uniform( "alpha", mAlpha );
		gl::drawCylinder( mRadius0, mRadius1, mLength1 - inset * 2.0f );
		
		mat.setToIdentity();
		mat.translate( mPos2 - mDir2 * inset );
		quat = Quatf( Vec3f::yAxis(), -mDir2 );
		mat *= quat.toMatrix44();
		shader->uniform( "mMatrix", mat );
		shader->uniform( "alpha", mAlpha );
		gl::drawCylinder( mRadius1, mRadius2, mLength2 - inset * 2.0f );
	}
}

void Finger::startDying()
{
	mIsDying = true;
}

bool Finger::isDead() const
{
	return mIsDying;
}