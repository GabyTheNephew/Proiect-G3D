#include "Environment.h"

Object3D::Object3D(std::string objName){
}

void Object3D::display() const{
}

Environment::Environment(Weather weather, TimeOfDay time, std::vector<Object3D> objects){
}

void Environment::setWeather(Weather newWeather){
}

void Environment::updateLighting(){
}

void Environment::displayEnvironment() const{
}
