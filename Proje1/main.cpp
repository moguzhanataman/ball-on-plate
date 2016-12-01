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
using namespace std;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif


IrrlichtDevice *device = 0;

void setActiveCamera(scene::ICameraSceneNode* newActive)
{
	if (0 == device)
		return;

	scene::ICameraSceneNode * active = device->getSceneManager()->getActiveCamera();
	active->setInputReceiverEnabled(false);

	newActive->setInputReceiverEnabled(true);
	device->getSceneManager()->setActiveCamera(newActive);
}

int main()
{
	EventReceiver receiver;

	video::E_DRIVER_TYPE driverType = video::E_DRIVER_TYPE::EDT_OPENGL;
	device =  createDevice(driverType, dimension2d<u32>(760, 600), 16, false, false, false, &receiver);

	if (!device)
		return 1;

	device->setWindowCaption(L"Ball on Plate - 3D");

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();

	/*smgr->setambientlight(video::scolorf(0.0, 0.0, 0.0, 1));
	ILightSceneNode* light1 = smgr->addLightSceneNode(0, core::vector3df(0, 400, -200), video::SColorf(0.3f, 0.0f, 0.0f), 1.0f, 1);
*/


	/* My Codes */
	// Create Platform
	scene::ISceneNode* plateModelSceneNode = smgr->addEmptySceneNode();

	// Create Plate
	IAnimatedMesh* plateMesh = smgr->getMesh("Plate/plate.obj");
	ISceneNode* plateSceneNode = 0;
	if (plateMesh)
	{
		//node2 = smgr->addOctreeSceneNode(mesh2->getMesh(0), 0, -1, 1024);
		plateSceneNode = smgr->addMeshSceneNode(plateMesh->getMesh(0));
		plateSceneNode->setMaterialFlag(EMF_LIGHTING, false);
		//plateSceneNode->setPosition(core::vector3df(0, 2, 0));
		plateSceneNode->setScale(vector3df(2, 2, 2));
	}

	// Add ball as child to plate
	plateModelSceneNode->addChild(plateSceneNode);

	// Create Ball
	IAnimatedMesh* ballMesh = smgr->getMesh("Ball/ball.obj");
	ISceneNode* ballSceneNode = 0;
	if (ballMesh)
	{
		//node2 = smgr->addOctreeSceneNode(mesh2->getMesh(0), 0, -1, 1024);
		ballSceneNode = smgr->addMeshSceneNode(ballMesh->getMesh(0));
		ballSceneNode->setMaterialFlag(EMF_LIGHTING, false);
		ballSceneNode->setPosition(core::vector3df(10, 2, 10));
		ballSceneNode->setScale(vector3df(2, 2, 2));
	}

	scene::ISceneNode* cylinderSceneNode = smgr->addCubeSceneNode();
	cylinderSceneNode->setPosition(core::vector3df(1, -11, 1));
	cylinderSceneNode->setScale(vector3df(0.3, 2, 0.3));
	cylinderSceneNode->setMaterialFlag(EMF_LIGHTING, false);
	//cylinderSceneNode->setMD2Animation(scene::EMAT_STAND);
	cylinderSceneNode->setMaterialTexture(0, driver->getTexture("Cylinder/Cylinder.jpg"));
	// Add ball as child to plate
	plateModelSceneNode->addChild(ballSceneNode);

	smgr->addCameraSceneNode(0, vector3df(0, 30, -40), vector3df(0, 5, 0));
	scene::ICameraSceneNode *camera = smgr->addCameraSceneNodeMaya(0, -100.0f, 100.0f, 100.0f);
	camera->setFarValue(20000.f);
	camera->setTarget(core::vector3df(0,30,0));
	setActiveCamera(camera);

	while (device->run())
	{
		core::vector3df platePosition = plateModelSceneNode->getPosition();
		core::vector3df plateRotation = plateModelSceneNode->getRotation();
		if (receiver.IsKeyDown(irr::KEY_KEY_A)) {
			plateRotation.X += 0.01;
		}
		//			nodePosition.Y += MOVEMENT_SPEED * frameDeltaTime;
		else if (receiver.IsKeyDown(irr::KEY_KEY_D)) {
			plateRotation.X -= 0.01;
		}
		//		nodePosition.Y -= MOVEMENT_SPEED * frameDeltaTime;
		else if (receiver.IsKeyDown(irr::KEY_KEY_W)) {
			plateRotation.Z += 0.01;
		}
		//	nodePosition.X -= MOVEMENT_SPEED * frameDeltaTime;
		else if (receiver.IsKeyDown(irr::KEY_KEY_S)) {
			plateRotation.Z -= 0.01;
		}
		//nodePosition.X += MOVEMENT_SPEED * frameDeltaTime;
		plateModelSceneNode->setPosition(platePosition);
		plateModelSceneNode->setRotation(plateRotation);
		driver->beginScene(true, true, SColor(255, 100, 101, 140));

		smgr->drawAll();
		guienv->drawAll();

		driver->endScene();
	}

	device->drop();

	return 0;
}

