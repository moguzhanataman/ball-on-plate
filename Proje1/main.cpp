#include <irrlicht.h>
#include <rect.h>
#include <iostream>

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
	IrrlichtDevice *device =
		createDevice( video::EDT_SOFTWARE, dimension2d<u32>(640, 480), 16,
			false, false, false, 0);

	if (!device)
		return 1;

	device->setWindowCaption(L"Hello World! - Irrlicht Engine Demo");

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();


    smgr->setAmbientLight(video::SColorf(0.0,0.0,0.0,1));
    ILightSceneNode* light1 = smgr->addLightSceneNode( 0, core::vector3df(0,400,-200), video::SColorf(0.3f,0.0f,0.0f), 1.0f, 1 );


	/* My Codes */
	scene::ISceneNode* plateSceneNode = smgr->addCubeSceneNode();
	if (plateSceneNode) {
        plateSceneNode->setScale(vector3df(1,5,5));
//        plateSceneNode->getMaterial(1).EmissiveColor.set(100,20,255,50);
	}
/*
    scene::ISceneNode* ballSceneNode = smgr->addSphereSceneNode(5);
    if (ballSceneNode) {
        ballSceneNode->setPosition(core::vector3df(0, 0, 0));
        ballSceneNode->setMaterialFlag(video::EMF_LIGHTING, true);

       ballSceneNode->getMaterial(0).AmbientColor = video::SColor(0, 0, 0, 255);
       ballSceneNode->getMaterial(0).DiffuseColor = video::SColor(255, 0, 0, 255);
       ballSceneNode->getMaterial(0).SpecularColor = video::SColor(0, 0, 0, 255);
       ballSceneNode->getMaterial(0).Shininess = 0.f;
    }*/

    IAnimatedMesh* mesh2 = smgr->getMesh("Ball/ball.obj");
    ISceneNode* node2 = 0;

    if(mesh2)
    {
        node2 = smgr->addOctreeSceneNode(mesh2->getMesh(0), 0, -1, 1024);
        //node2 = smgr->addMeshSceneNode(mesh2->getMesh(0));
        node2->setMaterialFlag(EMF_LIGHTING,false);
        node2->setPosition(core::vector3df(0, 0, 0));
        node2->setScale(vector3df(15,15,15));
    }


//	guienv->addStaticText(L"Hello World! This is the Irrlicht Software renderer!",
//		rect<s32>(10,10,260,22), true);

    /*
	IAnimatedMesh* mesh = smgr->getMesh("../../media/sydney.md2");
	if (!mesh)
	{
		device->drop();
		return 1;
	}
	IAnimatedMeshSceneNode* node = smgr->addAnimatedMeshSceneNode( mesh );

	if (node)
	{
		node->setMaterialFlag(EMF_LIGHTING, false);
		node->setMD2Animation(scene::EMAT_STAND);
		node->setMaterialTexture( 0, driver->getTexture("../../media/sydney.bmp") );
	}
    */
	smgr->addCameraSceneNode(0, vector3df(0,30,-40), vector3df(0,5,0));

	while(device->run())
	{
		driver->beginScene(true, true, SColor(255,100,101,140));

		smgr->drawAll();
		guienv->drawAll();

		driver->endScene();
	}

	device->drop();

	return 0;
}


