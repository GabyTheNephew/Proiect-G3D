#pragma once
class Train
{
private:
	float m_speed;
	float m_acceleration;
	int m_numWagons;
	float m_position;
	bool m_hornActive;
public:
	void accelerate(float amount);
	void brake(float amount);
	void activate();
	void updatePosition(float deltaTime);
};

