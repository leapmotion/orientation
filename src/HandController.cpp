#include <cinder/Rand.h>

#include "Globals.h"
#include "HandController.h"
#include "Utils.h"

using namespace ci;
using namespace std;

HandController::HandController( Camera *cam )
	: mCam( cam )
{
	mLeapHands.clear();
	mHands.clear();
}

void HandController::setShader( gl::GlslProg *fingerShader )
{
	mFingerShader = fingerShader;
}

void HandController::update( const Leap::HandList &handList )
{
	map<uint32_t,bool>			foundIds;
	map<uint32_t,Leap::Hand>	newHands, movedHands;
	
	// mark all existing touches as not found for now
	for( map<uint32_t,Leap::Hand>::const_iterator it = mLeapHands.begin(); it != mLeapHands.end(); ++it ){
		foundIds[it->first] = false;
	}
	
	const int numHands = handList.count();
	if( numHands > 0 ){
		for( int h=0; h<numHands; h++ ){
			const Leap::Hand& hand	= handList[h];
			uint32_t id				= hand.id();
			
			if( mLeapHands.find( id ) == mLeapHands.end() )
				newHands[id]		= hand;
			else
				movedHands[id]		= hand;
			
			foundIds[id] = true;
		}
	}
	mLeapHands.clear();
	
	// anybody we didn't find must have ended
	for( map<uint32_t,bool>::const_iterator foundIt = foundIds.begin(); foundIt != foundIds.end(); ++foundIt ){
		if( ! foundIt->second ){
			uint32_t id = foundIt->first;
			mHands[id].startDying();
			mHands.erase( id );
		}
	}
	
	// NEW HANDS
	if( ! newHands.empty() ) {
		for( map<uint32_t,Leap::Hand>::const_iterator it = newHands.begin(); it != newHands.end(); ++it ){
			uint32_t id				= it->first;
			Leap::Hand hand			= it->second;
			
			mLeapHands[id]	= hand;
			mHands.insert( make_pair( id, Hand() ) );
		}
	}
	
	// MOVED HANDS
	if( ! movedHands.empty() ) {
		for( map<uint32_t,Leap::Hand>::const_iterator it = movedHands.begin(); it != movedHands.end(); ++it ){
			uint32_t id				= it->first;
			Leap::Hand hand			= it->second;
			
			mLeapHands[id]	= hand;
			mHands[id].update( hand );
		}
	}
}

void HandController::draw()
{	
  glLineWidth(3.0f);
	for( map<uint32_t,Hand>::const_iterator it = mHands.begin(); it != mHands.end(); ++it ) {
		it->second.draw();
	}
}

void HandController::drawHands( gl::Texture &glowTex, bool glowsOnly )
{
  glLineWidth(3.0f);
  if (!glowsOnly) {
	  Matrix44f mvpMatrix = mCam->getProjectionMatrix() * mCam->getModelViewMatrix();
	  Vec3f eyePos		= mCam->getEyePoint();

	  for( map<uint32_t,Hand>::const_iterator it = mHands.begin(); it != mHands.end(); ++it ) {
		  it->second.drawFingerSplines();
	  }

	  gl::enableAlphaBlending();

	  // FINGER BONES
    if ( mFingerShader ) {
	    mFingerShader->bind();
	    mFingerShader->uniform( "eyePos", eyePos );
	    mFingerShader->uniform( "col", cBlue );
	    mFingerShader->uniform( "time", (float)app::getElapsedSeconds() );
	    mFingerShader->uniform( "mvpMatrix", mvpMatrix );
    }
	  for( map<uint32_t,Hand>::const_iterator it = mHands.begin(); it != mHands.end(); ++it ) {
		  it->second.drawFingerBones( mFingerShader );
	  }
    if ( mFingerShader ) {
	    mFingerShader->unbind();
    }

	  gl::enableAlphaBlending();
	  gl::enableAdditiveBlending();
  }

	// EVERYTHING ELSE
	for( map<uint32_t,Hand>::const_iterator it = mHands.begin(); it != mHands.end(); ++it ) {
    if (!glowsOnly) {
		  it->second.drawFingerKnuckles();
		  it->second.drawPalm();
    }
		it->second.drawFingerTipGlows( mCam, glowTex );
	}
}

void HandController::reset()
{
	mHands.clear();
}
