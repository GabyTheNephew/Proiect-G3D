#pragma once

#include <vector>
#include <string>

enum class Weather {
    Sunny,
    Rainy,
    Snowy
};


enum class TimeOfDay {
    Morning,
    Afternoon,
    Evening,
    Night
};


class Object3D {
public:
    std::string name;
    Object3D(std::string objName);
    void display() const;
};


class Environment {
private:
    Weather currentWeather;
    TimeOfDay timeOfDay;
    std::vector<Object3D> sceneryObjects;

public:
    Environment(Weather weather, TimeOfDay time, std::vector<Object3D> objects);
    void setWeather(Weather newWeather);
    void updateLighting();
    void displayEnvironment() const;
};


