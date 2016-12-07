#ifdef _MSC_VER
// We'll define this to stop MSVC complaining about sprintf().
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Irrlicht.lib")
#endif

#include <iostream>
#include <irrlicht.h>
#include <rect.h>


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace std;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

using namespace irr;
const int ResX = 640;
const int ResY = 480;



class MouseEventReceiverFor3D : public IEventReceiver
{
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		// Remember whether each key is down or up
		if (event.EventType == irr::EET_KEY_INPUT_EVENT)
			KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

		return false;
	}

	// This is used to check whether a key is being held down
	virtual bool IsKeyDown(EKEY_CODE keyCode) const
	{
		return KeyIsDown[keyCode];
	}

	MouseEventReceiverFor3D()
	{
		for (u32 i = 0; i<KEY_KEY_CODES_COUNT; ++i)
			KeyIsDown[i] = false;
	}

private:
	// We use this array to store the current state of each key
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
};




class MouseEventReceiverFor2D : public IEventReceiver
{
public:
	// We'll create a struct to record info on the mouse state
	struct SMouseState
	{
		core::position2di Position;
		bool LeftButtonDown;
		SMouseState() : LeftButtonDown(false), Position(core::position2di((ResX) / 2, (ResY) / 2)) { }
	} MouseState;

	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		// Remember the mouse state
		if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
		{
			switch (event.MouseInput.Event)
			{
			case EMIE_LMOUSE_PRESSED_DOWN:
				if (event.MouseInput.X <ResX - 60 && event.MouseInput.X > 60
					&& event.MouseInput.Y < ResY - 60 && event.MouseInput.Y > 60) {
					MouseState.Position.X = event.MouseInput.X;
					MouseState.Position.Y = event.MouseInput.Y;
				}break;
			default:
				// We won't use the wheel
				break;
			}
		}

		// The state of each connected joystick is sent to us
		// once every run() of the Irrlicht device.  Store the
		// state of the first joystick, ignoring other joysticks.
		// This is currently only supported on Windows and Linux.
		if (event.EventType == irr::EET_JOYSTICK_INPUT_EVENT
			&& event.JoystickEvent.Joystick == 0)
		{
			JoystickState = event.JoystickEvent;
		}

		return false;
	}

	const SEvent::SJoystickEvent & GetJoystickState(void) const
	{
		return JoystickState;
	}

	const SMouseState & GetMouseState(void) const
	{
		return MouseState;
	}


	MouseEventReceiverFor2D()
	{

	}

private:
	SEvent::SJoystickEvent JoystickState;
};

IrrlichtDevice *deviceFor3D = 0;

void setActiveCamera(scene::ICameraSceneNode* newActive)
{
	if (0 == deviceFor3D)
		return;

	scene::ICameraSceneNode * active = deviceFor3D->getSceneManager()->getActiveCamera();
	active->setInputReceiverEnabled(false);

	newActive->setInputReceiverEnabled(true);
	deviceFor3D->getSceneManager()->setActiveCamera(newActive);
}



int main() {
	
	MouseEventReceiverFor2D mouseReceiver;  
	MouseEventReceiverFor3D receiverForPlate;

	deviceFor3D = createDevice(irr::video::EDT_OPENGL, dimension2d<u32>(ResX, ResY), 16, false, false, false, &receiverForPlate);
	IrrlichtDevice* deviceFor2D = createDevice(video::E_DRIVER_TYPE::EDT_DIRECT3D9,
		core::dimension2d<u32>( ResX, ResY), 16, false, false, false, &mouseReceiver);

	if (deviceFor2D == 0)
		return 1; // could not create selected driver.
	if (!deviceFor3D)
		return 1;
	deviceFor3D->setWindowCaption(L"Ball on Plate - 3D");

	video::IVideoDriver* driverFor2D = deviceFor2D->getVideoDriver();
	scene::ISceneManager* smgrFor2D = deviceFor2D->getSceneManager();
	gui::IGUIEnvironment *guienvFor2D = deviceFor2D->getGUIEnvironment();

	driverFor2D->getMaterial2D().TextureLayer[0].BilinearFilter = true;
	driverFor2D->getMaterial2D().AntiAliasing = video::EAAM_FULL_BASIC;



	IVideoDriver* driverFor3D = deviceFor3D->getVideoDriver();
	ISceneManager* smgrFor3D = deviceFor3D->getSceneManager();
	IGUIEnvironment* guienvFor3D = deviceFor3D->getGUIEnvironment();


	

	core::array<SJoystickInfo> joystickInfoFor2D;
	if (deviceFor2D->activateJoysticks(joystickInfoFor2D))
	{
		std::cout << "Joystick support is enabled and " << joystickInfoFor2D.size() << " joystick(s) are present." << std::endl;

		for (u32 joystick = 0; joystick < joystickInfoFor2D.size(); ++joystick)
		{
			std::cout << "Joystick " << joystick << ":" << std::endl;
			std::cout << "\tName: '" << joystickInfoFor2D[joystick].Name.c_str() << "'" << std::endl;
			std::cout << "\tAxes: " << joystickInfoFor2D[joystick].Axes << std::endl;
			std::cout << "\tButtons: " << joystickInfoFor2D[joystick].Buttons << std::endl;

			std::cout << "\tHat is: ";

			switch (joystickInfoFor2D[joystick].PovHat)
			{
			case SJoystickInfo::POV_HAT_PRESENT:
				std::cout << "present" << std::endl;
				break;

			case SJoystickInfo::POV_HAT_ABSENT:
				std::cout << "absent" << std::endl;
				break;

			case SJoystickInfo::POV_HAT_UNKNOWN:
			default:
				std::cout << "unknown" << std::endl;
				break;
			}
		}
	}
	else
	{
		std::cout << "Joystick support is not enabled." << std::endl;
	}

	core::stringw tmp = L"Irrlicht Joystick Example (";
	tmp += joystickInfoFor2D.size();
	tmp += " joysticks)";


	deviceFor2D->setWindowCaption(L"Ball on Plate - 2D");

	


	// 3D Part
	// Create Platform
	scene::ISceneNode* plateModelSceneNode = smgrFor3D->addEmptySceneNode();

	// Create Plate
	IAnimatedMesh* plateMesh = smgrFor3D->getMesh("Plate/plate.obj");
	ISceneNode* plateSceneNode = 0;
	if (plateMesh)
	{
		//node2 = smgr->addOctreeSceneNode(mesh2->getMesh(0), 0, -1, 1024);
		plateSceneNode = smgrFor3D->addMeshSceneNode(plateMesh->getMesh(0));
		plateSceneNode->setMaterialFlag(EMF_LIGHTING, false);
		//plateSceneNode->setPosition(core::vector3df(0, 2, 0));
		plateSceneNode->setScale(vector3df(2, 2, 3));
	}

	// Add ball as child to plate
	plateModelSceneNode->addChild(plateSceneNode);

	// Create Ball
	IAnimatedMesh* ballMesh = smgrFor3D->getMesh("Ball/ball.obj");
	ISceneNode* ballSceneNode = 0;
	if (ballMesh)
	{
		//node2 = smgr->addOctreeSceneNode(mesh2->getMesh(0), 0, -1, 1024);
		ballSceneNode = smgrFor3D->addMeshSceneNode(ballMesh->getMesh(0));
		ballSceneNode->setMaterialFlag(EMF_LIGHTING, false);
		ballSceneNode->setPosition(core::vector3df(0, 2, 0));
		ballSceneNode->setScale(vector3df(2, 2, 2));
	}

	scene::ISceneNode* cylinderSceneNode = smgrFor3D->addCubeSceneNode();
	cylinderSceneNode->setPosition(core::vector3df(1, -11, 0.2));
	cylinderSceneNode->setScale(vector3df(0.3, 2, 0.3));
	cylinderSceneNode->setMaterialFlag(EMF_LIGHTING, false);
	//cylinderSceneNode->setMD2Animation(scene::EMAT_STAND);
	cylinderSceneNode->setMaterialTexture(0, driverFor3D->getTexture("Cylinder/Cylinder.jpg"));
	// Add ball as child to plate
	plateModelSceneNode->addChild(ballSceneNode);

	smgrFor3D->addCameraSceneNode(0, vector3df(0, 30, -40), vector3df(0, 5, 0));
	scene::ICameraSceneNode *camera = smgrFor3D->addCameraSceneNodeMaya(0, -100.0f, 100.0f, 100.0f);
	camera->setFarValue(20000.f);
	camera->setTarget(core::vector3df(0, 30, 0));
	setActiveCamera(camera);
	

	while (deviceFor2D->run() && deviceFor3D->run())
	{

		core::vector3df platePosition = plateModelSceneNode->getPosition();
		core::vector3df plateRotation = plateModelSceneNode->getRotation();
		if (receiverForPlate.IsKeyDown(irr::KEY_KEY_A)) {
			plateRotation.X += 0.3;
		}
		//			nodePosition.Y += MOVEMENT_SPEED * frameDeltaTime;
		else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_D)) {
			plateRotation.X -= 0.3;
		}
		//		nodePosition.Y -= MOVEMENT_SPEED * frameDeltaTime;
		else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_W)) {
			plateRotation.Z += 0.3;
		}
		//	nodePosition.X -= MOVEMENT_SPEED * frameDeltaTime;
		else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_S)) {
			plateRotation.Z -= 0.3;
		}
		//nodePosition.X += MOVEMENT_SPEED * frameDeltaTime;
		plateModelSceneNode->setPosition(platePosition);
		plateModelSceneNode->setRotation(plateRotation);
		

		bool movedWithJoystick = false;
		driverFor2D->beginScene(true, true, video::SColor(200, 113, 113, 133));
		driverFor3D->beginScene(true, true, SColor(255, 100, 101, 140));

		driverFor2D->enableMaterial2D();
		driverFor2D->draw2DPolygon(mouseReceiver.GetMouseState().Position, 20, video::SColor(10, 10, 10, 10), 100000);
		driverFor2D->draw2DPolygon(mouseReceiver.GetMouseState().Position, 1, video::SColor(244, 0, 0, 0), 4);
		driverFor2D->draw2DRectangleOutline(core::recti(20, 20, ResX - 20, ResY - 20));
		driverFor2D->draw2DRectangleOutline(core::recti(40, 40, ResX - 40, ResY - 40), video::SColor(255, 255, 50, 80));
		driverFor2D->enableMaterial2D(false);

		smgrFor3D->drawAll();
		guienvFor3D->drawAll();
		smgrFor2D->drawAll(); // draw the 2d scene
		driverFor2D->endScene();
		driverFor3D->endScene();
	}
	deviceFor2D->drop();
	deviceFor3D->drop();

	return 0;
}