#include <irrlicht.h>
#include <rect.h>
#include <iostream>
#include "EventReceiver.cpp"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

int main()
{
	EventReceiver receiver;

	video::E_DRIVER_TYPE driverType = video::E_DRIVER_TYPE::EDT_OPENGL;
	IrrlichtDevice *device =
		createDevice(driverType, dimension2d<u32>(640, 480), 16,
			false, false, false, &receiver);

	if (!device)
		return 1;

	device->setWindowCaption(L"Hello World! - Irrlicht Engine Demo");

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();

	/*smgr->setambientlight(video::scolorf(0.0, 0.0, 0.0, 1));
	ILightSceneNode* light1 = smgr->addLightSceneNode(0, core::vector3df(0, 400, -200), video::SColorf(0.3f, 0.0f, 0.0f), 1.0f, 1);
*/
	/* My Codes */
	scene::ISceneNode* plateSceneNode = smgr->addCubeSceneNode();
	if (plateSceneNode) {
		plateSceneNode->setScale(vector3df(5, 0.1, 5));
	}

	IAnimatedMesh* ballMesh = smgr->getMesh("Ball/ball.obj");
	ISceneNode* ballSceneNode = 0;

	if (ballMesh)
	{
		//node2 = smgr->addOctreeSceneNode(mesh2->getMesh(0), 0, -1, 1024);
		ballSceneNode = smgr->addMeshSceneNode(ballMesh->getMesh(0));
		ballSceneNode->setMaterialFlag(EMF_LIGHTING, false);
		ballSceneNode->setPosition(core::vector3df(0, 10, 0));
		ballSceneNode->setScale(vector3df(5, 5, 5));
	}

	smgr->addCameraSceneNode(0, vector3df(0, 30, -40), vector3df(0, 5, 0));

	while (device->run())
	{
		core::vector3df platePosition = plateSceneNode->getPosition();
		core::vector3df plateRotation = plateSceneNode->getRotation();
		if (receiver.IsKeyDown(irr::KEY_KEY_W)) {
			plateRotation.X += 0.1;
		}
		//			nodePosition.Y += MOVEMENT_SPEED * frameDeltaTime;
		else if (receiver.IsKeyDown(irr::KEY_KEY_S)) {
			plateRotation.X -= 0.1;
		}
		//		nodePosition.Y -= MOVEMENT_SPEED * frameDeltaTime;
		else if (receiver.IsKeyDown(irr::KEY_KEY_A)) {
			plateRotation.Y += 0.1;
		}
		//	nodePosition.X -= MOVEMENT_SPEED * frameDeltaTime;
		else if (receiver.IsKeyDown(irr::KEY_KEY_D)) {
			plateRotation.Y -= 0.1;
		}
		//nodePosition.X += MOVEMENT_SPEED * frameDeltaTime;
		plateSceneNode->setPosition(platePosition);
		plateSceneNode->setRotation(plateRotation);
		driver->beginScene(true, true, SColor(255, 100, 101, 140));

		smgr->drawAll();
		guienv->drawAll();

		driver->endScene();
	}

	device->drop();

	return 0;
}

