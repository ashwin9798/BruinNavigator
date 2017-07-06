# BruinNavigator
A map-based C++ application that recommends optimal directions from point to point in the Los Angeles area. Data was obtained from OpenStreetMaps, and was parsed with a self-designed C++ parser.
The navigation is optimized using A*, a path finding algorithm that recommends the shortest path based on a carefully chosen heuristic.

# How to use it

clone this repo, and cd into the directory. Then run the following commands:

$ make
$ ./BruinNav "{{location1}}" "{{location2}}"
Replace {{location1}} and {{location2}} with the two locations you want directions for

# Available Locations

All available locations can be found in the file "validlocs.txt"

# Map Data

The map data was parsed by Carey Nachenberg from Open Street Maps.
