#ifdef _MSC_VER
// We'll define this to stop MSVC complaining about sprintf().
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Irrlicht.lib")
#endif

//#define SERIAL_ON

#include <iostream>
#include <irrlicht.h>
#include <rect.h>
#include "src/EventReceivers/Mouse2D.cpp"
#include "src/EventReceivers/Mouse3D.cpp"
#include "serial/serial.h"
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <fstream>
#include <wchar.h>
#include <ctime>
#include <math.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace irr;
using namespace std;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif


/* ======= Global Variables ======= */
const int ResX = 640;
const int ResY = 480;
IrrlichtDevice *deviceFor3D = 0;
f32 cameraDistance = 30000.f;
ISceneManager* smgrFor3D;
scene::ISceneNode* plateModelSceneNode;
IVideoDriver* driverFor3D;


/* ======= Prototypes ======= */
void setActiveCamera(scene::ICameraSceneNode*);
void printRuler(video::IVideoDriver* driverFor2D, IGUIEnvironment* guienvFor2D);
void signal_handler(int sig) {
	close_serial();
	exit(0);
}
void printGraphic(video::IVideoDriver* driverFor2D, gui::IGUIEnvironment* guienvFor2D, double *graphicX, double *graphicY, int size);
void startMemoryGame() {
	char c = '1';
	sendBuf(&c, 1);
}

void startPIDMode() {
	char c = '0';
	sendBuf(&c, 1);
}

class Led
{
public:

	void openLed()
	{
		bill = smgrFor3D->addBillboardSceneNode();
		plateModelSceneNode->addChild(bill);
		bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
		bill->setMaterialTexture(0, driverFor3D->getTexture("assets/RealPlate/particlered.bmp"));
		bill->setSize(core::dimension2d<f32>(3.0f, 3.0f));
		bill->setPosition(core::vector3df(positionX, 0, positionY));

	}
	void closeLed()
	{
		bill->setSize(core::dimension2d<f32>(3.0f, 3.0f));
		bill->setPosition(core::vector3df(positionX, -1, positionY));
	}
	friend void fillLedArray(Led ledArray[6][8])
	{
		int ledPositionX = -10;
		int ledPositionY = -15;
		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				ledArray[i][j].setpositionX(ledPositionX);
				ledArray[i][j].setpositionY(ledPositionY);
				ledPositionY += 4.5;
			}
			ledPositionX += 4;
			ledPositionY = -15;
		}
	}
	void setpositionX(int pos) { positionX = pos; }
	void setpositionY(int pos) { positionY = pos; }
	int getpositionX() { return positionX; }
	int getpositionY() { return positionY; }
private:
	scene::IBillboardSceneNode * bill;
	int positionX;
	int positionY;
};

double mapping(double x, double in_min, double in_max, double out_min, double out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double calculateRotation( double servo_angle, double plateScale ) {

	double result;

	result = (180.0/PI) * asin( ( 1.5 * sin((PI / 180.0) *(servo_angle-90.0)) +
			sqrt( 156.25 - ( 2.25 + 2.25 * pow( cos((PI / 180.0)  * (servo_angle-90)), 2 ) -
				4.5 * cos((PI / 180.0) * (servo_angle - 90))  )  ) - 12.5) /plateScale );

	return result;
}

/* ======= Main ======= */
int main() {
	std::clock_t start;
	double duration;

#ifdef _WIN32
	
	if (AllocConsole() == 0)

		exit(EXIT_FAILURE);

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);

#endif
	
#ifdef SERIAL_ON
	signal(SIGINT, signal_handler);

	if (!init_serial()) {
		std::cerr << "Serial port initialization error\n";
		exit(1);
	}
#endif
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


	/*
	IGUIButton* buttonCircle = guienvFor2D->addButton(rect<s32>(20, ResY + 30, 120, ResY + 60 + 32), 0, -1,
		L"", L"DRAW CIRCLE");
	buttonCircle->setImage(driverFor2D->getTexture("assets/circle.jpg"));

	IGUIButton* buttonRectangle = guienvFor2D->addButton(rect<s32>(140, ResY + 30, 240, ResY + 60 + 32), 0, -1,
		L"", L"DRAW RECTANGLE");

	buttonRectangle->setImage(driverFor2D->getTexture("assets/rectangle.jpg"));

	IGUIButton* buttonTriangle = guienvFor2D->addButton(rect<s32>(260, ResY + 30, 360, ResY + 60 + 32), 0, -1,
		L"", L"DRAW TRIANGLE");

	buttonTriangle->setImage(driverFor2D->getTexture("assets/triangle.jpg"));*/

	driverFor3D = deviceFor3D->getVideoDriver();
	smgrFor3D = deviceFor3D->getSceneManager();
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

	// 2D Memory Game
	int base_y = 600;
	IGUIStaticText* memoryGameText = guienvFor2D->addStaticText(L"MEMORY GAME", core::rect<s32>(50, base_y - 40, 200, base_y - 20));

	IGUIStaticText* trueText = guienvFor2D->addStaticText(L"True: ", core::rect<s32>(50, base_y, 100, base_y + 20));
	IGUIStaticText* trueNumberText = guienvFor2D->addStaticText(L"", core::rect<s32>(100, base_y, 150, base_y + 20));
	int trueNumber = 0;

	IGUIStaticText* falseText = guienvFor2D->addStaticText(L"False: ", core::rect<s32>(50, base_y + 20, 100, base_y + 40));
	IGUIStaticText* falseNumberText = guienvFor2D->addStaticText(L"", core::rect<s32>(100, base_y + 20, 150, base_y + 40));
	int falseNumber = 0;

	IGUIButton* startMemoryGameButton = 
		guienvFor2D->addButton(core::rect<s32>(50, base_y + 40, 150, base_y + 60), 0, -1, L"Start Memory Game");

	IGUIButton* endMemoryGameButton = 
		guienvFor2D->addButton(core::rect<s32>(50, base_y + 60, 150, base_y + 80), 0, -1, L"End Memory Game");

	bool isGameStarted = false;
	IGUIStaticText* gameStatusText = 
		guienvFor2D->addStaticText(L"", core::rect<s32>(50, base_y + 80, 250, base_y + 100));

	// 2D Vision App
	int base_vision_x = 200;
	int base_vision_y = 640;
	
	IGUIButton* startVisionButton = 
			guienvFor2D->addButton(core::rect<s32>(base_vision_x, base_vision_y, base_vision_x + 100, base_vision_y + 20), 0, -1, L"Start Vision App");
	IGUIButton* endVisionButton = 
			guienvFor2D->addButton(core::rect<s32>(base_vision_x, base_vision_y + 20, base_vision_x + 100, base_vision_y + 40), 0, -1, L"End Vision App");


	// 3D Part
	// Create Platform
	plateModelSceneNode = smgrFor3D->addEmptySceneNode();

	// Create Plate
	IAnimatedMesh* plateMesh = smgrFor3D->getMesh("assets/RealPlate/plate3.obj");
	IMeshSceneNode* plateSceneNode = 0;
	if (plateMesh)
	{
	
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
	cylinderSceneNode->setMaterialTexture(0, driverFor3D->getTexture("assets/Cylinder/Cylinder.jpg"));
	// Add ball as child to plate
	plateModelSceneNode->addChild(ballSceneNode);



	// servo first
	scene::ISceneNode* firstServo = smgrFor3D->addCubeSceneNode();
	if (firstServo) {
		firstServo->setScale(vector3df(0.5, 0.5, 0.5));
		firstServo->setMaterialTexture(0, driverFor3D->getTexture("assets/Cylinder/Cylinder.jpg"));
	}
	firstServo->setPosition(core::vector3df(12, -20, 0));


	// second first
	scene::ISceneNode* secondServo = smgrFor3D->addCubeSceneNode();
	if (secondServo) {
		secondServo->setScale(vector3df(0.5, 0.5, 0.5));
		secondServo->setMaterialTexture(0, driverFor3D->getTexture("assets/Cylinder/Cylinder.jpg"));
	}
	secondServo->setPosition(core::vector3df(0, -20, -20));



	// Alt plaka
	scene::ISceneNode* lowerPlate = smgrFor3D->addCubeSceneNode();
	if (lowerPlate) {
		lowerPlate->setScale(vector3df(3.5, 0.1, 4.7));
	}
	lowerPlate->setPosition(core::vector3df(-2, -22, 0.2));
	

	smgrFor3D->addCameraSceneNode(0, vector3df(0, 30, -40), vector3df(0, 5, 0));
	scene::ICameraSceneNode *camera = smgrFor3D->addCameraSceneNodeMaya(0, -100.0f, 100.0f, 100.0f);
	camera->setFarValue(cameraDistance);
	camera->setTarget(core::vector3df(10, 15, 0));
	setActiveCamera(camera);



	double firstServoLength = 2.1;
	// first servo arm
	scene::ISceneNode* firstArm = smgrFor3D->addCubeSceneNode();
	firstArm->setPosition(core::vector3df(14, -12, 0));
	firstArm->setScale(vector3df(0.05, firstServoLength, 0.05));
	firstArm->setMaterialFlag(EMF_LIGHTING, true);
	

	double secondServoLength = 2.1;
	// second servo arm
	scene::ISceneNode* secondArm = smgrFor3D->addCubeSceneNode();
	secondArm->setPosition(core::vector3df(0, -12, -22));
	secondArm->setScale(vector3df(0.05, secondServoLength, 0.05));
	secondArm->setMaterialFlag(EMF_LIGHTING, true);

	

	// Create table
	IAnimatedMesh* tableMesh = smgrFor3D->getMesh("assets/table/table.3ds");
	IMeshSceneNode* tableSceneNode = 0;
	if (tableMesh)
	{
	
		tableSceneNode = smgrFor3D->addMeshSceneNode(tableMesh->getMesh(0));
		tableSceneNode->setMaterialFlag(EMF_LIGHTING, false);
		tableSceneNode->setMaterialFlag(E_MATERIAL_FLAG::EMF_COLOR_MATERIAL, false);
		tableSceneNode->setPosition(core::vector3df(1, -28, 0.2));
		tableSceneNode->setScale(vector3df(0.2, 0.3, 0.3));
	}

	
	IAnimatedMesh* roommesh = smgrFor3D->getMesh("assets/kappa.obj");
	ISceneNode* roomNode = 0;
	if (roommesh)
	{
		
		roomNode = smgrFor3D->addMeshSceneNode(roommesh->getMesh(0));
		roomNode->setMaterialFlag(EMF_LIGHTING, false);
		roomNode->setPosition(core::vector3df(-10, 40, -10));
		roomNode->setScale(vector3df(6, 3.5, 5));
	}
	

	int gameMode = 0;
	int gamePoints = 0;
	char ledSerial[6][8];
	Led led3D[6][8];

	//cerceve ustu
	fillLedArray(led3D);

	// for (int i = 0; i < 6; i++)
	// {
	// 	for (int j = 0; j < 8; j++)
	// 	{c
	// 		led3D[i][j].openLed();
	// 	}
	// }

	for (int i = 0; i < 6; ++i) {
		for (int j = 0; j < 8; ++j) {
			ledSerial[i][j] = 'f';
		}	
	}

	double graphicX[41] = { 0 }, graphicY[41] = {0};
	double lastX = 0, lastY=0;
	int size = 1;		


	core::position2df currentPosition = core::position2df(0,0);
	int16_t x=450, y=420;
	float servo_x=90, servo_y=90;
	wchar_t buffer[50] = L"";
	wchar_t wstrBuffer[128] = L"";
	core::position2di lastPos = mouseReceiver.GetMouseState().Position;
	double buffPosX=0, buffPosY=0;

	//////////////////////////////////////////////////////////////////////////////////////////////
	//									LOOP
	//////////////////////////////////////////////////////////////////////////////////////////////
	while (deviceFor2D->run() && deviceFor3D->run())
	{
		start = std::clock();

		// TODO: degistir
		trueNumberText->setText(L"TRUE...");

		
		
		core::vector3df platePosition = plateModelSceneNode->getPosition();
		core::vector3df plateRotation = plateModelSceneNode->getRotation();

		#ifdef SERIAL_ON
		if (getCoordinates(&x, &y, &servo_x, &servo_y)) {
			if (gameMode == 1) {
				readLeds((char**)ledSerial);

				for (int i = 0; i < 6; ++i) {
					for (int j = 0; j < 8; ++j) {
						if (ledSerial[i][j] == 't') {
							led3D[i][j].openLed();
						} else {
							led3D[i][j].closeLed();
						}
					}
				}

				if ( !(x == 0 && y == 1023) ) {
					if (ledSerial[0][0] == 'f' && ledSerial[1][1] == 't') {
						gameStatusText->setText(L"Try Again");
					}

					if (ledSerial[4][6] == 't') {
						gamePoints++;
						swprintf(wstrBuffer, 128, L"+1 Point! Your current score: %d", gamePoints);
						gameStatusText->setText(wstrBuffer);
					}
				}

				if (gamePoints == 5) {
					gameStatusText->setText(L"Congratulations, You won!");
				}
			}

			double posX, posY;
			posX = mapping(x, 160, 910, 40, ResX - 40);
			posY = mapping(y, 190, 880, 40, ResY - 40);
			currentPosition = core::position2df(posX, posY);
			core::vector3df ballPosition = core::vector3df(currentPosition.X, 2.0 ,currentPosition.Y);

		}
		#endif

		cout << x << " " << y << " " << servo_x << " " << servo_y << endl;
		plateRotation.Z = (double)mapping(servo_x, 90, 180, 0, 6.5);
		
		if (lastPos != mouseReceiver.GetMouseState().Position) {
			lastPos = mouseReceiver.GetMouseState().Position;
			buffPosX = mapping(lastPos.X, 40, ResX - 40, 200, 910);
			buffPosY = mapping(lastPos.Y, 40, ResY - 40, 190, 880);
			sendSetpoints(buffPosX, buffPosY);
		}

		plateRotation.Z = -1 * calculateRotation(servo_y, 12);
		firstServoLength = (double)mapping(servo_y, 90, 180, 2.1, 2.388);

		cout << "X -> " << x << " and Y -> " << y << endl;

		if (startMemoryGameButton->isPressed()) {
			startMemoryGameButton->setPressed(false);
			
			sendBuf("1", 1);

			gameMode = 1;
			trueNumber = 0;
			falseNumber = 0;
			gameStatusText->setText(L"Game started");
			// trueNumberText->setText(L"0");
			// falseNumberText->setText(L"0");
		}

		if (endMemoryGameButton->isPressed()) {
			endMemoryGameButton->setPressed(false);
			char c = '0';
			sendBuf(&c, 1);
			swprintf(wstrBuffer, 128, L"Game ended you completed with %d true paths and %d false paths", trueNumber, falseNumber);
			gameStatusText->setText(wstrBuffer);
			gameMode = 0;
			trueNumber = 0;
			falseNumber = 0;
		}

		if (startVisionButton->isPressed()) {
			startVisionButton->setPressed(false);
			gameStatusText->setText(L"Vision app started");
			// trueNumberText->setText(L"0");
			// falseNumberText->setText(L"0");
		}

		if (endVisionButton->isPressed()) {
			endVisionButton->setPressed(false);
			gameStatusText->setText(L"Vision app ended");
		}


		plateRotation.X = calculateRotation(servo_x, 16);
		secondServoLength = (double)mapping(servo_x, 90, 180, 2.1, 2.604);

		firstArm->setScale(vector3df(0.05, firstServoLength, 0.05));
		secondArm->setScale(vector3df(0.05, secondServoLength, 0.05));

		
		ballSceneNode->setPosition(core::vector3df(-15.0 + currentPosition.Y * (30.0 / ResY),
							2.0, -23.0 + currentPosition.X * (46.0 / ResX)));
		plateModelSceneNode->setRotation(plateRotation);

		bool movedWithJoystick = false;
		driverFor2D->beginScene(true, true, video::SColor(200, 41, 57, 85));
		driverFor3D->beginScene(true, true, SColor(255, 41, 57, 85));

		printRuler(driverFor2D, guienvFor2D);



		driverFor2D->enableMaterial2D();
		driverFor2D->draw2DPolygon(mouseReceiver.GetMouseState().Position, 20, video::SColor(10, 74, 162, 226), 100000);
		driverFor2D->draw2DPolygon(mouseReceiver.GetMouseState().Position, 1, video::SColor(244, 74, 162, 226), 4);
		driverFor2D->draw2DRectangleOutline(core::recti(20, 20, ResX - 20, ResY - 20));
		driverFor2D->draw2DRectangleOutline(core::recti(40, 40, ResX - 40, ResY - 40), video::SColor(255, 255, 50, 80));
		driverFor2D->enableMaterial2D(false);



		

		if (size == 41) {
			for (int k = 0; k < 40; k++) {
				graphicX[k] = graphicX[k + 1];
			}
		
			for (int k = 0; k < 40; k++) {
				graphicY[k] = graphicY[k + 1];
			}
			size = 40;
		}


		graphicX[size] = mapping(x-lastX, 600, -600, 45, 165);
		graphicY[size] = mapping(y-lastY, 600, -600, 265, 385);

		lastX = x;
		lastY = y;

		printGraphic(driverFor2D, guienvFor2D, graphicX, graphicY, size);
		cout << "error -> " << lastX <<  " " << graphicX[size] << " " << 
				lastY << " " << graphicY[size] << endl;
		size++;
		
		
		smgrFor3D->drawAll();
		guienvFor3D->drawAll();
		guienvFor2D->drawAll();
		smgrFor2D->drawAll(); // draw the 2d scene
		driverFor2D->endScene();
		driverFor3D->endScene();
		duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;


		

	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//									END OF LOOP
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	
	deviceFor2D->drop();
	deviceFor3D->drop();
	close_serial();
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
void printRuler(video::IVideoDriver* driverFor2D, IGUIEnvironment* guienvFor2D) {
	SColor rulerColor = SColor(255, 255, 255, 255);
	driverFor2D->draw2DLine(position2di(20, ResY - 10), position2di(20, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(29.68, ResY - 10), position2di(29.68, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(39.35, ResY - 10), position2di(39.35, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(49.03, ResY - 10), position2di(49.03, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(58.71, ResY - 10), position2di(58.71, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(68.39, ResY - 10), position2di(68.39, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(78.07, ResY - 10), position2di(78.07, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(87.75, ResY - 10), position2di(87.75, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(97.43, ResY - 10), position2di(97.43, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(107.11, ResY - 10), position2di(107.11, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(116.79, ResY - 10), position2di(116.79, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(126.47, ResY - 10), position2di(126.47, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(136.15, ResY - 10), position2di(136.15, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(145.83, ResY - 10), position2di(145.83, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(155.51, ResY - 10), position2di(155.51, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(165.19, ResY - 10), position2di(165.19, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(174.87, ResY - 10), position2di(174.87, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(184.55, ResY - 10), position2di(184.55, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(194.23, ResY - 10), position2di(194.23, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(203.91, ResY - 10), position2di(203.91, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(213.59, ResY - 10), position2di(213.59, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(223.27, ResY - 10), position2di(223.27, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(232.95, ResY - 10), position2di(232.95, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(242.63, ResY - 10), position2di(242.63, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(252.31, ResY - 10), position2di(252.31, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(261.99, ResY - 10), position2di(261.99, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(271.67, ResY - 10), position2di(271.67, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(281.35, ResY - 10), position2di(281.35, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(291.03, ResY - 10), position2di(291.03, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(300.68, ResY - 10), position2di(300.68, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(310.36, ResY - 10), position2di(310.36, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(320.04, ResY - 10), position2di(320.04, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(329.72, ResY - 10), position2di(329.72, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(339.4, ResY - 10), position2di(339.4, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(349.08, ResY - 10), position2di(349.08, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(358.76, ResY - 10), position2di(358.76, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(368.44, ResY - 10), position2di(368.44, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(378.12, ResY - 10), position2di(378.12, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(387.8, ResY - 10), position2di(387.8, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(397.48, ResY - 10), position2di(397.48, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(407.16, ResY - 10), position2di(407.16, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(416.84, ResY - 10), position2di(416.84, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(426.52, ResY - 10), position2di(426.52, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(436.2, ResY - 10), position2di(436.2, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(445.88, ResY - 10), position2di(445.88, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(455.56, ResY - 10), position2di(455.56, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(465.24, ResY - 10), position2di(465.24, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(474.92, ResY - 10), position2di(474.92, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(484.6, ResY - 10), position2di(484.6, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(494.28, ResY - 10), position2di(494.28, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(503.96, ResY - 10), position2di(503.96, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(513.64, ResY - 10), position2di(513.64, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(523.32, ResY - 10), position2di(523.32, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(533, ResY - 10), position2di(533, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(542.68, ResY - 10), position2di(542.68, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(552.36, ResY - 10), position2di(552.36, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(562.04, ResY - 10), position2di(562.04, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(571.72, ResY - 10), position2di(571.72, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(581.4, ResY - 10), position2di(581.4, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(591.08, ResY - 10), position2di(591.08, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(600.76, ResY - 10), position2di(600.76, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(610.44, ResY - 10), position2di(610.44, ResY), rulerColor);
	driverFor2D->draw2DLine(position2di(620, ResY - 10), position2di(620, ResY + 10), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 20), position2di(ResX + 10, 20), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 29.17), position2di(ResX, 29.17), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 38.34), position2di(ResX + 10, 38.34), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 47.51), position2di(ResX, 47.51), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 56.68), position2di(ResX + 10, 56.68), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 65.85), position2di(ResX, 65.85), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 75.02), position2di(ResX + 10, 75.02), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 84.19), position2di(ResX, 84.19), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 93.36), position2di(ResX + 10, 93.36), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 102.53), position2di(ResX, 102.53), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 111.7), position2di(ResX + 10, 111.7), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 120.87), position2di(ResX, 120.87), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 130.04), position2di(ResX + 10, 130.04), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 139.21), position2di(ResX, 139.21), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 148.38), position2di(ResX + 10, 148.38), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 157.55), position2di(ResX, 157.55), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 166.72), position2di(ResX + 10, 166.72), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 175.89), position2di(ResX, 175.89), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 185.06), position2di(ResX + 10, 185.06), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 194.23), position2di(ResX, 194.23), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 203.4), position2di(ResX + 10, 203.4), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 212.57), position2di(ResX, 212.57), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 221.74), position2di(ResX + 10, 221.74), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 230.91), position2di(ResX, 230.91), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 240.08), position2di(ResX + 10, 240.08), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 249.25), position2di(ResX, 249.25), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 258.42), position2di(ResX + 10, 258.42), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 267.59), position2di(ResX, 267.59), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 276.76), position2di(ResX + 10, 276.76), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 285.93), position2di(ResX, 285.93), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 295.1), position2di(ResX + 10, 295.1), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 304.27), position2di(ResX, 304.27), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 313.44), position2di(ResX + 10, 313.44), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 322.61), position2di(ResX, 322.61), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 331.78), position2di(ResX + 10, 331.78), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 340.95), position2di(ResX, 340.95), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 350.12), position2di(ResX + 10, 350.12), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 359.29), position2di(ResX, 359.29), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 368.46), position2di(ResX + 10, 368.46), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 377.63), position2di(ResX, 377.63), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 386.8), position2di(ResX + 10, 386.8), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 395.97), position2di(ResX, 395.97), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 405.14), position2di(ResX + 10, 405.14), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 414.31), position2di(ResX, 414.31), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 423.48), position2di(ResX + 10, 423.48), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 432.65), position2di(ResX, 432.65), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 441.82), position2di(ResX + 10, 441.82), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 450.99), position2di(ResX, 450.99), rulerColor);
	driverFor2D->draw2DLine(position2di(ResX - 10, 460), position2di(ResX + 10, 460), rulerColor);
	guienvFor2D->addStaticText(L"0", rect<s32>(18, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"1", rect<s32>(37.35, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"2", rect<s32>(56.71, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"3", rect<s32>(76.07, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"4", rect<s32>(95.43, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"5", rect<s32>(114.79, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"6", rect<s32>(134.15, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"7", rect<s32>(153.51, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"8", rect<s32>(172.87, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"9", rect<s32>(192.23, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"10", rect<s32>(211.59, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"11", rect<s32>(230.95, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"12", rect<s32>(250.31, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"13", rect<s32>(269.67, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"14", rect<s32>(289.03, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"15", rect<s32>(308.36, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"16", rect<s32>(327.44, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"17", rect<s32>(347.08, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"18", rect<s32>(366.44, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"19", rect<s32>(385.8, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"20", rect<s32>(405.16, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"21", rect<s32>(424.52, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"22", rect<s32>(443.88, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"23", rect<s32>(463.24, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"24", rect<s32>(482.6, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"25", rect<s32>(501.96, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"26", rect<s32>(521.32, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"27", rect<s32>(540.68, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"28", rect<s32>(560.04, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"29", rect<s32>(579.4, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"30", rect<s32>(598.76, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"31", rect<s32>(618, 500, 640, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"0", rect<s32>(660, 16, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"1", rect<s32>(660, 34.34, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"2", rect<s32>(660, 52.68, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"3", rect<s32>(660, 71.02, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"4", rect<s32>(660, 89.36, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"5", rect<s32>(660, 107.11, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"6", rect<s32>(660, 126.04, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"7", rect<s32>(660, 144.38, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"8", rect<s32>(660, 162.72, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"9", rect<s32>(660, 181.06, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"10", rect<s32>(660, 199.4, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"11", rect<s32>(660, 217.74, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"12", rect<s32>(660, 236.08, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"13", rect<s32>(660, 254.42, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"14", rect<s32>(660, 272.76, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"15", rect<s32>(660, 291.1, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"16", rect<s32>(660, 309.44, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"17", rect<s32>(660, 327.78, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"18", rect<s32>(660, 346.12, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"19", rect<s32>(660, 364.46, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"20", rect<s32>(660, 382.8, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"21", rect<s32>(660, 401.14, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"22", rect<s32>(660, 419.48, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"23", rect<s32>(660, 437.82, 700, 520), false)->setOverrideColor(rulerColor);
	guienvFor2D->addStaticText(L"24", rect<s32>(660, 456, 700, 520), false)->setOverrideColor(rulerColor);
}


void printGraphic(video::IVideoDriver* driverFor2D, gui::IGUIEnvironment* guienvFor2D, double *graphicX, double *graphicY, int size)
{
	SColor graphicColor = SColor(255, 255, 255, 255);
	guienvFor2D->addStaticText(L"ErrorX", rect<s32>(680, 20, 740, 195), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"Time", rect<s32>(905, 100, 940, 300), false)->setOverrideColor(graphicColor);
	driverFor2D->draw2DLine(position2di(697, 105), position2di(900, 105), graphicColor);
	driverFor2D->draw2DLine(position2di(890, 100), position2di(900, 105), graphicColor);
	driverFor2D->draw2DLine(position2di(890, 110), position2di(900, 105), graphicColor);
	driverFor2D->draw2DLine(position2di(700, 30), position2di(700, 180), graphicColor);
	driverFor2D->draw2DLine(position2di(695, 40), position2di(700, 30), graphicColor);
	driverFor2D->draw2DLine(position2di(705, 40), position2di(700, 30), graphicColor);
	driverFor2D->draw2DLine(position2di(695, 170), position2di(700, 180), graphicColor);
	driverFor2D->draw2DLine(position2di(705, 170), position2di(700, 180), graphicColor);

	driverFor2D->draw2DLine(position2di(697, 45), position2di(703, 45), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 55), position2di(703, 55), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 65), position2di(703, 65), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 75), position2di(703, 75), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 85), position2di(703, 85), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 95), position2di(703, 95), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 115), position2di(703, 115), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 125), position2di(703, 125), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 135), position2di(703, 135), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 145), position2di(703, 145), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 155), position2di(703, 155), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 165), position2di(703, 165), graphicColor);
	guienvFor2D->addStaticText(L"600", rect<s32>(680, 40, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"500", rect<s32>(680, 50, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"400", rect<s32>(680, 60, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"300", rect<s32>(680, 70, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"200", rect<s32>(680, 80, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"100", rect<s32>(680, 90, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"0", rect<s32>(680, 100, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-100", rect<s32>(678, 110, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-200", rect<s32>(678, 120, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-300", rect<s32>(678, 130, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-400", rect<s32>(678, 140, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-500", rect<s32>(678, 150, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-600", rect<s32>(678, 160, 697, 520), false)->setOverrideColor(graphicColor);

	guienvFor2D->addStaticText(L"ErrorY", rect<s32>(680, 235, 740, 245), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"Time", rect<s32>(905, 320, 940, 350), false)->setOverrideColor(graphicColor);
	driverFor2D->draw2DLine(position2di(697, 325), position2di(900, 325), graphicColor);
	driverFor2D->draw2DLine(position2di(890, 320), position2di(900, 325), graphicColor);
	driverFor2D->draw2DLine(position2di(890, 330), position2di(900, 325), graphicColor);
	driverFor2D->draw2DLine(position2di(700, 250), position2di(700, 400), graphicColor);
	driverFor2D->draw2DLine(position2di(695, 260), position2di(700, 250), graphicColor);
	driverFor2D->draw2DLine(position2di(705, 260), position2di(700, 250), graphicColor);
	driverFor2D->draw2DLine(position2di(695, 390), position2di(700, 400), graphicColor);
	driverFor2D->draw2DLine(position2di(705, 390), position2di(700, 400), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 265), position2di(703, 265), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 275), position2di(703, 275), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 285), position2di(703, 285), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 295), position2di(703, 295), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 305), position2di(703, 305), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 315), position2di(703, 315), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 335), position2di(703, 335), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 345), position2di(703, 345), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 355), position2di(703, 355), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 365), position2di(703, 365), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 375), position2di(703, 375), graphicColor);
	driverFor2D->draw2DLine(position2di(697, 385), position2di(703, 385), graphicColor);
	guienvFor2D->addStaticText(L"600", rect<s32>(680, 260, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"500", rect<s32>(680, 270, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"400", rect<s32>(680, 280, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"300", rect<s32>(680, 290, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"200", rect<s32>(680, 300, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"100", rect<s32>(680, 310, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"0", rect<s32>(680, 320, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-100", rect<s32>(678, 330, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-200", rect<s32>(678, 340, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-300", rect<s32>(678, 350, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-400", rect<s32>(678, 360, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-500", rect<s32>(678, 370, 697, 520), false)->setOverrideColor(graphicColor);
	guienvFor2D->addStaticText(L"-600", rect<s32>(678, 380, 697, 520), false)->setOverrideColor(graphicColor);

	int x = 700;

	for (int i = 1; i < size; i++) {
		
		driverFor2D->draw2DLine(position2di(x - 5, graphicX[i - 1] ), position2di(x, graphicX[i]), graphicColor);

		driverFor2D->draw2DLine(position2di(x - 5, graphicY[i - 1]), position2di(x, graphicY[i]), graphicColor);

		x += 5;
	}

}