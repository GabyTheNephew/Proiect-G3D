#pragma once
class UserInterface
{
private:
	float SpeedIndicator;
	bool hornStatus;

public:
	void displayHUD();
	void handleUserInput();
};

