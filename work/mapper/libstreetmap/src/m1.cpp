/* 
 * Copyright 2019 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include<bits/stdc++.h> 
#include <string>
#include <cstring> 
#include <cmath> 
#include <set>
#include <map>
#include "MAPINFO.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

mapInfo MAPDATA;
std::string newMapOSM;

//Functions to load the map data and data structures
bool load_map(std::string map_name) {
    
    std::string streets_identifier = "streets";
    
    newMapOSM = map_name;
    newMapOSM.replace(newMapOSM.find(streets_identifier), streets_identifier.length(), "osm");
    std::cout << newMapOSM << std::endl;
    //Loads {map}.streets.bin file
    bool load_successful = loadStreetsDatabaseBIN(map_name) && loadOSMDatabaseBIN(newMapOSM);//Indicates whether the map has loaded successfully
    
    
    if (load_successful) {
        
        srand (time(NULL));
        
        //Creating the data structure that can access the ways from the OSM Database
        for (unsigned i = 0; i < getNumberOfWays(); ++i) {
            OSMID ID = getWayByIndex(i)->id();
            MAPDATA.way_OSM.insert(std::make_pair(ID, i));
        }
        
        //Creating the data structure that can access the nodes from the OSM Database
        for (unsigned i = 0; i < getNumberOfNodes(); ++i) {
            OSMID ID = getNodeByIndex(i)->id();
            MAPDATA.node_OSM.insert(std::make_pair(ID, i));
        }
        
        //Creating the data structure that can access the relations from the OSM Database
        for (unsigned i = 0; i < getNumberOfRelations(); ++i) {
            OSMID ID = getNodeByIndex(i)->id();
            MAPDATA.relation_OSM.insert(std::make_pair(ID, i));
        }
        
        
        //Resize vectors based on total values for indexing later
        MAPDATA.street_streetsegments.resize(getNumStreets());
        MAPDATA.street_intersections.resize(getNumStreets());
        MAPDATA.street_intersections_set.resize(getNumStreets());
        
        //Creating the street_street_segments data structure
        for (int ss_id = 0; ss_id < getNumStreetSegments(); ++ss_id) {
            MAPDATA.street_streetsegments[getInfoStreetSegment(ss_id).streetID].push_back(ss_id);
            MAPDATA.streetsegments_highlighter.push_back(false);

        }
        
        //Creating the street_intersections_set data structure
        for(int intersection_id = 0; intersection_id < getNumIntersections(); ++intersection_id){
            //Loops through the street segments of the intersection
            for(int ss_counter = 0; ss_counter < getIntersectionStreetSegmentCount(intersection_id); ++ss_counter){
                //Access street ID based on street segment and adds intersection to the data structure
                int ss = getIntersectionStreetSegment(ss_counter, intersection_id);
                MAPDATA.street_intersections_set[getInfoStreetSegment(ss).streetID].insert(intersection_id);
            }
        }
        
         
        //Loops through streets and copies the values from street_intersections_set
        for(int street_id = 0; street_id < getNumStreets(); ++street_id){
            //Creating the street_intersections data structure
            std::copy(MAPDATA.street_intersections_set[street_id].begin(), 
                    MAPDATA.street_intersections_set[street_id].end(), 
                    std::back_inserter(MAPDATA.street_intersections[street_id]));
            
            //Creating the streets_names_map data structure
            //Converts names to lowercase before storing them
            std::string lowerCaseVersion = getStreetName(street_id);
            std::transform(lowerCaseVersion.begin(), lowerCaseVersion.end(), lowerCaseVersion.begin(), ::tolower); 
            char first_char = lowerCaseVersion[0];
            MAPDATA.streets_names_map[first_char].insert(std::make_pair(lowerCaseVersion, street_id));
        }
        
        //Creating a data structure that stores the time length of each street segment
        for(int street_segments_id = 0; street_segments_id < getNumStreetSegments(); ++street_segments_id){
            double distance_m = find_street_segment_length(street_segments_id);
            double speed_mh = 1000.0 * getInfoStreetSegment(street_segments_id).speedLimit; 
            double time_h = distance_m/speed_mh;
            double time_s = time_h * 3600.0;
            MAPDATA.street_segments_travel_times.push_back(time_s);
        }
        
        //Creating the intersections data structure
        max_lat = getIntersectionPosition(0).lat();
        min_lat = max_lat;
        max_lon = getIntersectionPosition(0).lon();
        min_lon = max_lon;

        //Write code to initialize intersections

        MAPDATA.intersections.resize(getNumIntersections());

        for (int intersection_id = 0; intersection_id < getNumIntersections(); ++intersection_id) {
            MAPDATA.intersections[intersection_id].position = getIntersectionPosition(intersection_id);
            MAPDATA.intersections[intersection_id].name = getIntersectionName(intersection_id);


            max_lat = std::max(max_lat, MAPDATA.intersections[intersection_id].position.lat());
            min_lat = std::min(min_lat, MAPDATA.intersections[intersection_id].position.lat());
            max_lon = std::max(max_lon, MAPDATA.intersections[intersection_id].position.lon());
            min_lon = std::min(min_lon, MAPDATA.intersections[intersection_id].position.lon());

            //lat +=  intersections[id].position.lat();
            for(int ss_id = 0; ss_id < getIntersectionStreetSegmentCount(intersection_id); ++ss_id) {
                //Extracts the ID of the street segments and adds it to the data structure 
                int street_segment_id = getIntersectionStreetSegment(ss_id, intersection_id);
                MAPDATA.intersections[intersection_id].street_segments.push_back(street_segment_id);
            }
        }

        LAT_AVG = (max_lat + min_lat) * (DEG_TO_RAD/2);
        
    }
    return load_successful;
}


//Function that ensures the structures are empty before loading the next map
void close_map() {
    //Clears all data structures
    MAPDATA.street_streetsegments.clear();
    MAPDATA.streetsegments_highlighter.clear();
    MAPDATA.street_intersections.clear();
    MAPDATA.street_intersections_set.clear();
    MAPDATA.street_segments_travel_times.clear();
    MAPDATA.streets_names_map.clear();
    MAPDATA.intersections.clear();
    MAPDATA.node_OSM.clear();
    MAPDATA.way_OSM.clear();
    MAPDATA.relation_OSM.clear();
    closeOSMDatabase();
    closeStreetDatabase();
    selectedIntersectionPathBegin = -1;
    selectedIntersectionPathEnd = -1;
}

// Returns a vector of street segments at the given intersection
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id){
    //return MAPDATA.intersection_street_segments[intersection_id];
    return MAPDATA.intersections[intersection_id].street_segments;
}

//Returns the street names at the given intersection
std::vector<std::string> find_intersection_street_names(unsigned intersection_id){
    
    std::vector<std::string> intersection_names;
    // Iterating through all streets segments of a given intersection
    for(int ss_counter = 0; ss_counter < getIntersectionStreetSegmentCount(intersection_id); ++ss_counter){
        int ss_id = getIntersectionStreetSegment(ss_counter, intersection_id);
        int s_id = getInfoStreetSegment(ss_id).streetID;
        intersection_names.push_back(getStreetName(s_id));
    }
    return intersection_names;
}

// Returns true if you can get from intersection1 to intersection2 using a single 
// street segment
bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2){
    //Checks if intersection 1 and 2 are the same
    if(intersection_id1 == intersection_id2) return true;       
    
    //Vectors that hold the street segments corresponding to the intersections  
    std::vector<unsigned int> street_segments_idVec1 = MAPDATA.intersections[intersection_id1].street_segments;
    std::vector<unsigned int> street_segments_idVec2 = MAPDATA.intersections[intersection_id2].street_segments;
    
    bool found_connecting_street = false;      //Flag to check for a one way street when a match is found
    bool can_drive_from_a_to_b = false;
    int street_segment_id;  //Variable to hold street segment ID
    
    //Loop goes through street segments to find a match
    for(int i = 0; i < getIntersectionStreetSegmentCount(intersection_id1); ++i){
            for(int j = 0; j < getIntersectionStreetSegmentCount(intersection_id2); ++j){
                //If a match is found, flag will be checked and value is stored
                if (street_segments_idVec1[i] == street_segments_idVec2[j]){
                    found_connecting_street = true;
                    street_segment_id = street_segments_idVec1[i];
                }
            }
    }
    
    //If match is found, checks for if it is a one way street
    if(found_connecting_street == true){
        can_drive_from_a_to_b = true;
        if (getInfoStreetSegment(street_segment_id).oneWay == true){
            if(static_cast<unsigned>(getInfoStreetSegment(street_segment_id).from) == intersection_id2){
                can_drive_from_a_to_b = false;
            }
        }
    }
    
    return can_drive_from_a_to_b;
}

// Returns all intersections reachable by traveling down one street segment 
// from given intersection
std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id){
    //Creates a set to hold the adjacent intersections
    //Set was chosen there would be no duplicate intersections
    std::set<unsigned> adj_intersections_set;
    
    //Loop goes through the street segments and evaluates the other intersections
    for(int ss_counter = 0; ss_counter < getIntersectionStreetSegmentCount(intersection_id); ++ss_counter){
        int ss_id = getIntersectionStreetSegment(ss_counter,  intersection_id);          //Stores street segment
        if (intersection_id == static_cast<unsigned>(getInfoStreetSegment(ss_id).from)) {
            //If the street segment is running from the intersection, adds intersection it is running to
            adj_intersections_set.insert(getInfoStreetSegment(ss_id).to);
        }
        else {
            //Evaluates if it is a one-way street segment and then adds the intersection
            if (getInfoStreetSegment(ss_id).oneWay == false) 
                adj_intersections_set.insert(getInfoStreetSegment(ss_id).from);
        }
    }
    //Creates a vector to hold the adjacent intersections and copies set values to vector
    std::vector<unsigned int> adj_intersections;
    std::copy(adj_intersections_set.begin(), adj_intersections_set.end(), std::back_inserter(adj_intersections));
 
    return adj_intersections;
}

// Returns all street segments for the given street
std::vector<unsigned> find_street_street_segments(unsigned street_id){
    //Indexing through vector to return vector that holds street segments 
    return MAPDATA.street_streetsegments[street_id];
}

// Returns all intersections along the a given street
std::vector<unsigned> find_all_street_intersections(unsigned street_id){
    //Indexing through vector to return vector that holds intersections
    return MAPDATA.street_intersections[street_id];
}
// Return all intersection ids for two intersecting streets
// This function will typically return one intersection id.
std::vector<unsigned> find_intersection_ids_from_street_ids(unsigned street_id1, unsigned street_id2){
    
    std::vector<unsigned> common_intersections;
    // Loop through street1 and see if any of them have have the same streetID and street2 
    for (auto it = MAPDATA.street_intersections[street_id1].begin(); 
            it != MAPDATA.street_intersections[street_id1].end(); 
            ++it) {
        if (std::find(MAPDATA.street_intersections[street_id2].begin(), MAPDATA.street_intersections[street_id2].end(), *it) != MAPDATA.street_intersections[street_id2].end())
            common_intersections.push_back(*it);
    }
    return common_intersections;
}

// Returns the distance between two coordinates in meters
double find_distance_between_two_points(LatLon point1, LatLon point2){
    //Convert radian latitude/longitude to rectangular (x,y)
    
    double latavg = (point1.lat() + point2.lat()) * DEG_TO_RAD/2;
    double x1 = point1.lon() * DEG_TO_RAD * std::cos(latavg);
    double y1 = point1.lat() * DEG_TO_RAD;
    
    double x2 = point2.lon() * DEG_TO_RAD * std::cos(latavg);
    double y2 = point2.lat() * DEG_TO_RAD;
    
    
    //Use Pythagoras to find distance
    double distance =   EARTH_RADIUS_IN_METERS * 
                        std::sqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1));
    
    return distance;
    
}

// Returns the length of the given street segment in meters
double find_street_segment_length(unsigned street_segment_id){
    
    double total_distance = 0;
    
    //Declare intersectionIndex variables for the start and finish of the street segment,
    //ie the intersections.
    IntersectionIndex intersection_from = getInfoStreetSegment(street_segment_id).from;
    IntersectionIndex intersection_to   = getInfoStreetSegment(street_segment_id).to;
    
    //Find the latitude and longitude of these intersections.
    LatLon from_latlon  = getIntersectionPosition(intersection_from);
    LatLon to_latlon    = getIntersectionPosition(intersection_to);

    //If it's a straight road, just find the distance using the 
    // "find_distance_between_two_points" function from above.
    if(getInfoStreetSegment(street_segment_id).curvePointCount == 0){
        total_distance = find_distance_between_two_points(from_latlon, to_latlon);
    }
    
    //If the street segment has curves, find the distance for the street segment by 
    //iterating through all the curve points, and add it up into total_distance.
    else{
        LatLon first_curve_point = getStreetSegmentCurvePoint(0, street_segment_id);
        total_distance = find_distance_between_two_points(from_latlon, first_curve_point);
        
        //Iterate through the curve points, and add the distance between them to the 
        //total_distance.
        int i = 0;
        for(; i < getInfoStreetSegment(street_segment_id).curvePointCount-1; i++){
            LatLon curve_point1 = getStreetSegmentCurvePoint(i, street_segment_id);
            LatLon curve_point2 = getStreetSegmentCurvePoint(i+1, street_segment_id);
            total_distance += find_distance_between_two_points(curve_point1, curve_point2);
        }
        LatLon last_curve_point = getStreetSegmentCurvePoint(i, street_segment_id);
        total_distance += find_distance_between_two_points(last_curve_point, to_latlon);
    }
    
    return total_distance;
}

// Returns the length of the specified street in meters
double find_street_length(unsigned street_id){
    
    double total_distance = 0;
    
    if(street_id >= static_cast<unsigned>(getNumStreets())) return total_distance; 
    
    //Iterating through the street segments of the street and finding the length of each segment
    //using the "find_street_segment_length" function from above, using ITERATOR.
    std::vector<unsigned> street_segments = MAPDATA.street_streetsegments[street_id];
    
    for(int i = 0; i < static_cast<int>(street_segments.size()); ++i)
        total_distance += find_street_segment_length(street_segments[i]);
                
    return total_distance;
}

// Returns the travel time to drive a street segment in seconds 
double find_street_segment_travel_time(unsigned street_segment_id){
    
    // Index the predetermined time length 
    
    return MAPDATA.street_segments_travel_times[street_segment_id];
    
}

// Returns the nearest point of interest to the given position
unsigned find_closest_point_of_interest(LatLon my_position){
    //Defined closest POI
    int closest_POI = 0;
    int oldDistance = 99999999;         //Defined distance between POI and point
    for(int poi = 0; poi < getNumPointsOfInterest(); ++poi){
        //Obtained position of POI
        LatLon poi_position = getPointOfInterestPosition(poi);
        
        //Obtained distance from point and POI
        int distanceCurrent = find_distance_between_two_points(my_position, poi_position);
        
        //If the distance is less than the previously stored POI, new POI is stored
        if(distanceCurrent < oldDistance) {
            oldDistance = distanceCurrent;
            closest_POI = poi;
        }
    }
    return closest_POI;
}

// Returns the nearest intersection to the given position
unsigned find_closest_intersection(LatLon my_position){
    //Defined closest intersection
    int closest_i = 0;
    //int oldDistance = 99999999;         //Defined distance between intersection and point
    int smallestDistance = find_distance_between_two_points(my_position, getIntersectionPosition(0));
    
    for(int intersection_id = 0; intersection_id < getNumIntersections(); ++intersection_id){
        //Obtained position of intersection
        LatLon i_position = getIntersectionPosition(intersection_id);
        
        //Obtained distance from point and intersection
        int distanceCurrent = find_distance_between_two_points(my_position, i_position);
        
        //If the distance is less than the previously stored intersection, new intersection is stored
        if(distanceCurrent < smallestDistance) {
            smallestDistance = distanceCurrent;
            closest_i = intersection_id;
        }
    }
    return closest_i;
}

//Returns all street ids corresponding to street names that start with the given prefix
std::vector<unsigned> find_street_ids_from_partial_street_name(std::string street_prefix){
    
    std::vector<unsigned> street_ids;
    
    // Convert all letters to lowercase
    std::transform(street_prefix.begin(), street_prefix.end(), street_prefix.begin(), ::tolower); 
    
    // Loop from the first occurrence of the street prefix in a street name
    // until the last occurrence
    for(auto it = MAPDATA.streets_names_map[street_prefix[0]].lower_bound(street_prefix); 
            it != MAPDATA.streets_names_map[street_prefix[0]].end(); ++it){
        if (street_prefix.length() > it->first.length())
            return street_ids;
        for (unsigned i = 0, len = street_prefix.length(); i<len; ++i) {
            if (it->first[i] != street_prefix[i])
                return street_ids;
        }
        street_ids.push_back(it->second);
    }
    return street_ids;
}

