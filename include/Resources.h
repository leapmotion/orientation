#ifndef __Resources_h__
#define __Resources_h__

#include <cinder/CinderResources.h>

#define ICON_IDX 1

#define RES_APP_ICON          CINDER_RESOURCE( ../resources/, leap-icon.ico, 150, ICON )
#define RES_GLOW_PNG		  CINDER_RESOURCE( ../resources/, glow.png,			151, IMAGE )
#define RES_FINGER_FRAG	  CINDER_RESOURCE( ../resources/, finger_frag.glsl,		153, GLSL )
#define RES_FINGER_VERT	  CINDER_RESOURCE( ../resources/, finger_vert.glsl,		154, GLSL )
#define RES_GLOW_FRAG     CINDER_RESOURCE( ../resources/, blur_frag.glsl, 157, GLSL )
#define RES_GLOW_VERT     CINDER_RESOURCE( ../resources/, blur_vert.glsl, 158, GLSL )
#define RES_MOTION_FRAG   CINDER_RESOURCE( ../resources/, motion_frag.glsl, 159, GLSL )
#define RES_MOTION_VERT   CINDER_RESOURCE( ../resources/, pass_thru_vert.glsl, 160, GLSL )
#define RES_LOGO_PNG      CINDER_RESOURCE( ../resources/, leap_motion_logo.png, 164, IMAGE )
#define RES_PLUG_IN_PNG CINDER_RESOURCE( ../resources/, plug-in-device.png, 165, IMAGE)
#define RES_PLUG_IN_PNG_PONGO CINDER_RESOURCE( ../resources/, plug-in-device-pongo.png, 166, IMAGE)
#define RES_PLUG_IN_PNG_HOPS CINDER_RESOURCE( ../resources/, plug-in-device-hops.png, 167, IMAGE)

#define RES_BASS_OGG CINDER_RESOURCE( ../assets/, bass.ogg, 169, OGG )
#define RES_DRAWING_OGG CINDER_RESOURCE( ../assets/, drawing.ogg, 170, OGG )
#define RES_FLUID_OGG CINDER_RESOURCE( ../assets/, fluid.ogg, 171, OGG )
#define RES_GLOW_OGG CINDER_RESOURCE( ../assets/, glow.ogg, 172, OGG )
#define RES_HAND_OGG CINDER_RESOURCE( ../assets/, hand.ogg, 173, OGG )
#define RES_ROTATE_OGG CINDER_RESOURCE( ../assets/, rotate.ogg, 174, OGG )
#define RES_WAIT_OGG CINDER_RESOURCE( ../assets/, wait.ogg, 175, OGG )

#endif
