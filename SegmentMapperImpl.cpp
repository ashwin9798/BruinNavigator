#include "provided.h"
#include <vector>
#include "MyMap.h"
using namespace std;

class SegmentMapperImpl
{
public:
	SegmentMapperImpl();
	~SegmentMapperImpl();
	void init(const MapLoader& ml);
	vector<StreetSegment> getSegments(const GeoCoord& gc) const;
private:
    MyMap<GeoCoord, vector<StreetSegment>> segMap;
};

SegmentMapperImpl::SegmentMapperImpl()
{
}

SegmentMapperImpl::~SegmentMapperImpl()
{
}

void SegmentMapperImpl::init(const MapLoader& ml)
{
    StreetSegment temp;
    cerr<<ml.getNumSegments()<<endl;
    for(int i=0; i<ml.getNumSegments(); i++)
    {
        ml.getSegment(i, temp);
        if(segMap.find(temp.segment.start) != nullptr)
            segMap.find(temp.segment.start)->push_back(temp);
        else{
            vector<StreetSegment> s;
            s.push_back(temp);
            segMap.associate(temp.segment.start, s);
        }
        if(segMap.find(temp.segment.end) != nullptr)
            segMap.find(temp.segment.end)->push_back(temp);
        else{
            vector<StreetSegment> s;
            s.push_back(temp);
            segMap.associate(temp.segment.end, s);
            
        }
        for(int i=0; i<temp.attractions.size(); i++)
        {
            if(segMap.find(temp.attractions[i].geocoordinates) != nullptr)
                segMap.find(temp.attractions[i].geocoordinates)->push_back(temp);
            else{
                vector<StreetSegment> s;
                s.push_back(temp);
                segMap.associate(temp.attractions[i].geocoordinates, s);
            }
        }
    }
    cerr << segMap.size() << endl;
}

vector<StreetSegment> SegmentMapperImpl::getSegments(const GeoCoord& gc) const
{
	if(segMap.find(gc) != nullptr)
    {
        vector<StreetSegment> returning;
        const vector<StreetSegment>* s = segMap.find(gc);
        for(int i=0; i<s->size(); i++)
            returning.push_back((*s)[i]);
        return returning;
    }
    return {};
}

//******************** SegmentMapper functions ********************************

// These functions simply delegate to SegmentMapperImpl's functions.
// You probably don't want to change any of this code.

SegmentMapper::SegmentMapper()
{
	m_impl = new SegmentMapperImpl;
}

SegmentMapper::~SegmentMapper()
{
	delete m_impl;
}

void SegmentMapper::init(const MapLoader& ml)
{
	m_impl->init(ml);
}

vector<StreetSegment> SegmentMapper::getSegments(const GeoCoord& gc) const
{
	return m_impl->getSegments(gc);
}
