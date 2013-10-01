#include <cinder/Rand.h>
#include "Hand.h"
#include "Globals.h"

using namespace ci;
using namespace std;

Hand::Hand()
{
	mIsDying		= false;
	mAlpha			= 0.0f;
	mPower			= 0.0f;
}

void Hand::update( const Leap::Hand &hand )
{	
	mAlpha		-= ( mAlpha - 1.0f ) * 0.1f;
	
	mPos		= toVecModified( hand.palmPosition() );
	mDir		= toVec( hand.direction() );
	mPos		-= mDir * 15.0f;
	mBasePos	= mPos + mDir * -75.0f;
	mPalmPos	= mPos * 0.35f + mBasePos * 0.65f;
	mNormal		= toVec( hand.palmNormal() );
	mAxis		= mDir.cross( mNormal );
	mRightPos	= mBasePos - mAxis * 10.0f;
	mLeftPos	= mBasePos + mAxis * 10.0f;
	
	map<uint32_t,bool>			foundIds;
	map<uint32_t,Leap::Finger>  newFingers, movedFingers;
	
	// mark all existing touches as not found for now
	for( map<uint32_t,Leap::Finger>::const_iterator it = mLeapFingers.begin(); it != mLeapFingers.end(); ++it ){
		foundIds[it->first] = false;
	}
	
	const Leap::FingerList &fingerList	= hand.fingers();
	const int numFingers				= fingerList.count();
	if( numFingers > 0 ) {
		for( int i=0; i<numFingers; ++i ){
			
			const Leap::Finger& finger	= fingerList[i];
			uint32_t id					= finger.id();
			
			if( mLeapFingers.find( id ) == mLeapFingers.end() )
				newFingers[id]	 = finger;
			else
				movedFingers[id] = finger;
			
			foundIds[id] = true;
		}
	}
	mLeapFingers.clear();

	// anybody we didn't find must have ended
	for( map<uint32_t,bool>::const_iterator it = foundIds.begin(); it != foundIds.end(); ++it ){
		if( ! it->second ){
			uint32_t id = it->first;
			mFingers[id].startDying();
			mFingers.erase( id );
		}
	}
	
	
	// create new fingers
	if( ! newFingers.empty() ) {
		for( map<uint32_t,Leap::Finger>::const_iterator it = newFingers.begin(); it != newFingers.end(); ++it ){
			uint32_t id				= it->first;
			Leap::Finger finger		= it->second;
			
			mLeapFingers[id]	= finger;
			mFingers.insert( make_pair( id, Finger( this, id, toVecModified( finger.tipPosition() ) ) ) );
		}
	}
	
	// move the fingers that have moved
	if( ! movedFingers.empty() ) {
		for( map<uint32_t,Leap::Finger>::const_iterator it = movedFingers.begin(); it != movedFingers.end(); ++it ){
			uint32_t id				= it->first;
			Leap::Finger finger		= it->second;
			
			mLeapFingers[id]	= finger;
			mFingers[id].update( mBasePos, finger );
		}
	}
	
		mPower		-= ( mPower - 1.0f ) * 0.1f;
}

void Hand::draw() const
{
	// BACK OF HAND
	gl::color( ColorA( cOrange, mAlpha * mPower ) );
	gl::drawSphere( mPos, 2.0f );
	gl::drawSphere( mPalmPos - mDir * 10.0f, 4.0f );
	gl::drawSphere( mPalmPos, 2.0f );
	gl::drawLine( mPalmPos, mPos );
	
	gl::color( ColorA( cBlue, mAlpha * mPower ) );
	gl::drawSphere( mRightPos, 2.0f );
	gl::drawSphere( mLeftPos, 2.0f );
	gl::drawSphere( mLeftPos + mDir * 47.0f, 1.0f );
	gl::drawSphere( mRightPos + mDir * 47.0f, 1.0f );
	gl::color( ColorA( cBlue, mAlpha * mPower * 0.4f ) );
	gl::drawLine( mRightPos, mRightPos + mDir * 47.0f );
	gl::drawLine( mLeftPos, mLeftPos + mDir * 47.0f );
	
	// WRIST CIRCLES
	gl::pushModelView();
	gl::translate( mBasePos );
  gl::rotate( Matrix44<float>(mNormal.cross(mDir), -mNormal, -mDir));
	gl::translate( Vec3f::yAxis() * -20.0f );
	gl::scale( Vec3f( 1.0f, 0.6f, 1.0f ) );
	gl::drawStrokedCircle( Vec2f::zero(), 32.0f );
	gl::translate( Vec3f::zAxis() * 5.0f );
	gl::drawStrokedCircle( Vec2f::zero(), 35.0f );
	gl::popModelView();
}

void Hand::drawPalm() const
{
	gl::color( ColorA( cBlue, mAlpha * mPower * 0.25f ) );
	gl::pushModelView();
	gl::translate( mPos );
	Quatf q = Quatf( -Vec3f::zAxis(), mNormal );
	gl::rotate( q );
	
	gl::drawStrokedCircle( Vec2f::zero(), 30.0f );
	
	gl::drawStrokedCircle( Vec2f::zero(), 10.0f );
	
	gl::translate( Vec3f::zAxis() * 4.0f );
	gl::color( ColorA( cBlue, mAlpha * mPower * 0.05f ) );
	gl::drawSolidCircle( Vec2f::zero(), 25.0f );
	
	gl::color( ColorA( cBlue, mAlpha * mPower * 0.25f ) );
	gl::drawStrokedCircle( Vec2f::zero(), 25.5f );
	
	gl::drawStrokedCircle( Vec2f::zero(), 8.0f );
	gl::popModelView();
}

void Hand::drawFingerBones( gl::GlslProg *shader ) const
{
	gl::color( ColorA( cBlue, 0.2f ) );
	for( map<uint32_t,Finger>::const_iterator it = mFingers.begin(); it != mFingers.end(); ++it ) {
		//it->second.drawBones( shader );
	}
}

void Hand::drawFingerSplines() const
{
	gl::enableAlphaBlending();
	gl::color( ColorA( cRed, 1.0f ) );
	for( map<uint32_t,Finger>::const_iterator it = mFingers.begin(); it != mFingers.end(); ++it ) {
		it->second.drawSplines();
	}
}

void Hand::drawFingerKnuckles() const
{
	for( map<uint32_t,Finger>::const_iterator it = mFingers.begin(); it != mFingers.end(); ++it ) {
		it->second.drawKnuckles();
	}
}

void Hand::drawFingerTipGlows( Camera *cam, gl::Texture &tex ) const
{
  Vec3f right, up;
	cam->getBillboardVectors( &right, &up );
	
	gl::enableAdditiveBlending();
	gl::disableDepthWrite();
	tex.enableAndBind();
	for( map<uint32_t,Finger>::const_iterator it = mFingers.begin(); it != mFingers.end(); ++it ) {
		gl::color( ColorA( cBlue, 0.5f ) );
		gl::drawBillboard( it->second.mPos0, Vec2f( 120.0f, 120.0f ), 0.0f, right, up );
		gl::color( ColorA( cRed, Rand::randFloat( 0.5f, 1.0f ) ) );
		gl::drawBillboard( it->second.mPos0, Vec2f( 40.0f, 40.0f ), 0.0f, right, up );
		gl::color( ColorA( cYellow, Rand::randFloat( 0.5f, 1.0f ) ) );
		gl::drawBillboard( it->second.mPos0, Vec2f( 15.0f, 15.0f ), 0.0f, right, up );
  }

	gl::enableAlphaBlending();
	gl::disable( GL_TEXTURE_2D );
}

void Hand::startDying()
{
	mIsDying	= true;
	for( map<uint32_t,Finger>::iterator it = mFingers.begin(); it != mFingers.end(); ++it ) {
		it->second.startDying();
	}
}

bool Hand::isDead() const
{
	return mIsDying;
}
