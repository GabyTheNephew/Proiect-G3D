
#include <vector>
#include <string>

class Segment;
class Station;

class Track {
private:
    std::vector<Segment> trackSegments; 
    float trackLength;                  
    std::vector<Station> stations;      

public:
    Track();
    ~Track();

    void loadTrackModel(const std::string& filePath);
    Segment getSegmentAtPosition(float position) const;
    void displayTrack() const;

    float getTrackLength() const;
    const std::vector<Segment>& getTrackSegments() const;
    const std::vector<Station>& getStations() const;
};
