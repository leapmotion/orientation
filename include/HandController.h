#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Sphere.h"

#include "Hand.h"
#include "Finger.h"
#include "Leap.h"

#include <map>
#include <list>

class HandController {
  public:	
	HandController( ci::Camera *cam );
	void createSphere( ci::gl::VboMesh &mesh, int res );
	void addHand();
	void setShader( ci::gl::GlslProg *fingerShader );
	void update( const Leap::HandList &hands );
	void draw();
	void drawHands( ci::gl::Texture &glowTex, bool glowsOnly = false );
	void reset();

	ci::Camera					    *mCam;
	
	ci::gl::GlslProg				*mFingerShader;
	
	std::map<uint32_t,Hand>			mHands;
	std::map<uint32_t,Leap::Hand>	mLeapHands;
};
