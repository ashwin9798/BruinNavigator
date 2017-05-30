#include "provided.h"
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

class MapLoaderImpl
{
public:
	MapLoaderImpl();
	~MapLoaderImpl();
	bool load(string mapFile);
	size_t getNumSegments() const;
	bool getSegment(size_t segNum, StreetSegment& seg) const;
private:
    vector<StreetSegment> segmentContainer;
};

MapLoaderImpl::MapLoaderImpl()
{
    cerr << "constructed" << endl;
}

MapLoaderImpl::~MapLoaderImpl()
{
    cerr << "destroyed" <<endl;
}

bool MapLoaderImpl::load(string mapFile)
{
    ifstream input(mapFile);
    if(input.peek() == input.eof())
        return false;
    
    string content;
    int count;

    bool examiningNewSegment = true;
    int containerIndex = 0;
    int linesToExamine = 3;
    int linesExamined = 0;
    
    string startLat, startLon, endLat, endLon, attractionLat, attractionLon, attractionName;
    
    while(getline(input, content)){
        if(linesExamined == 0)
        {
            StreetSegment s;
            segmentContainer.push_back(s);
            segmentContainer[containerIndex].streetName = content;
        }
        switch(linesExamined)
        {
            case 0:
                break;
            case 1:
                startLat =""; startLon = ""; endLat = ""; endLon = "";
                replace(content.begin(), content.end(), ' ', ',');
                for(int i=0, count = 0; i<content.size(); i++)
                {
                    if(content[i] != ',' && content[i]!='/'){
                        switch(count){
                            case 0:
                                startLat += content[i];
                                break;
                            case 1:
                                startLon += content[i];
                                break;
                            case 2:
                                endLat += content[i];
                                break;
                            case 3:
                                endLon += content[i];
                                break;
                        }
                    }
                    else if(content[i] == ',' && content[i+1] != ',')
                        count++;
                }
                segmentContainer[containerIndex].segment = GeoSegment(GeoCoord(startLat, startLon), GeoCoord(endLat, endLon));
                break;
            case 2:
                linesToExamine+=stoi(content);
                break;
            default:
                attractionName = "", attractionLat="",attractionLon="";
                bool nameProcessed = false;
                for(int i=0, count = 0; i<content.size(); i++)
                {
                    if((content[i]!='|' && content[i]!=',') || (content[i]==' ' && !nameProcessed))
                    {
                        switch(count)
                        {
                            case 0:
                                attractionName+=(content[i]);
                                break;
                            case 1:
                                attractionLat+=(content[i]);
                                break;
                            case 2:
                                attractionLon+=(content[i]);
                        }
                    }
                    else if(content[i]=='|'){
                        nameProcessed = true;
                        count++;
                    }
                    else if(content[i]==',' && nameProcessed){
                        count++;
                    }
                }
                Attraction a;
                segmentContainer[containerIndex].attractions.push_back(a);
                segmentContainer[containerIndex].attractions[linesExamined-3].name = attractionName;
                segmentContainer[containerIndex].attractions[linesExamined-3].geocoordinates = GeoCoord(attractionLat, attractionLon);
                count++;
                cerr << count << "attraction" << endl;
                    
        }
        linesExamined++;
        if(linesExamined == linesToExamine)
        {
            linesExamined = 0;
            linesToExamine = 3;
            containerIndex++;
        }
    }
    return true;
}

size_t MapLoaderImpl::getNumSegments() const
{
    return segmentContainer.size();
}

bool MapLoaderImpl::getSegment(size_t segNum, StreetSegment &seg) const
{
	if(segNum >= segmentContainer.size())
        return false;
    seg = segmentContainer[segNum];
    return true;
}

//******************** MapLoader functions ************************************

// These functions simply delegate to MapLoaderImpl's functions.
// You probably don't want to change any of this code.

MapLoader::MapLoader()
{
	m_impl = new MapLoaderImpl;
}

MapLoader::~MapLoader()
{
	delete m_impl;
}

bool MapLoader::load(string mapFile)
{
	return m_impl->load(mapFile);
}

size_t MapLoader::getNumSegments() const
{
	return m_impl->getNumSegments();
}

bool MapLoader::getSegment(size_t segNum, StreetSegment& seg) const
{
   return m_impl->getSegment(segNum, seg);
}
