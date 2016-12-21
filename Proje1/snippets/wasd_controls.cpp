// WASD controls for plate.

if (receiverForPlate.IsKeyDown(irr::KEY_KEY_A)) {
	//plateRotation.X += 0.3;
	secondServoLength += 0.021;
	secondArm->setScale(vector3df(0.05, secondServoLength, 0.05));
}
else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_D)) {
	//plateRotation.X -= 0.3;
	secondServoLength -= 0.021;
	secondArm->setScale(vector3df(0.05, secondServoLength, 0.05));
}
else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_W)) {
	plateRotation.Z += 0.3;
	firstServoLength += 0.012;
	firstArm->setScale(vector3df(0.05, firstServoLength, 0.05));
}
else if (receiverForPlate.IsKeyDown(irr::KEY_KEY_S)) {
	plateRotation.Z -= 0.3;
	firstServoLength -= 0.012;
	firstArm->setScale(vector3df(0.05, firstServoLength, 0.05));
}