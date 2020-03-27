#pragma once //protects against multiple inclusions of this header file


#include <iostream>
#include "StreetsDatabaseAPI.h"
#include<bits/stdc++.h> 
#include <string>
#include <cstring> 
#include <cmath> 
#include <set>
#include <map>
#include <vector>
//#include "m1.h"

struct intersection_data{
    LatLon position;
    bool visited = false;
    std::string name;
    bool highlight = false;
    std::vector<unsigned int> street_segments;
};

class mapInfo {
    public:

    //The following are the data structures used:
        
    //Vector of all intersections with each index containing all its street segments
    std::vector<intersection_data> intersections;

    //Vector of all streets with each index containing all street segments of that street
    std::vector<std::vector<unsigned>> street_streetsegments;
    
    //Vector of all street segment with a corresponding bool for highlighting the segment
    std::vector<bool> streetsegments_highlighter;

    //Vector of all streets with each index containing all the intersections of that street
    std::vector<std::vector<unsigned>> street_intersections;

    //Vector of all streets with each index containing all the intersections of that street in a set 
    //This is used to avoid duplicate intersections
    std::vector<std::set<unsigned>> street_intersections_set;

    //Vector that contains the street segments travel times
    std::vector<double> street_segments_travel_times;

    //Unordered map which contains the pair of the first character of street names 
    //and a multimap which contains the pair of the street names and street IDs
    std::unordered_map<char, std::multimap<std::string, unsigned>> streets_names_map;
    
    std::map<OSMID, unsigned> way_OSM;
    std::map<OSMID, unsigned> node_OSM;
    std::map<OSMID, unsigned> relation_OSM;
    
};

// These deal with the loading and changing of maps
extern std::string newMap;
extern std::string newMapOSM;
extern bool switchMap;

//These are for toggling the filters
extern bool hotelOn;
extern bool restaurantOn;
extern bool subwayOn;
extern bool barOn;
extern bool bankOn;
extern bool darkMode;

// Constants
const double PI = 3.141592653589793238462643383279;
const double right_turn_penalty_global = 15; 
const double left_turn_penalty_global = 25;

// These are needed for when drawing the map
extern double LAT_AVG;
extern double max_lat;
extern double min_lat;
extern double max_lon;
extern double min_lon;
extern double lat;
extern int lastHighlightedIndex;


// Destination path finding intersections
extern int selectedIntersectionPathBegin;
extern int selectedIntersectionPathEnd;

extern char *direction;

// The main database class
extern mapInfo MAPDATA;

//constexpr double EARTH_RADIUS = 6372797.560856;
//constexpr double DEG_TO_RADS = 0.017453292519943295769236907684886;

// Helper function: get the cartesian x value from Longitude.
float x_from_lon(float lon_par);
// Helper function: get the cartesian y value from Latitude.
float y_from_lat(float lat_par);

// Helper function: get the Longitude from cartesian x.
float lon_from_x(float x);

// Helper function: get the Latitude from cartesian y.
float lat_from_y(float y);
