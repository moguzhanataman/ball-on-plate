#ifdef _MSC_VER
// We'll define this to stop MSVC complaining about sprintf().
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Irrlicht.lib")
#endif

#include <iostream>
#include <irrlicht.h>
#include <rect.h>
#include "src/EventReceivers/Mouse2D.cpp"
#include "src/EventReceivers/Mouse3D.cpp"
#include "serial/serial.h"
#include <signal.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace std;
using namespace irr;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif


/* ======= Global Variables ======= */
const int ResX = 640;
const int ResY = 480;
IrrlichtDevice *deviceFor3D = 0;

/* ======= Prototypes ======= */
void setActiveCamera(scene::ICameraSceneNode*);
void printRuler(video::IVideoDriver*);
void signal_handler(int sig) {
    close_serial();
    exit(0);
}
/* ======= Main ======= */
int main() {
    signal(SIGINT, signal_handler);

    if (!init_serial()) {
        std::cerr << "Serial port initialization error\n";
        exit(1);
    }
    
	MouseEventReceiverFor2D mouseReceiver;
	MouseEventReceiverFor3D receiverForPlate;

	deviceFor3D = createDevice(irr::video::EDT_OPENGL, dimension2d<u32>(ResX, ResY), 16, false, true, false, &receiverForPlate);
	IrrlichtDevice* deviceFor2D = createDevice(video::E_DRIVER_TYPE::EDT_BURNINGSVIDEO,
		core::dimension2d<u32>(1.5*ResX, 1.5*ResY), 16, false, false, false, &mouseReceiver);


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



	IGUIButton* buttonCircle = guienvFor2D->addButton(rect<s32>(20, ResY + 30, 120, ResY + 60 + 32), 0, -1,
		L"", L"DRAW CIRCLE");
	buttonCircle->setImage(driverFor2D->getTexture("assets/circle.jpg"));

	IGUIButton* buttonRectangle = guienvFor2D->addButton(rect<s32>(140, ResY + 30, 240, ResY + 60 + 32), 0, -1,
		L"", L"DRAW RECTANGLE");

	buttonRectangle->setImage(driverFor2D->getTexture("assets/rectangle.jpg"));

	IGUIButton* buttonTriangle = guienvFor2D->addButton(rect<s32>(260, ResY + 30, 360, ResY + 60 + 32), 0, -1,
		L"", L"DRAW TRIANGLE");

	buttonTriangle->setImage(driverFor2D->getTexture("assets/triangle.jpg"));

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


	IGUIButton* changeButton = guienvFor2D->addButton(core::rect<s32>(ResX + 140, 140, ResX + 200, 160), 0, -1, L"Change");

	changeButton->setScaleImage(false);

	IGUIStaticText* pxText = guienvFor2D->addStaticText(L"px: ", core::rect<s32>(ResX + 20, 35, ResX + 40, 50));
	IGUIEditBox* px = guienvFor2D->addEditBox(L"px", core::rect<s32>(ResX + 40, 20, ResX + 100, 60));

	IGUIStaticText* dxText = guienvFor2D->addStaticText(L"dx: ", core::rect<s32>(ResX + 120, 35, ResX + 140, 50));
	IGUIEditBox* dx = guienvFor2D->addEditBox(L"dx", core::rect<s32>(ResX + 140, 20, ResX + 200, 60));

	IGUIStaticText* ixText = guienvFor2D->addStaticText(L"ix: ", core::rect<s32>(ResX + 220, 35, ResX + 240, 50));
	IGUIEditBox* ix = guienvFor2D->addEditBox(L"ix", core::rect<s32>(ResX + 240, 20, ResX + 300, 60));

	IGUIStaticText* pyText = guienvFor2D->addStaticText(L"py: ", core::rect<s32>(ResX + 20, 95, ResX + 40, 120));
	IGUIEditBox* py = guienvFor2D->addEditBox(L"py", core::rect<s32>(ResX + 40, 80, ResX + 100, 120));

	IGUIStaticText* dyText = guienvFor2D->addStaticText(L"dy: ", core::rect<s32>(ResX + 120, 95, ResX + 140, 110));
	IGUIEditBox* dy = guienvFor2D->addEditBox(L"dy", core::rect<s32>(ResX + 140, 80, ResX + 200, 120));

	IGUIStaticText* iyText = guienvFor2D->addStaticText(L"iy: ", core::rect<s32>(ResX + 220, 95, ResX + 240, 110));
	IGUIEditBox* iy = guienvFor2D->addEditBox(L"iy", core::rect<s32>(ResX + 240, 80, ResX + 300, 120));

	// 3D Part
	// Create Platform
	scene::ISceneNode* plateModelSceneNode = smgrFor3D->addEmptySceneNode();

	// Create Plate
	IAnimatedMesh* plateMesh = smgrFor3D->getMesh("assets/RealPlate/plate3.obj");
	IMeshSceneNode* plateSceneNode = 0;
	if (plateMesh)
	{
		//node2 = smgr->addOctreeSceneNode(mesh2->getMesh(0), 0, -1, 1024);
		plateSceneNode = smgrFor3D->addMeshSceneNode(plateMesh->getMesh(0));
		plateSceneNode->setMaterialFlag(EMF_LIGHTING, true);
		plateSceneNode->setPosition(core::vector3df(0, -0.8, 0));
		plateSceneNode->setScale(vector3df(2, 17, 3));

	    plateSceneNode->addShadowVolumeSceneNode();
	}

	// Add ball as child to plate
	plateModelSceneNode->addChild(plateSceneNode);

	// Create Ball
	IAnimatedMesh* ballMesh = smgrFor3D->getMesh("assets/Ball/ball2.obj");
	IMeshSceneNode* ballSceneNode = 0;
	if (ballMesh)
	{
		//node2 = smgr->addOctreeSceneNode(mesh2->getMesh(0), 0, -1, 1024);
		ballSceneNode = smgrFor3D->addMeshSceneNode(ballMesh->getMesh(0));
		ballSceneNode->setMaterialFlag(EMF_LIGHTING, true);
		ballSceneNode->setPosition(core::vector3df(0, 2, 0));
		ballSceneNode->setScale(vector3df(2, 2, 2));


	    ballSceneNode->addShadowVolumeSceneNode();

	}

	smgrFor3D->setShadowColor(video::SColor(150,0,0,0));

	//light
	smgrFor3D->addLightSceneNode(0, core::vector3df(20,20,20), video::SColorf(1.0f, 1.0f, 1.0f, 1.0f), 1800.0f);


	scene::ISceneNode* cylinderSceneNode = smgrFor3D->addCubeSceneNode();
	cylinderSceneNode->setPosition(core::vector3df(1, -11, 0.2));
	cylinderSceneNode->setScale(vector3df(0.3, 2, 0.3));
	cylinderSceneNode->setMaterialFlag(EMF_LIGHTING, true);
	//cylinderSceneNode->setMD2Animation(scene::EMAT_STAND);
	cylinderSceneNode->setMaterialTexture(0, driverFor3D->getTexture("assets/Cylinder/Cylinder.jpg"));
	// Add ball as child to plate
	plateModelSceneNode->addChild(ballSceneNode);


	// Alat plaka
	scene::ISceneNode* lowerPlate = smgrFor3D->addCubeSceneNode();
	if (plateSceneNode) {
		lowerPlate->setScale(vector3df(4, 0.1, 4));
	}
	lowerPlate->setPosition(core::vector3df(1, -22, 0.2));


	smgrFor3D->addCameraSceneNode(0, vector3df(0, 30, -40), vector3df(0, 5, 0));
	scene::ICameraSceneNode *camera = smgrFor3D->addCameraSceneNodeMaya(0, -100.0f, 100.0f, 100.0f);
	camera->setFarValue(20000.f);
	camera->setTarget(core::vector3df(10, 15, 0));
	setActiveCamera(camera);

	float arr[6] = {};

	camera->setFarValue(10000.0f);

	
	IAnimatedMesh* roommesh = smgrFor3D->getMesh("assets/kappa.obj");
	ISceneNode* roomNode = 0;
	if (roommesh)
	{
		
		roomNode = smgrFor3D->addMeshSceneNode(roommesh->getMesh(0));
		roomNode->setMaterialFlag(EMF_LIGHTING, false);
		roomNode->setPosition(core::vector3df(-30, -30, -30));
		roomNode->setScale(vector3df(1, 1, 1));
	}

	int16_t x, y;
    float servo_x, servo_y;
    bool coord;
	wchar_t buffer[50] = L"";
	while (deviceFor2D->run() && deviceFor3D->run())
	{
		core::vector3df ballPosition = ballSceneNode->getPosition();

        coord = getCoordinates(&x, &y, &servo_x, &servo_y);
        if (coord) {
            ballPosition.X = 0.001 * x;
            ballPosition.Y = 0.001 * y;
            plateModelSceneNode->setPosition(ballPosition);
        }
		ballSceneNode->setPosition(core::vector3df(-15.0 + (double)mouseReceiver.GetMouseState().Position.Y * (30.0 / ResY), 2.0,
			-23.0 + (double)mouseReceiver.GetMouseState().Position.X * (46.0 / ResX)));

		if (receiverForPlate.IsKeyDown(irr::KEY_KEY_A)) {
			plateRotation.X += 0.3;
		}
		else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_D)) {
			plateRotation.X -= 0.3;
		}
		else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_W)) {
			plateRotation.Z += 0.3;
		}
		else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_S)) {
			plateRotation.Z -= 0.3;
		}
		plateModelSceneNode->setPosition(platePosition);
		plateModelSceneNode->setRotation(plateRotation);

		bool movedWithJoystick = false;
		driverFor2D->beginScene(true, true, video::SColor(200, 41, 57, 85));
		driverFor3D->beginScene(true, true, SColor(255, 41, 57, 85));

		printRuler(driverFor2D);

	

	

		driverFor2D->enableMaterial2D();
		driverFor2D->draw2DPolygon(mouseReceiver.GetMouseState().Position, 20, video::SColor(10, 74, 162, 226), 100000);
		driverFor2D->draw2DPolygon(mouseReceiver.GetMouseState().Position, 1, video::SColor(244, 74, 162, 226), 4);
		driverFor2D->draw2DRectangleOutline(core::recti(20, 20, ResX - 20, ResY - 20));
		driverFor2D->draw2DRectangleOutline(core::recti(40, 40, ResX - 40, ResY - 40), video::SColor(255, 255, 50, 80));
		driverFor2D->enableMaterial2D(false);

		if (changeButton->isPressed()) {
			// Some magic here xD
			arr[0] = std::wcstof(px->getText(), NULL);
			arr[1] = std::wcstof(ix->getText(), NULL);
			arr[2] = std::wcstof(dx->getText(), NULL);
			arr[3] = std::wcstof(py->getText(), NULL);
			arr[4] = std::wcstof(iy->getText(), NULL);
			arr[5] = std::wcstof(dy->getText(), NULL);

			//sendPID(arr);


			changeButton->setPressed(false);
		}

		smgrFor3D->drawAll();
		guienvFor3D->drawAll();
		guienvFor2D->drawAll();
		smgrFor2D->drawAll(); // draw the 2d scene
		driverFor2D->endScene();
		driverFor3D->endScene();
	}
	deviceFor2D->drop();
	deviceFor3D->drop();

	return 0;
}

void setActiveCamera(scene::ICameraSceneNode* newActive) {
	if (0 == deviceFor3D)
		return;

	scene::ICameraSceneNode * active = deviceFor3D->getSceneManager()->getActiveCamera();
	active->setInputReceiverEnabled(false);

	newActive->setInputReceiverEnabled(true);
	deviceFor3D->getSceneManager()->setActiveCamera(newActive);
}

/**********************************************************
 * printRuler
 *
 * params:
 * 	- driverFor2D: Driver for 2D interface
 **********************************************************/
void printRuler(video::IVideoDriver* driverFor2D) {
	driverFor2D->draw2DLine(position2di(20, ResY - 10), position2di(20, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(40, ResY - 10), position2di(40, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(60, ResY - 10), position2di(60, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(80, ResY - 10), position2di(80, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(100, ResY - 10), position2di(100, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(120, ResY - 10), position2di(120, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(140, ResY - 10), position2di(140, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(160, ResY - 10), position2di(160, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(180, ResY - 10), position2di(180, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(200, ResY - 10), position2di(200, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(220, ResY - 10), position2di(220, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(240, ResY - 10), position2di(240, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(260, ResY - 10), position2di(260, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(280, ResY - 10), position2di(280, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(300, ResY - 10), position2di(300, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(320, ResY - 10), position2di(320, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(340, ResY - 10), position2di(340, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(360, ResY - 10), position2di(360, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(380, ResY - 10), position2di(380, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(400, ResY - 10), position2di(400, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(420, ResY - 10), position2di(420, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(440, ResY - 10), position2di(440, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(460, ResY - 10), position2di(460, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(480, ResY - 10), position2di(480, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(500, ResY - 10), position2di(500, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(520, ResY - 10), position2di(520, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(540, ResY - 10), position2di(540, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(560, ResY - 10), position2di(560, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(580, ResY - 10), position2di(580, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(600, ResY - 10), position2di(600, ResY), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(620, ResY - 10), position2di(620, ResY + 10), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 20), position2di(ResX + 10, 20), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 40), position2di(ResX, 40), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 60), position2di(ResX + 10, 60), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 80), position2di(ResX, 80), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 100), position2di(ResX + 10, 100), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 120), position2di(ResX, 120), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 140), position2di(ResX + 10, 140), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 160), position2di(ResX, 160), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 180), position2di(ResX + 10, 180), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 200), position2di(ResX, 200), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 220), position2di(ResX + 10, 220), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 240), position2di(ResX, 240), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 260), position2di(ResX + 10, 260), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 280), position2di(ResX, 280), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 300), position2di(ResX + 10, 300), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 320), position2di(ResX, 320), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 340), position2di(ResX + 10, 340), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 360), position2di(ResX, 360), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 380), position2di(ResX + 10, 380), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 400), position2di(ResX, 400), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 420), position2di(ResX + 10, 420), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 440), position2di(ResX, 440), SColor(255, 0, 0, 0));
	driverFor2D->draw2DLine(position2di(ResX - 10, 460), position2di(ResX + 10, 460), SColor(255, 0, 0, 0));
}
