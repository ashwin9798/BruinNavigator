//
//  support.cpp
//  CS 32 Project 4
//
//  Created by Ashwin Vivek on 3/12/17.
//  Copyright © 2017 AshwinVivek. All rights reserved.
//

#include <stdio.h>
#include "provided.h"

bool operator==(const GeoCoord& a, const GeoCoord& other)
{
    return(a.longitude == other.longitude && a.latitude == other.latitude);
}

bool operator<(const GeoCoord& a, const GeoCoord& other)
{
    if(a.latitude == other.latitude)
        return(a.longitude<other.longitude);
    return(a.latitude < other.latitude);
}
bool operator>(const GeoCoord& a, const GeoCoord& other) {
    if(a.latitude == other.latitude)
        return(a.longitude>other.longitude);
    return(a.latitude > other.latitude);
}
