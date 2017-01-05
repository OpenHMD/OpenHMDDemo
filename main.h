/*
* OpenHMD - Free and Open Source API and drivers for immersive technology.
* Copyright (C) 2013 Fredrik Hultin.
* Copyright (C) 2013 Jakob Bornecrantz.
* OpenHMDDemo is designed and written by Joey Ferwerda from Thorworks game development studio.
* Distributed under the Boost 1.0 licence, see LICENSE for full text.
*/
#ifndef __Application_h_
#define __Application_h_

#include "DotSceneLoader.h"
#include "OpenHMD.h"

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>
#include <OgreCompositorManager.h>
#include <OgreCompositorInstance.h>
#include <OgreCompositorChain.h>

//Bullet Ogre includes
#include "BtOgrePG.h"
#include "BtOgreGP.h"
#include "BtOgreExtras.h"

//OIS (for Input)
#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

namespace Globals
{
    btDynamicsWorld *phyWorld;
    BtOgre::DebugDrawer *dbgdraw;
};

class Application : public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener, public Ogre::WindowEventListener
{
public:
    Application(void);
    virtual ~Application(void);
    bool init(void);
    void bulletInit();

    //Set of movement variables
    bool moveForward;
    bool moveBack;
    bool moveLeft;
    bool moveRight;

    Ogre::String level;
protected:
    Ogre::Root *mRoot;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

    //Player
    Ogre::SceneNode* mCamera;
    Ogre::Entity* mPlayerEntity;
    btRigidBody *mPlayerBody;
    btCollisionShape *mPlayerShape;

    //Bullet
    btAxisSweep3 *mBroadphase;
    btDefaultCollisionConfiguration *mCollisionConfig;
    btCollisionDispatcher *mDispatcher;
    btSequentialImpulseConstraintSolver *mSolver;

    bool mCursorWasVisible;                   // was cursor visible before dialog appeared
    bool mShutDown;

    // OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    // Ogre::WindowEventListener
    virtual void windowResized(Ogre::RenderWindow* rw);
    virtual void windowClosed(Ogre::RenderWindow* rw);

    //OpenHMD objects
    OpenHMD* openhmd;
	Ogre::Viewport* leftVP;
	Ogre::Viewport* rightVP;
	Ogre::Camera* stereo_cam_left;
	Ogre::Camera* stereo_cam_right;
};

#endif // #ifndef __Application_h_
