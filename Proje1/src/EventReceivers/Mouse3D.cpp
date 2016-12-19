#include <irrlicht.h>

using namespace irr;
class MouseEventReceiverFor3D : public IEventReceiver
{
private:
	// We use this array to store the current state of each key
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		// Remember whether each key is down or up
		if (event.EventType == irr::EET_KEY_INPUT_EVENT)
			KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
		else if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
				switch (event.MouseInput.Event)
				{
				case EMIE_MOUSE_WHEEL:
					break;
				default:
					// We won't use the wheel
					break;
				}
		return false;
	}

	// This is used to check whether a key is being held down
	virtual bool IsKeyDown(EKEY_CODE keyCode) const
	{
		return KeyIsDown[keyCode];
	}

	

	MouseEventReceiverFor3D()
	{
		for (u32 i = 0; i < KEY_KEY_CODES_COUNT; ++i)
			KeyIsDown[i] = false;
	}


};