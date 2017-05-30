#include "provided.h"
#include <string>
#include "MyMap.h"
using namespace std;

class AttractionMapperImpl
{
public:
	AttractionMapperImpl();
	~AttractionMapperImpl();
	void init(const MapLoader& ml);
	bool getGeoCoord(string attraction, GeoCoord& gc) const;
private:
    MyMap<string, GeoCoord> attractionMap;
};

AttractionMapperImpl::AttractionMapperImpl()
{
}

AttractionMapperImpl::~AttractionMapperImpl()
{
}

void AttractionMapperImpl::init(const MapLoader& ml)
{
    StreetSegment temp;
    int count =0;
    for(int i=0; i<ml.getNumSegments(); i++)
    {
//        cerr << i <<endl;
        ml.getSegment(i, temp);
        if(!temp.attractions.empty()){
            for(int k=0; k<temp.attractions.size(); k++){
                string t = "";
                for(int j=0; j<temp.attractions[k].name.size(); j++){
                    t+= tolower(temp.attractions[k].name[j]);
                }
                attractionMap.associate(t, temp.attractions[k].geocoordinates);
                count++;
                cerr << count << endl;
                cerr << attractionMap.size() << endl;
                
            }
        }
    }
    cerr << attractionMap.size() << endl;
}

bool AttractionMapperImpl::getGeoCoord(string attraction, GeoCoord& gc) const
{
    for(int i=0; i<attraction.size(); i++)
        attraction[i] = tolower(attraction[i]);
    const GeoCoord* gPtr = attractionMap.find(attraction);
    if(gPtr != nullptr){
        gc = *gPtr;
        return true;
    }
    return false;
}

//******************** AttractionMapper functions *****************************

// These functions simply delegate to AttractionMapperImpl's functions.
// You probably don't want to change any of this code.

AttractionMapper::AttractionMapper()
{
	m_impl = new AttractionMapperImpl;
}

AttractionMapper::~AttractionMapper()
{
	delete m_impl;
}

void AttractionMapper::init(const MapLoader& ml)
{
	m_impl->init(ml);
}

bool AttractionMapper::getGeoCoord(string attraction, GeoCoord& gc) const
{
	return m_impl->getGeoCoord(attraction, gc);
}
