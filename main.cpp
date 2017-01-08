/*
-----------------------------------------------------------------------------
  /$$$$$$                                /$$   /$$ /$$      /$$ /$$$$$$$
 /$$__  $$                              | $$  | $$| $$$    /$$$| $$__  $$
| $$  \ $$  /$$$$$$   /$$$$$$  /$$$$$$$ | $$  | $$| $$$$  /$$$$| $$  \ $$
| $$  | $$ /$$__  $$ /$$__  $$| $$__  $$| $$$$$$$$| $$ $$/$$ $$| $$  | $$
| $$  | $$| $$  \ $$| $$$$$$$$| $$  \ $$| $$__  $$| $$  $$$| $$| $$  | $$
| $$  | $$| $$  | $$| $$_____/| $$  | $$| $$  | $$| $$\  $ | $$| $$  | $$
|  $$$$$$/| $$$$$$$/|  $$$$$$$| $$  | $$| $$  | $$| $$ \/  | $$| $$$$$$$/
 \______/ | $$____/  \_______/|__/  |__/|__/  |__/|__/     |__/|_______/
          | $$
          | $$
          |__/
-----------------------------------------------------------------------------
* OpenHMD - Free and Open Source API and drivers for immersive technology.
* Copyright (C) 2013 Fredrik Hultin.
* Copyright (C) 2013 Jakob Bornecrantz.
* OpenHMDDemo is designed and written by Joey Ferwerda on behalf of Thorworks game development studio.
* Distributed under the Boost 1.0 licence, see LICENSE for full text.
*/

#include "main.h"
#include "OgreEuler.h"
#include "OgreHelpers.h"

Application::Application(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
    moveForward(false),
    moveBack(false),
    moveLeft(false),
    moveRight(false),
    stereo_cam_left(0),
    stereo_cam_right(0),
    mPlayerNode(0),
    debugPhysics(false)
{
}

Application::~Application(void)
{
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);

        //Free Bullet stuff.
    delete Globals::dbgdraw;
    delete Globals::phyWorld;
    delete mSolver;
    delete mDispatcher;
    delete mCollisionConfig;
    delete mBroadphase;

    delete mRoot;
    delete openhmd;
}

void Application::bulletInit()
{
    mBroadphase = new btAxisSweep3(btVector3(-10000,-10000,-10000), btVector3(10000,10000,10000), 1024);
    mCollisionConfig = new btDefaultCollisionConfiguration();
    mDispatcher = new btCollisionDispatcher(mCollisionConfig);
    mSolver = new btSequentialImpulseConstraintSolver();

    Globals::phyWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
    Globals::phyWorld->setGravity(btVector3(0,-9.81,0));
}

void Application::lazyCollisions()
{
    //Convert Player
    BtOgre::StaticMeshToShapeConverter converter(mPlayerEntity);
    mPlayerShape = converter.createConvex();

    //Calculate inertia.
    btScalar mass = 5;
    btVector3 inertia;
    mPlayerShape->calculateLocalInertia(mass, inertia);

    //Create BtOgre MotionState (connects Ogre and Bullet).
    BtOgre::RigidBodyState* playerRBState = new BtOgre::RigidBodyState(mPlayerNode);

    //Create the Body.
    mPlayerBody = new btRigidBody(mass, playerRBState, mPlayerShape, inertia);
    mPlayerBody->setAngularFactor(btVector3(0,0,0));
    mPlayerBody->setActivationState(DISABLE_DEACTIVATION);
    Globals::phyWorld->addRigidBody(mPlayerBody);

    //Convert all objects which are not the Player to a trimesh collision object
    Ogre::SceneManager::MovableObjectIterator iterator = mSceneMgr->getMovableObjectIterator("Entity");
    while(iterator.hasMoreElements())
    {
        Ogre::Entity* eobj = static_cast<Ogre::Entity*>(iterator.getNext());

        if (eobj->getName() == "PlayerCube")
        {
        }
        else
        {
            Ogre::AxisAlignedBox aabb = eobj->getBoundingBox();
            printf("AABB size of object %s is %f %f %f\n", eobj->getName().c_str(), aabb.getSize().x, aabb.getSize().y, aabb.getSize().z);
            //Create the ground shape.
            BtOgre::StaticMeshToShapeConverter converter2(eobj);
            btBvhTriangleMeshShape* mCollisionShape = converter2.createTrimesh();

            //Create MotionState (no need for BtOgre here, you can use it if you want to though).
            btDefaultMotionState* collisionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));

            //Create the Body.
            btRigidBody* mCollisionBody = new btRigidBody(0, collisionState, mCollisionShape, btVector3(0,0,0));
            Globals::phyWorld->addRigidBody(mCollisionBody);
        }
    }
}

bool Application::init(void)
{
    printf("Starting OpenHMD OGRE Demo\n");
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";

    // construct Ogre::Root
    mRoot = new Ogre::Root(mPluginsCfg);

    // setup resources
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    // configure
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog()) //mRoot->restoreConfig()
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "OpenHMD Space Demo");
    }
    else
    {
        return false;
    }

    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

    // Create the cameras and cameraNode
    mPlayerNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("mPlayerNode");
    mPlayerEntity = mSceneMgr->createEntity("PlayerCube", Ogre::SceneManager::PT_CUBE);
    mPlayerEntity->setMaterialName("TestMaterial");
    mPlayerNode->attachObject(mPlayerEntity);
    mPlayerNode->setScale(0.011,0.0172,0.006);

    mCamera = mSceneMgr->getRootSceneNode()->createChildSceneNode("mCamera");
    mCamera->setPosition(Ogre::Vector3(0, 1.72/2, 0));
    attachInPlace(mSceneMgr->getRootSceneNode(), mCamera, mPlayerNode);

    stereo_cam_left = mSceneMgr->createCamera("leftCam");
    stereo_cam_right = mSceneMgr->createCamera("rightCam");

    mPlayerNode->setPosition(Ogre::Vector3(0,1,0)); //Spawn position

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);


    // load resources
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    // Create the scene
    Ogre::DotSceneLoader* dotSceneLoader = new Ogre::DotSceneLoader();
    dotSceneLoader->parseDotScene(level, "General", mSceneMgr);

    // Set ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

    // Create a light
    Ogre::Light* l = mSceneMgr->createLight("MainLight");
    l->setPosition(20,80,50);

    //create FrameListener
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	//setup OpenHMD
	openhmd = new OpenHMD();
	openhmd->init();

	//Create a camera setup
	stereo_cam_left->setPosition(openhmd->getLeftViewMatrix().getTrans());
    stereo_cam_right->setPosition(openhmd->getRightViewMatrix().getTrans());
    //Set clip distances
    stereo_cam_left->setNearClipDistance(0.000012);
    stereo_cam_left->setFarClipDistance(200*120);
    stereo_cam_right->setNearClipDistance(0.000012);
    stereo_cam_right->setFarClipDistance(200*120);

    stereo_cam_left->detachFromParent();
    stereo_cam_right->detachFromParent();

    mCamera->attachObject(stereo_cam_left);
    mCamera->attachObject(stereo_cam_right);

    //setup viewports and compositor
    mWindow->removeAllViewports();
    leftVP = mWindow->addViewport(stereo_cam_left, 1, 0, 0, 0.5f, 1.0f);
    rightVP = mWindow->addViewport(stereo_cam_right, 2, 0.5f, 0, 0.5f, 1.0f);

    leftVP->setBackgroundColour(Ogre::ColourValue(0.145f, 0.25f, 0.4f));
    rightVP->setBackgroundColour(Ogre::ColourValue(0.145f, 0.25f, 0.4f));

	//Get physical screen resolution and use closest available compositor with stretching
    Ogre::Vector2 hmdScreenSize = openhmd->getScreenSize();

    if (!openhmd->isDummy())
    {
        if (hmdScreenSize[0] == 1280 && hmdScreenSize[1] == 800) //assume Oculus DK1 shader
        {
            Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/OculusDK1");
            Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/OculusDK1");
            leftComp->setEnabled(true);
            rightComp->setEnabled(true);
        }
        else if (hmdScreenSize[0] == 1920 && hmdScreenSize[1] == 1080) //assume Oculus DK2 shader
        {
            Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/OculusDK2");
            Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/OculusDK2");
            leftComp->setEnabled(true);
            rightComp->setEnabled(true);
        }
        else
        {
            Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/GenericAutoScaling");
            Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/GenericAutoScaling");
            leftComp->setEnabled(true);
            rightComp->setEnabled(true);
        }
    }
    else
    {
        Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/GenericAutoScaling");
        Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/GenericAutoScaling");
        leftComp->setEnabled(true);
        rightComp->setEnabled(true);
    }

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    ///Bullet Stuff
    bulletInit();
    Globals::dbgdraw = new BtOgre::DebugDrawer(mSceneMgr->getRootSceneNode(), Globals::phyWorld);
    Globals::phyWorld->setDebugDrawer(Globals::dbgdraw);
    Globals::dbgdraw->setDebugMode(debugPhysics);
    lazyCollisions();

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
    mRoot->addFrameListener(this);
    mRoot->startRendering();

    return true;
}

//this is basically the main loop
bool Application::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

	//Update camera movement
    Ogre::Vector3 movement = Ogre::Vector3::ZERO;
	if (moveForward) movement.z -=4;
	if (moveBack) movement.z +=4;
	if (moveLeft) movement.x -=4;
	if (moveRight) movement.x +=4;

    if (movement != Ogre::Vector3::ZERO)
    {
        movement = mPlayerNode->_getDerivedOrientation()*movement;
        mPlayerBody->setLinearVelocity(btVector3(movement.x, movement.y, movement.z));
    }
    else //stop
    {
        mPlayerBody->setLinearVelocity(btVector3(0, mPlayerBody->getLinearVelocity().y(), 0));
    }

	//Update OpenHMD
	openhmd->update();
    stereo_cam_left->setCustomProjectionMatrix(true, openhmd->getLeftProjectionMatrix().transpose());
    stereo_cam_right->setCustomProjectionMatrix(true, openhmd->getRightProjectionMatrix().transpose());
    Ogre::Quaternion oculusCameraOrientation = openhmd->getQuaternion();
    stereo_cam_left->setOrientation(oculusCameraOrientation);
    stereo_cam_right->setOrientation(oculusCameraOrientation);

    //Update Bullet world. Don't forget the debugDrawWorld() part!
    Globals::phyWorld->stepSimulation(evt.timeSinceLastFrame, 10);
    Globals::phyWorld->debugDrawWorld();

    Globals::dbgdraw->step();

    return true;
}

bool Application::keyPressed( const OIS::KeyEvent &arg )
{
	switch (arg.key)
	{
		case OIS::KC_W: moveForward = true; break;
		case OIS::KC_S: moveBack = true; break;
		case OIS::KC_A: moveLeft = true; break;
		case OIS::KC_D: moveRight = true; break;
        case OIS::KC_SPACE: mPlayerBody->applyImpulse(btVector3(0,20,0), btVector3(0,0,0)); break;

        case OIS::KC_L: // toggle the lens correction shader
        {
            Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->getCompositorChain(leftVP)->getCompositor(0);
            Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->getCompositorChain(rightVP)->getCompositor(0);
            leftComp->setEnabled(!leftComp->getEnabled());
            rightComp->setEnabled(!rightComp->getEnabled());
            break;
        }

		case OIS::KC_R:   // cycle polygon rendering mode
		{
			Ogre::String newVal;
			Ogre::PolygonMode pm;

			switch (stereo_cam_left->getPolygonMode())
			{
			case Ogre::PM_SOLID:
				newVal = "Wireframe";
				pm = Ogre::PM_WIREFRAME;
				break;
			case Ogre::PM_WIREFRAME:
				newVal = "Points";
				pm = Ogre::PM_POINTS;
				break;
			default:
				newVal = "Solid";
				pm = Ogre::PM_SOLID;
			}

			stereo_cam_left->setPolygonMode(pm);
			stereo_cam_right->setPolygonMode(pm);
			break;
		}
		case OIS::KC_F5: Ogre::TextureManager::getSingleton().reloadAll(); break; // refresh all textures
        case OIS::KC_F6: Globals::dbgdraw->setDebugMode((debugPhysics = !debugPhysics)); break; //Shows debug if F3 key down.
		case OIS::KC_SYSRQ: mWindow->writeContentsToTimestampedFile("screenshot", ".jpg"); break; // take a screenshot
		case OIS::KC_ESCAPE: mShutDown = true; break;

		return true;
	}
}

bool Application::keyReleased( const OIS::KeyEvent &arg )
{
	switch (arg.key)
	{
		case OIS::KC_W: moveForward = false; break;
		case OIS::KC_S: moveBack = false; break;
		case OIS::KC_A: moveLeft = false; break;
		case OIS::KC_D: moveRight = false; break;
	}

    return true;
}

bool Application::mouseMoved( const OIS::MouseEvent &arg )
{
    int relX = arg.state.X.rel;
    int relY = arg.state.Y.rel;
	if ( relX != 0 )
	{
		//horizontal rotation
        Ogre::Quaternion rotX;
		if (relX > 0)
			rotX = Ogre::Euler(-(float)relX/100, 0, 0).toQuaternion();
		else
			rotX = Ogre::Euler(-(float)relX/100, 0, 0).toQuaternion();

        mPlayerNode->rotate(rotX, Ogre::SceneNode::TS_LOCAL);
	}

	if ( relY != 0 )
	{
        Ogre::Quaternion rotZ;
		//vertical rotation
		if (relY > 0)
			rotZ = Ogre::Euler(0, -relY/100, 0).toQuaternion();
		else
			rotZ = Ogre::Euler(0, relY/100, 0).toQuaternion();

        //mPlayerNode->rotate(rotZ, Ogre::SceneNode::TS_LOCAL);
        ///NOTE: For HMD games, Rolling the camera is bad, should be left to the player.
	}

    btTransform trans = mPlayerBody->getWorldTransform();
    Ogre::Quaternion v = mPlayerNode->_getDerivedOrientation();
    trans.setRotation(btQuaternion(v.x, v.y, v.z, v.w));
    mPlayerBody->setWorldTransform(trans);

    return true;
}

bool Application::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    return true;
}

bool Application::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    return true;
}

//Adjust mouse clipping area
void Application::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void Application::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
		Ogre::String level = "resources/PhysicsWarehouse/PhysicsWarehouse.scene";
		if ( argc > 1 ) // There are Command line parameters
		{
			for(int i = 1; i < argc; i++)
			{
				std::string cmdline = argv[i];

				if (cmdline == "-l")
				{
					if ( i < argc-1 ) // If there's at least another argument left
					{
						i++;
						level = argv[i];
					}
				}
			}
		}
        // Create application object
        Application app;
		app.level = level;

        try {
            app.init();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
