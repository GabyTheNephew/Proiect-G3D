#include "Track.h"
#include <iostream> // For debugging or logging

Track::Track() : trackLength(0.0f) {}
Track::~Track() {}

void Track::loadTrackModel(const std::string& filePath) {
    std::cout << "Loading track model from: " << filePath << std::endl;

}


void Track::displayTrack() const {
    std::cout << "Rendering track..." << std::endl;

}

float Track::getTrackLength() const {
    return trackLength;
}

const std::vector<Segment>& Track::getTrackSegments() const {
    return trackSegments;
}

const std::vector<Station>& Track::getStations() const {
    return stations;
}
