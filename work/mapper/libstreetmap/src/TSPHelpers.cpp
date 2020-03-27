/* 
 * File:   TSPHelpers.cpp
 * Author: furaijiy, mahroos2, chadhas7
 *
 * Created on March 29, 2019, 5:39 PM
 */

#include "MAPINFO.h"
#include "m3.h"
#include "m1.h"
#include "m4.h"
#include "StreetsDatabaseAPI.h"
#include "FindPathAlgorithm.h"
#include "TSPHelpers.h"
#include <vector>
#include <limits>

//Returns a pair where the first element is a vector of indices for the delivery status vcector to update values
//(needs to be vector to account for multiple things at one intersection)
//and second element is the path of street segments
std::pair<std::vector<unsigned>, std::vector<unsigned>> find_path(
                const unsigned intersect_id_start, 
                std::vector<delivery_status>& delivery_status_vector,
                const double right_turn_penalty, 
                const double left_turn_penalty,
                const float truck_capacity,
                const float current_capacity){
    
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
    bool break_out = false;
    unsigned closest_intersection = 0;
    
    //Loop goes through all of the intersections until all intersections have been updated or destination is reached
    while (!intersection_queue.empty()) {
        //Takes off the top node
        unsigned id_top_heap = intersection_queue.top().second;
        intersection_queue.pop();
        
        break_out = false;
        for(unsigned i = 0; i < delivery_status_vector.size(); ++i) {
            //if (delivery_status_vector[i].get_package_weight() < truck_capacity - current_capacity) {
                //NEED TO REORDER THE IF STATEMENTS TO IMPROVE THE SPEED
                if (delivery_status_vector[i].get_location_type() == type_of_location::pickup &&
                    (delivery_status_vector[i].get_package_weight() < truck_capacity - current_capacity)) {
                        if (delivery_status_vector[i].get_pickup_intersection() == id_top_heap) {
                            break_out = true;
                            closest_intersection = id_top_heap;
                        }
                }
                else if (delivery_status_vector[i].get_location_type() == type_of_location::dropoff) {
                    if (delivery_status_vector[i].get_dropoff_intersection() == id_top_heap) {
                        break_out = true;
                        closest_intersection = id_top_heap;
                    }
                }
            //} 
        }
        
        if (break_out) break;
            
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
            if ((node_vector[other_intersection_id].get_best_time_from_source() > node_vector[id_top_heap].get_best_time_from_source() + ss_time)) {

                //Updates the time value, reaching edge, reaching intersection from the Node class and the street segment visited bool
                node_vector[other_intersection_id].set_best_time_from_source(node_vector[id_top_heap].get_best_time_from_source() + ss_time);
                node_vector[other_intersection_id].set_best_reaching_edge(ss_id);
                node_vector[other_intersection_id].set_best_reaching_intersection(id_top_heap);
                street_segment_visited[ss_id] = true;

                //Adds the updated pair back into the priority queue to be reevaluated
                intersection_queue.push(std::make_pair(node_vector[other_intersection_id].get_best_time_from_source(), other_intersection_id));
            }
        }    
    }
    
    std::vector<unsigned> traced_path = path_trace_back(intersect_id_start, closest_intersection, node_vector);
    std::vector<unsigned> vector_indices;
     for(unsigned i = 0; i < delivery_status_vector.size(); ++i) {
        if (delivery_status_vector[i].get_location_type() == type_of_location::pickup){
            if (delivery_status_vector[i].get_pickup_intersection() == closest_intersection) {
                if (delivery_status_vector[i].get_package_weight() < truck_capacity - current_capacity) {
                    vector_indices.push_back(i);
                }
            }
        }
        if (delivery_status_vector[i].get_location_type() == type_of_location::dropoff){
            if (delivery_status_vector[i].get_dropoff_intersection() == closest_intersection) {
                vector_indices.push_back(i);
            }
        }
    }
    
    std::pair<std::vector<unsigned>, std::vector<unsigned>> indices_with_path = std::make_pair(vector_indices, traced_path);
    
    //Clear data structures
    node_vector.clear();
    street_segment_visited.clear();
    traced_path.clear();
    vector_indices.clear();
    
    return indices_with_path;
}

std::vector<delivery_status> setup_delivery_status_vector(const std::vector<DeliveryInfo>& deliveries) {
    std::vector<delivery_status> delivery_status_vector;
    for(unsigned i = 0; i < deliveries.size(); ++i) {
        delivery_status_vector.push_back(delivery_status(deliveries[i].pickUp, deliveries[i].dropOff, i, deliveries[i].itemWeight));
    }
    return delivery_status_vector;
} 

std::pair<unsigned, std::vector<unsigned>> find_path_final_depot(
                const unsigned intersect_id_start, 
                const std::vector<unsigned>& depots,
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
    bool break_out = false;
    unsigned closest_intersection = 0;
    
    //Loop goes through all of the intersections until all intersections have been updated or destination is reached
    while (!intersection_queue.empty()) {
        //Takes off the top node
        unsigned id_top_heap = intersection_queue.top().second;
        intersection_queue.pop();
        
        break_out = false;
        for(unsigned i = 0; i < depots.size(); ++i) {
            if (depots[i] == id_top_heap) {
                break_out = true;
                closest_intersection = id_top_heap;
            }
        }
        
        if (break_out) break;
            
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
            if ((node_vector[other_intersection_id].get_best_time_from_source() > node_vector[id_top_heap].get_best_time_from_source() + ss_time)) {

                //Updates the time value, reaching edge, reaching intersection from the Node class and the street segment visited bool
                node_vector[other_intersection_id].set_best_time_from_source(node_vector[id_top_heap].get_best_time_from_source() + ss_time);
                node_vector[other_intersection_id].set_best_reaching_edge(ss_id);
                node_vector[other_intersection_id].set_best_reaching_intersection(id_top_heap);
                street_segment_visited[ss_id] = true;

                //Adds the updated pair back into the priority queue to be reevaluated
                intersection_queue.push(std::make_pair(node_vector[other_intersection_id].get_best_time_from_source(), other_intersection_id));
            }
        }    
    }
    
    std::vector<unsigned> traced_path = path_trace_back(intersect_id_start, closest_intersection, node_vector);
    std::pair<unsigned, std::vector<unsigned>> intersection_with_path = std::make_pair(closest_intersection, traced_path);
    
    //Clear data structures
    node_vector.clear();
    street_segment_visited.clear();
    traced_path.clear();
    
    return intersection_with_path;
}

double find_path_first_depot(
                const unsigned intersect_id_start, 
                std::vector<delivery_status>& delivery_status_vector,
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
    bool break_out = false;
    unsigned closest_intersection = 0;
    
    //Loop goes through all of the intersections until all intersections have been updated or destination is reached
    while (!intersection_queue.empty()) {
        //Takes off the top node
        unsigned id_top_heap = intersection_queue.top().second;
        intersection_queue.pop();
        
        break_out = false;
        for(unsigned i = 0; i < delivery_status_vector.size(); ++i) {
            if (delivery_status_vector[i].get_location_type() == type_of_location::pickup) {
                if (delivery_status_vector[i].get_pickup_intersection() == id_top_heap) {
                    break_out = true;
                    closest_intersection = id_top_heap;
                }
            }
        }
        
        if (break_out) break;
            
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
            if ((node_vector[other_intersection_id].get_best_time_from_source() > node_vector[id_top_heap].get_best_time_from_source() + ss_time)) {

                //Updates the time value, reaching edge, reaching intersection from the Node class and the street segment visited bool
                node_vector[other_intersection_id].set_best_time_from_source(node_vector[id_top_heap].get_best_time_from_source() + ss_time);
                node_vector[other_intersection_id].set_best_reaching_edge(ss_id);
                node_vector[other_intersection_id].set_best_reaching_intersection(id_top_heap);
                street_segment_visited[ss_id] = true;

                //Adds the updated pair back into the priority queue to be reevaluated
                intersection_queue.push(std::make_pair(node_vector[other_intersection_id].get_best_time_from_source(), other_intersection_id));
            }
        }    
    }
    
    double travel_time = node_vector[closest_intersection].get_best_time_from_source();
    //Clear data structures
    node_vector.clear();
    street_segment_visited.clear();
    
    return travel_time;
}

// SOURCE: 297 m4 unit tester

double compute_courier_path_travel_time(const std::vector<CourierSubpath>& courier_route, 
                                        const float right_turn_penalty, 
                                        const float left_turn_penalty) {
    // extract the whole route 
    std::vector<unsigned> path; 
    for (size_t i = 0; i < courier_route.size(); i++)
        path.insert(path.end(), courier_route[i].subpath.begin(), courier_route[i].subpath.end());

    // Call compute path function 
    return compute_path_travel_time(path, right_turn_penalty, left_turn_penalty);
}