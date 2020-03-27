/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "MAPINFO.h"
#include "m3.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "FindPathAlgorithm.h"
#include <vector>

//Global vector for when a street segment is visited in the find path function
std::vector<bool> street_segment_visited;

//Function to return the turn type based on the street segments
TurnType find_turn_type(unsigned street_segment1, unsigned street_segment2){
    
    // The start and end of the 2 vectors to take dot product of
    LatLon ss1start, ss1end, ss2start, ss2end;
    
    // Check if the segments don't intersect
    if(getInfoStreetSegment(street_segment1).from != getInfoStreetSegment(street_segment2).from
            && getInfoStreetSegment(street_segment1).from != getInfoStreetSegment(street_segment2).to
            && getInfoStreetSegment(street_segment1).to != getInfoStreetSegment(street_segment2).from
            && getInfoStreetSegment(street_segment1).to != getInfoStreetSegment(street_segment2).to){
        
        return TurnType::NONE;
    }
    
    // Check if the segments are of the same street
    else if(getInfoStreetSegment(street_segment1).streetID == getInfoStreetSegment(street_segment2).streetID){
        
        return TurnType::STRAIGHT;
    }
    
    // There 4 different possibilities the 2 segments could intersect. Appropriate action is taken for each
    else if(getInfoStreetSegment(street_segment1).from == getInfoStreetSegment(street_segment2).from){  // Option A
    
        // Assign the vector ends taking into account the case where curve points do/don't exist
        if(getInfoStreetSegment(street_segment1).curvePointCount == 0){
            ss1start = getIntersectionPosition(getInfoStreetSegment(street_segment1).to);
        }else{
            ss1start = getStreetSegmentCurvePoint(0, street_segment1);
        }
        ss1end = getIntersectionPosition(getInfoStreetSegment(street_segment1).from);
        ss2start = getIntersectionPosition(getInfoStreetSegment(street_segment2).from);
        if(getInfoStreetSegment(street_segment2).curvePointCount == 0){
            ss2end = getIntersectionPosition(getInfoStreetSegment(street_segment2).to);
        }else{
            ss2end = getStreetSegmentCurvePoint(0, street_segment2);
        }  
       
    }
    else if(getInfoStreetSegment(street_segment1).from == getInfoStreetSegment(street_segment2).to){    // Option B
        
        if(getInfoStreetSegment(street_segment1).curvePointCount == 0){
            ss1start = getIntersectionPosition(getInfoStreetSegment(street_segment1).to);
        }else{
            ss1start = getStreetSegmentCurvePoint(0, street_segment1);
        }
        ss1end = getIntersectionPosition(getInfoStreetSegment(street_segment1).from);
        ss2start = getIntersectionPosition(getInfoStreetSegment(street_segment2).to);
        
        if(getInfoStreetSegment(street_segment2).curvePointCount == 0){
            ss2end = getIntersectionPosition(getInfoStreetSegment(street_segment2).from);
        }else{
            ss2end = getStreetSegmentCurvePoint(getInfoStreetSegment(street_segment2).curvePointCount - 1, street_segment2);
        }
        
    }
    else if(getInfoStreetSegment(street_segment1).to == getInfoStreetSegment(street_segment2).from){    // Option C
        
        if(getInfoStreetSegment(street_segment1).curvePointCount == 0){
            ss1start = getIntersectionPosition(getInfoStreetSegment(street_segment1).from);
        }else{
            ss1start = getStreetSegmentCurvePoint(getInfoStreetSegment(street_segment1).curvePointCount - 1, street_segment1);
        }
        ss1end = getIntersectionPosition(getInfoStreetSegment(street_segment1).to);
        ss2start = getIntersectionPosition(getInfoStreetSegment(street_segment2).from);
        if(getInfoStreetSegment(street_segment2).curvePointCount == 0){
            ss2end = getIntersectionPosition(getInfoStreetSegment(street_segment2).to);
        }else{
            ss2end = getStreetSegmentCurvePoint(0, street_segment2);
        }
        
    }
    else if(getInfoStreetSegment(street_segment1).to == getInfoStreetSegment(street_segment2).to){      // Option D
        
        if(getInfoStreetSegment(street_segment1).curvePointCount == 0){
            ss1start = getIntersectionPosition(getInfoStreetSegment(street_segment1).from);
        }else{
            ss1start = getStreetSegmentCurvePoint(getInfoStreetSegment(street_segment1).curvePointCount - 1, street_segment1);
        }
        
        ss1end = getIntersectionPosition(getInfoStreetSegment(street_segment1).to);
        ss2start = getIntersectionPosition(getInfoStreetSegment(street_segment2).to);
        if(getInfoStreetSegment(street_segment2).curvePointCount == 0){
            ss2end = getIntersectionPosition(getInfoStreetSegment(street_segment2).from);
        }else{
            ss2end = getStreetSegmentCurvePoint(getInfoStreetSegment(street_segment2).curvePointCount - 1, street_segment2);
        }
        
    }
    
    // Take the cross product
    double cross_product = (ss1end.lon() - ss1start.lon())*(ss2end.lat()-ss2start.lat())-(ss1end.lat() - ss1start.lat())*(ss2end.lon()-ss2start.lon());
    
    // If the cross product is positive, the segments are alligned so that a left turn is needed.
    // Otherwise, the turn type must be a right if cross prodct is 0 or less 
    if(cross_product > 0)
        return TurnType::LEFT;
    else
        return TurnType::RIGHT;
}

//Function to return the travel time given a path of street segments
double compute_path_travel_time(const std::vector<unsigned>& path, 
                                const double right_turn_penalty, 
                                const double left_turn_penalty){
    
    //If the path is empty
    if(path.size() == 0){
        return 0;
    }
    
    double total_time = 0;
    //Loop goes through all the street segments in the path
    for (unsigned i = 0; i < path.size(); ++i) {
        //Calculates time by dividing the length of the street segment by the speed limit
        total_time += find_street_segment_travel_time(path[i]);
        
        //Calculation to take into account left and right turns
        if (i < path.size() - 1) {
            if (find_turn_type(path[i], path[i+1]) == TurnType::RIGHT)
                total_time += right_turn_penalty;
            else if (find_turn_type(path[i], path[i+1]) == TurnType::LEFT)
                total_time += left_turn_penalty;
        }
    }
    return total_time;
}

//Function to return a path of street segments given a starting and ending point
std::vector<unsigned> find_path_between_intersections(
		  const unsigned intersect_id_start, 
                  const unsigned intersect_id_end,
                  const double right_turn_penalty, 
                  const double left_turn_penalty){
    
    //Sets all the elements in the vector to false
    street_segment_visited.resize(getNumStreetSegments(), false);
    
    /* Vector of Nodes is created and setup
     * Time value of starting intersection is set to 0
     * Time value of other intersections is set to the max double value */
    std::vector<Node> node_vector;
    set_up_node_vector(node_vector, intersect_id_start);
    
    /* Priority Queue (heap) is set up and contains heap_pairs
     * Each heap_pair contains a time value matched with an intersection id
     * Inserted the starting intersection into the priority queue */
    std::priority_queue<hNode, std::vector <hNode> , std::greater<hNode>> intersection_queue;  
        intersection_queue.push(std::make_pair(node_vector[intersect_id_start].get_best_time_from_source(), intersect_id_start));
    
    /* Declared some variables used in creating the path
     * ss_time: the time value associated with an intersection
     * other_intersection_id: intersection id of an adjacent node
     * connected: a bool to determine if the other_intersection_id should be evaluated or not*/
    double ss_time;
    unsigned other_intersection_id;
    bool connected;
    
    //Loop goes through all of the intersections until all intersections have been updated or destination is reached
    while (!intersection_queue.empty()) {
        //Takes off the top node
        unsigned id_top_heap = intersection_queue.top().second;
        intersection_queue.pop();
        
        //If the destination is reached, then break out of the loop
        if (id_top_heap == intersect_id_end) break;
        
        //Loop through the street segments associated with the intersection
        for (int i = 0; i < getIntersectionStreetSegmentCount(id_top_heap); ++i) {
            unsigned ss_id = getIntersectionStreetSegment(i, id_top_heap);
            if (street_segment_visited[ss_id]) continue;
            //Obtains the intersection id at the other end of the street segment
            if (id_top_heap == static_cast<unsigned>(getInfoStreetSegment(ss_id).from))
                other_intersection_id = getInfoStreetSegment(ss_id).to;
            else 
                other_intersection_id = getInfoStreetSegment(ss_id).from;
            
            //Determines if the intersections are truly directly connected
            if (id_top_heap == static_cast<unsigned>(getInfoStreetSegment(ss_id).from)) 
                connected = true;
            else connected = !getInfoStreetSegment(ss_id).oneWay;
            
            if (!connected) continue;
            //Checks if top of the heap is not the source intersection because turn penalties will need to be accounted for
            if (id_top_heap != intersect_id_start) {
                /* Obtains the turn type to get from the top_heap_id to the other_intersection
                 * Calculates the travel time of the street segment */
                TurnType turntype = find_turn_type(node_vector[id_top_heap].get_best_reaching_edge(), ss_id);
                ss_time = MAPDATA.street_segments_travel_times[ss_id];

                //Add necessary turn penalties
                if (turntype == TurnType::LEFT)
                    ss_time += left_turn_penalty;
                else if (turntype == TurnType::RIGHT)
                    ss_time += right_turn_penalty;
            }
            else
                ss_time = MAPDATA.street_segments_travel_times[ss_id];

            //If a better path is found, updates the value for that corresponding intersection
            if ((node_vector[other_intersection_id].get_best_time_from_source() > node_vector[id_top_heap].get_best_time_from_source() + ss_time)
                    && (node_vector[id_top_heap].get_best_time_from_source() + ss_time < node_vector[intersect_id_end].get_best_time_from_source())) {

                //Updates the time value, reaching edge, reaching intersection from the Node class and the street segment visited bool
                node_vector[other_intersection_id].set_best_time_from_source(node_vector[id_top_heap].get_best_time_from_source() + ss_time);
                node_vector[other_intersection_id].set_best_reaching_edge(ss_id);
                node_vector[other_intersection_id].set_best_reaching_intersection(id_top_heap);
                street_segment_visited[ss_id] = true;

                //Calculates distance from the intersection to the destination for added emphasis
                node_vector[other_intersection_id].set_linear_distance_to_destination(find_distance_between_two_points(getIntersectionPosition(intersect_id_end),
                                                                                                               getIntersectionPosition(other_intersection_id)));

                //Adds the updated pair back into the priority queue to be reevaluated
                intersection_queue.push(std::make_pair(node_vector[other_intersection_id].get_best_time_from_source() 
                     + node_vector[other_intersection_id].get_linear_distance_to_destination() * 0.01, other_intersection_id));

                /*//Calculates distance from the intersection to the destination for added emphasis
                node_vector[other_intersection_id].linear_distance_to_destination = 
                        find_distance_between_two_points(getIntersectionPosition(intersect_id_end),
                        getIntersectionPosition(other_intersection_id));

                //Adds the updated pair back into the priority queue to be reevaluated
                intersection_queue.push(std::make_pair(node_vector[other_intersection_id].get_best_time_from_source() 
                     + node_vector[other_intersection_id].linear_distance_to_destination * 0.01, other_intersection_id));*/
            }
        }    
    }
    
    //Path of street segments is traced
    std::vector<unsigned> traced_path = path_trace_back(intersect_id_start, intersect_id_end, node_vector);
    
    //Clear data structures
    node_vector.clear();
    street_segment_visited.clear();
    
    return traced_path;
}