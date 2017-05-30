//
//  NavigatorSaved2.cpp
//  CS 32 Project 4
//
//  Created by Ashwin Vivek on 3/13/17.
//  Copyright © 2017 AshwinVivek. All rights reserved.
//

#include <stdio.h>
#include "provided.h"
#include <string>
#include <vector>
#include <queue>
#include "MyMap.h"
using namespace std;

class NavigatorImpl
{
public:
    NavigatorImpl();
    ~NavigatorImpl();
    bool loadMapData(string mapFile);
    NavResult navigate(string start, string end, vector<NavSegment>& directions) const;
private:
    MapLoader m;
    AttractionMapper a;
    SegmentMapper s;
    
    //What is a GraphNode?
    //A GraphNode contains a geocoordinate, the streetname it's on, the priority value used to determine its position in the priority queue and the distance travelled on the current path from the start to the node's geocoord.
    
    struct GraphNode
    {
        GraphNode(GeoCoord start, double d = 0, double i = 0, string street=""): distanceTravelledUntilHere(d), priority(i)
        {
            g = start;
            streetName = street;
            previousNode = nullptr;
        }
        GraphNode()
        {
            g = GeoCoord("-1","-1");
            distanceTravelledUntilHere = 0;
        }
        
        //priority is calculated as the sum of the distance already travelled on the path to the node, and the distance from the coordinate to the destination. I multiplied by 1.2 as this was the heuristic that yielded the most optimal routes.
        double updatePriority(GeoCoord destination)
        {
            priority = distanceTravelledUntilHere + distanceEarthMiles(g, destination)*1.2;
            return priority;
        }
        //add the distance already travelled and the distance of the segment just moved over
        int updateDistanceTravelled(double distanceAlready, GeoCoord oldPos)
        {
            distanceTravelledUntilHere = distanceAlready + distanceEarthMiles(oldPos, g);
            return distanceTravelledUntilHere;
        }
        
        bool operator<(const GraphNode& b) const
        {
            return g<b.g;
        }
        
        bool operator==(const GraphNode& b) const
        {
            return g == b.g;
        }
        bool operator>(const GraphNode& b) const
        {
            return g>b.g;
        }
        bool operator<(const GraphNode* a) const
        {
            return this->priority < a->priority;
        }
        bool operator()(GraphNode* a, GraphNode* b) const
        {
            return a->priority < b->priority;
        }
        
        GeoCoord g;
        string streetName;
        double distanceTravelledUntilHere;
        double priority;   //A* algorithm adds the cost to the heuristic to determine priority (lowest better)
        vector<NavSegment> navSegments;
        GraphNode* previousNode;
        int countDestroyedNodes;
    };
    
    struct CompareNodes     //comparator class that priority queue uses to compare Graphnodes by priority
    {
        bool operator()(GraphNode a, GraphNode b) const
        {
            return a.priority > b.priority;
        }
    };
    
    //Given a geosegment, calculate the angular direction in degrees
    string calculateDirection(GeoSegment gs) const
    {
        double angularDir = angleOfLine(gs);
        string directionMoved;
        if(angularDir >=0 && angularDir <= 22.5)
            directionMoved = "east";
        else if(angularDir > 22.5 && angularDir <= 67.5)
            directionMoved = "northeast";
        else if(angularDir > 67.5 && angularDir <= 112.5)
            directionMoved = "north";
        else if(angularDir > 112.5 && angularDir <= 157.5)
            directionMoved = "northwest";
        else if(angularDir > 157.5 && angularDir <= 202.5)
            directionMoved = "west";
        else if(angularDir > 202.5 && angularDir <= 247.5)
            directionMoved = "southwest";
        else if(angularDir > 247.5 && angularDir <= 292.5)
            directionMoved = "south";
        else if(angularDir > 292.5 && angularDir <= 337.5)
            directionMoved = "southeast";
        else if(angularDir > 337.5 && angularDir <= 360)
            directionMoved = "east";
        return directionMoved;
    }
    
};

NavigatorImpl::NavigatorImpl()
{
}

NavigatorImpl::~NavigatorImpl()
{
}

bool NavigatorImpl::loadMapData(string mapFile)
{
    if(!m.load(mapFile))
        return false;
    a.init(m);
    s.init(m);
    
    return true;
}

NavResult NavigatorImpl::navigate(string start, string end, vector<NavSegment> &directions) const
{
    NavSegment firstSegment;
    GeoCoord startCoord;
    GeoCoord endCoord;
    
    
    int countInstructions = 0;
    bool endFound = false;
    
    //convert to lowercase for comparison to attraction map, where all values are lowercase
    for(int i=0; i<start.length(); i++)
        start[i] = tolower(start[i]);
    for(int j=0; j<end.length(); j++)
        end[j] = tolower(end[j]);
    
    //check if attractions are valid
    if(!a.getGeoCoord(start, startCoord))
        return NAV_BAD_SOURCE;
    if(!a.getGeoCoord(end, endCoord))
        return NAV_BAD_DESTINATION;
    
    int priorityQueueIndex = 0;
    GraphNode n0;
    GraphNode m0;
    
    
    //Open list keeps track of all nodes that have yet to be examined (all possible paths at the current moment)
    //map a GraphNode to itself. Although it sounds silly, it allows me to use the find and associate functions of MyMap to great effect. And since these run in log(N) time, are quite efficient
    MyMap<GraphNode, GraphNode> OpenListMap;
    
    //Closed list keeps track of all nodes that have been visited and prevents back-tracking.
    MyMap<GraphNode, int> ClosedListMap;
    
    priority_queue<GraphNode, vector<GraphNode>, CompareNodes> pq[2];
    
    //clear the lists before starting
    OpenListMap.clear();
    
    //n0 is the Graphnode at the origin. Perform necessary preparation.
    n0 = GraphNode(startCoord);
    cerr << "Node created" << endl;
    n0.updatePriority(endCoord);
    
    //push the start coord to the queue and add it to the list (it will be popped immediately)
    pq[priorityQueueIndex].push(n0);
    OpenListMap.associate(n0, n0);
    
    //SPECIAL CASE: Both the start and end attractions are on the same segment, perform the check and return. Only needs to be done once
    if((s.getSegments(startCoord)[0].segment.start == s.getSegments(endCoord)[0].segment.start) && s.getSegments(startCoord)[0].segment.end == s.getSegments(endCoord)[0].segment.end)
    {
        directions.clear();
        NavSegment sameSegment(calculateDirection(GeoSegment(startCoord, endCoord)), s.getSegments(startCoord)[0].streetName,distanceEarthMiles(startCoord, endCoord), GeoSegment(startCoord, endCoord));
        directions.push_back(sameSegment);
        return NAV_SUCCESS;
    }
    

    while(!pq[priorityQueueIndex].empty())
    {
        GeoCoord currentCoord = pq[priorityQueueIndex].top().g;
        if(!(currentCoord == startCoord))
        {
            //n0 is the GraphNode at the top of the priority queue
            n0 = GraphNode(pq[priorityQueueIndex].top().g, pq[priorityQueueIndex].top().distanceTravelledUntilHere, pq[priorityQueueIndex].top().priority, pq[priorityQueueIndex].top().streetName);
            cerr << "Node created" << endl;
        }
        
        //mark the node as visited
        ClosedListMap.associate(n0, 1);
        
        //find the node in the map and store it locally to perform necessary operations.
        GraphNode currentNode = *OpenListMap.find(n0);
        
        pq[priorityQueueIndex].pop();
        
        vector<StreetSegment> vec = s.getSegments(currentCoord);
        
        //loop through all the segments that originate from this coordinate, as these represent all the different paths that originate from this point.
        for(int i=0; i<vec.size(); i++)
        {
            if(!endFound){
                GeoCoord possibleCoords;
                bool isProceeding = true;   //changes to false if turning
                string directionMoved;
                double distanceTravelledAlready;
                double priority;
                
                //at the current segment, check if any of the attractions matches the end
                for(int k=0; k<vec[i].attractions.size(); k++)
                {
                    string check = "";
                    for(int j=0; j<vec[i].attractions[k].name.size(); j++)
                        check+=tolower(vec[i].attractions[k].name[j]);
                    if(check == end){
                        endFound = true;
                        possibleCoords = vec[i].attractions[k].geocoordinates;
                        
                        if(vec[i].streetName != currentNode.streetName)
                            isProceeding = false;
                    }
                }
                
                //This if block is only performed for the first node
                if(currentNode.previousNode == nullptr && countInstructions == 0)
                {
                    GeoCoord lessDesirableCoordinate;
                    
                    if(distanceEarthMiles(vec[i].segment.start, endCoord) < distanceEarthMiles(vec[i].segment.end, endCoord))
                    {
                        possibleCoords = vec[i].segment.start;
                        lessDesirableCoordinate = vec[i].segment.end;
                        
                        currentNode.streetName = vec[i].streetName;
                        countInstructions++;
                    }
                    else{
                        possibleCoords = vec[i].segment.end;
                        lessDesirableCoordinate = vec[i].segment.start;
                        
                        currentNode.streetName = vec[i].streetName;
                        countInstructions++;
                    }
                    // adds both possible starting steps to the priority queue instead of picking one direction from the onset.

                    NavSegment desirable(calculateDirection(GeoSegment(currentCoord, possibleCoords)), vec[i].streetName, distanceEarthMiles(currentCoord, possibleCoords), GeoSegment(currentCoord, possibleCoords));
                    
                    m0 = GraphNode(possibleCoords);
                    m0.updateDistanceTravelled(0, currentCoord);
                    m0.updatePriority(endCoord);
                    m0.streetName = vec[i].streetName;
                    
                    m0.navSegments.push_back(desirable);

                    m0.previousNode = nullptr;
                    pq[priorityQueueIndex].push(m0);
                    OpenListMap.associate(m0, m0);
                    
                    NavSegment lessDesirable(calculateDirection(GeoSegment(currentCoord, lessDesirableCoordinate)), vec[i].streetName, distanceEarthMiles(currentCoord, lessDesirableCoordinate), GeoSegment(currentCoord, lessDesirableCoordinate));
                    
                    m0 = GraphNode(lessDesirableCoordinate);
                    cerr << "Node created" << endl;
                    m0.updateDistanceTravelled(0, currentCoord);
                    m0.updatePriority(endCoord);
                    m0.streetName = vec[i].streetName;
                    
                    m0.navSegments.push_back(lessDesirable);
                    
                    m0.previousNode = nullptr;
                    pq[priorityQueueIndex].push(m0);
                    OpenListMap.associate(m0, m0);
                }
                
                //if the current segment starts at the current node's geocoord, then move to the end of the segment
                if(!endFound && vec[i].segment.start == currentCoord){
                    possibleCoords = vec[i].segment.end;
                    if((currentNode.streetName) != "" && currentNode.streetName != vec[i].streetName && countInstructions >0){
                        isProceeding = false;
                    }
                    else{
                        currentNode.streetName = vec[i].streetName;
                        countInstructions++;
                    }
                }
                
                 //if the current segment ends at the current node's geocoord, then move to the end of the segment
                else if(!endFound && vec[i].segment.end == currentCoord){
                    possibleCoords = vec[i].segment.start;
                    if((currentNode.streetName) != "" && currentNode.streetName != vec[i].streetName && countInstructions >0){
                        isProceeding = false;
                    }
                    else{
                        currentNode.streetName = vec[i].streetName;
                        countInstructions++;
                    }
                }
                //special case, then just continue
                else if(!endFound)
                    continue;
                
                
                if(isProceeding)
                    directionMoved = calculateDirection(GeoSegment(currentCoord, possibleCoords));
                cerr << angleOfLine(GeoSegment(currentCoord, possibleCoords));
                
                
                //check if the node has already been visited before
                if(ClosedListMap.find(GraphNode(possibleCoords)) == nullptr)
                {
                    //m0 is the node following the current node.
                    
                    m0 = GraphNode(possibleCoords);
                    cerr << "Node created" << endl;
                    m0.updateDistanceTravelled(currentNode.distanceTravelledUntilHere, currentNode.g);
                    distanceTravelledAlready = m0.distanceTravelledUntilHere;
                    priority = m0.updatePriority(endCoord);
                    m0.streetName = vec[i].streetName;
                    
                    //if the node has not been added to the open list
                    if(OpenListMap.find(GraphNode(possibleCoords,distanceTravelledAlready,priority)) == nullptr)
                    {
                        NavSegment proceedSeg;
                        proceedSeg.m_streetName = vec[i].streetName;
                        
                        //if proceeding, then m0 will have one element in the navSegment vector
                        if(!endFound && isProceeding)
                        {
                            NavSegment proc(directionMoved, vec[i].streetName, distanceEarthMiles(currentCoord, possibleCoords), GeoSegment(currentCoord, possibleCoords));
                            
                            m0.navSegments.push_back(proc);
                        }
                        
                        //if turning, then there are 2 elements in the navSegment vector
                        else if(!endFound){
                            NavSegment turnSeg;
                            turnSeg.m_command = NavSegment::TURN;
                            turnSeg.m_streetName = vec[i].streetName;
                            if(angleBetween2Lines(currentNode.navSegments[0].m_geoSegment, GeoSegment(currentCoord, possibleCoords)) >= 180)
                                turnSeg.m_direction = "right";
                            else
                                turnSeg.m_direction = "left";
                            
                            m0.navSegments.push_back(turnSeg);
                            
                            NavSegment turnSeg2(calculateDirection(GeoSegment(currentCoord, m0.g)), m0.streetName, distanceEarthMiles(currentCoord, m0.g), GeoSegment(currentCoord, m0.g));
                            
                            m0.navSegments.push_back(turnSeg2);
                        }
                        
                        //if the end is found, then do the same thing
                        else if(endFound)
                        {
                            currentNode.streetName = vec[i].streetName;
                            
                            if(isProceeding){
                                NavSegment proc(directionMoved, vec[i].streetName, distanceEarthMiles(currentCoord, possibleCoords), GeoSegment(currentCoord, possibleCoords));
                                
                                m0.navSegments.push_back(proc);
                            }
                            else{
                                NavSegment turnSeg;
                                turnSeg.m_command = NavSegment::TURN;
                                if(angleBetween2Lines(currentNode.navSegments[0].m_geoSegment, GeoSegment(currentCoord, possibleCoords)) >= 180)
                                    turnSeg.m_direction = "right";
                                else
                                    turnSeg.m_direction = "left";
                                cerr << angleBetween2Lines(currentNode.navSegments[0].m_geoSegment, GeoSegment(currentCoord, possibleCoords));
                                
                                turnSeg.m_streetName = vec[i].streetName;
                                
                                m0.navSegments.clear();
                                
                                m0.navSegments.push_back(turnSeg);
                                
                                NavSegment turnSeg2(calculateDirection(GeoSegment(currentCoord, endCoord)), vec[i].streetName, distanceEarthMiles(currentCoord, endCoord), GeoSegment(currentCoord, endCoord));

                                m0.navSegments.push_back(turnSeg2);
                                
                            }
                        }
                        //set m0's previous node to the current node, so that we can trace back, and add it to the priority queue. If it has a lower priority, we will more likely follow this path.
                        m0.previousNode = (OpenListMap.find(currentNode));
                        pq[priorityQueueIndex].push(m0);
                        OpenListMap.associate(m0, m0);
                        
                    }
                    
                    //else if the coordinate being looked at already exists in the open list, then do the same thing, since the new m0 will have a lower priority and thus be higher in the priority queue
                    else if
                        ((*(OpenListMap.find(GraphNode(possibleCoords,distanceTravelledAlready,priority)))).priority > m0.priority)
                    {
                        
                        NavSegment proceedSeg;
                        proceedSeg.m_streetName = vec[i].streetName;
                        if(!endFound && isProceeding)
                        {
                            NavSegment proc(directionMoved, vec[i].streetName, distanceEarthMiles(currentCoord, possibleCoords), GeoSegment(currentCoord, possibleCoords));
                            
                            m0.navSegments.push_back(proc);
                        }
                        else if(!endFound){
                            NavSegment turnSeg;
                            turnSeg.m_command = NavSegment::TURN;
                            turnSeg.m_streetName = vec[i].streetName;
                            if(angleBetween2Lines(currentNode.navSegments[0].m_geoSegment, GeoSegment(currentCoord, possibleCoords)) >= 180)
                                turnSeg.m_direction = "right";
                            else
                                turnSeg.m_direction = "left";
                            
                            m0.navSegments.push_back(turnSeg);
                            
                            NavSegment turnSeg2(calculateDirection(GeoSegment(currentCoord, m0.g)), m0.streetName, distanceEarthMiles(currentCoord, m0.g), GeoSegment(currentCoord, m0.g));
                            
                            m0.navSegments.push_back(turnSeg2);
                            
                        }
                        else if(endFound)
                        {
                            if(isProceeding){
                                NavSegment proc(directionMoved, vec[i].streetName, distanceEarthMiles(currentCoord, possibleCoords), GeoSegment(currentCoord, possibleCoords));
                                m0.navSegments.push_back(proc);
                            }
                            else{
                                NavSegment turnSeg1;
                                turnSeg1.m_command = NavSegment::TURN;
                                if(angleBetween2Lines(currentNode.navSegments[0].m_geoSegment, GeoSegment(currentCoord, possibleCoords)) >= 180)
                                    turnSeg1.m_direction = "right";
                                else
                                    turnSeg1.m_direction = "left";
                                cerr << angleBetween2Lines(currentNode.navSegments[0].m_geoSegment, GeoSegment(possibleCoords, currentCoord));
                                
                                turnSeg1.m_streetName = vec[i].streetName;
                                
                                currentNode.navSegments.clear();
                                
                                currentNode.navSegments.push_back(turnSeg1);
                                
                                NavSegment turnSeg2(calculateDirection(GeoSegment(currentCoord, m0.g)), m0.streetName, distanceEarthMiles(currentCoord, m0.g), GeoSegment(currentCoord, m0.g));
                                
                                currentNode.navSegments.push_back(turnSeg2);
                                
                            }
                        }
                        OpenListMap.associate(currentNode, currentNode);
                        m0.previousNode = (OpenListMap.find(currentNode));
                        OpenListMap.associate(m0, m0);
                        pq[priorityQueueIndex].push(m0);
                        
                        //This part of the code was taken from the pseudocode on the code.activestate.com C++ A* recipe
                        while(!(pq[priorityQueueIndex].top().g==possibleCoords))
                        {
                            pq[1-priorityQueueIndex].push(pq[priorityQueueIndex].top());
                            pq[priorityQueueIndex].pop();
                        }
                        pq[priorityQueueIndex].pop();
                        
                        if(pq[priorityQueueIndex].size()>pq[1-priorityQueueIndex].size())
                            priorityQueueIndex = 1-priorityQueueIndex;
                        
                        while(!pq[priorityQueueIndex].empty())
                        {
                            pq[1-priorityQueueIndex].push(pq[priorityQueueIndex].top());
                            pq[priorityQueueIndex].pop();
                        }
                        priorityQueueIndex = 1-priorityQueueIndex;
                        pq[priorityQueueIndex].push(m0);
                    }
                }
            }
            
            //trace back until the first coordinate
            if(endFound)
            {
                directions.clear();
                vector<NavSegment> temp;
                while(m0.previousNode != nullptr)
                {
                    if(m0.navSegments.size() > 1)
                        for(int i=0; i<2; i++)
                            temp.push_back(m0.navSegments[1-i]);
                    else
                        temp.push_back(m0.navSegments[0]);
                    m0 = *(m0.previousNode);
                    
                    if(m0.previousNode == nullptr)
                    {
                        if(m0.navSegments.size() > 1)
                            for(int i=0; i<m0.navSegments.size(); i++)
                                temp.push_back(m0.navSegments[1-i]);
                        else
                            temp.push_back(m0.navSegments[0]);
                    }
                    
                }
                //reverse the directions since they are from last to first
                for(int i = temp.size()-1; i>=0; i--)
                    directions.push_back(temp[i]);
                while(!pq[priorityQueueIndex].empty())
                    pq[priorityQueueIndex].pop();
                OpenListMap.clear();
                ClosedListMap.clear();
                return NAV_SUCCESS;
            }
        }
        if(!endFound){
            cerr << "destroyed" << endl;
        }
    }
    OpenListMap.clear();
    ClosedListMap.clear();
    return NAV_NO_ROUTE;
}

//******************** Navigator functions ************************************

// These functions simply delegate to NavigatorImpl's functions.
// You probably don't want to change any of this code.

Navigator::Navigator()
{
    m_impl = new NavigatorImpl;
}

Navigator::~Navigator()
{
    delete m_impl;
}

bool Navigator::loadMapData(string mapFile)
{
    return m_impl->loadMapData(mapFile);
}

NavResult Navigator::navigate(string start, string end, vector<NavSegment>& directions) const
{
    return m_impl->navigate(start, end, directions);
}
