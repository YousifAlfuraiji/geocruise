/* 
 * File:   FindPathAlgorithm.h
 * Author: furaijiy, mahroos2, chadhas7
 *
 * Created on March 15, 2019, 2:07 PM
 */

#pragma once

#include <string>
#include <cstring> 
#include <cmath> 
#include <set>
#include <map>
#include <vector>
#include <queue>
#include <limits>

#define NO_EDGE -1              // Illegal edge ID -> no reaching edge 
#define NO_INTERSECTION -1      // Illegal intersection ID -> no reaching intersection

class Node {
private:
    unsigned intersection_id;               //Intersection ID of the node
    //bool visited = false;                   //Bool for if node has been sent into the heap
    int best_reaching_edge = NO_EDGE;  // ID of the edge used to reach this node
    int best_reaching_intersection = NO_INTERSECTION;              //ID of the intersection used to reach this node
    double best_time_from_source = std::numeric_limits<double>::max();  //Time value to get from the source to this node
    double linear_distance_to_destination;                              //Distance from node to the destination
    
public:
    // Constructor
    Node(unsigned id){
        intersection_id = id; 
    }
    
    Node(int id, int reach_edge, int reaching_intersection, double best_time) : 
    intersection_id(id), best_reaching_edge(reach_edge),
    best_reaching_intersection(reaching_intersection), best_time_from_source(best_time){    
    }
    // Copy constructor

    Node(const Node &node2) {
        intersection_id = node2.intersection_id;
        best_reaching_edge = node2.best_reaching_edge;
        best_reaching_intersection = node2.best_reaching_intersection;
        best_time_from_source = node2.best_time_from_source;
    }
    
    //ACCESSORS
    unsigned get_intersection_id(){
        return intersection_id;
    }

    int get_best_reaching_edge(){
        return best_reaching_edge;
    }

    int get_best_reaching_intersection(){
        return best_reaching_intersection;
    }

    double get_best_time_from_source(){
        return best_time_from_source;
    }
    
    double get_linear_distance_to_destination(){
        return linear_distance_to_destination;
    }
    
    //MUTATORS
    void set_intersection_id(unsigned id){
        intersection_id = id;
    }

    void set_best_reaching_edge(int reaching_e){
        best_reaching_edge = reaching_e;
    }

    void set_best_reaching_intersection(int intersection){
        best_reaching_intersection = intersection;
    }

    void set_best_time_from_source(double time){
         best_time_from_source = time;
    }
    
    void set_linear_distance_to_destination(double distance){
        linear_distance_to_destination = distance;
    }
};

//Pair of time value and intersection ID
typedef std::pair<double, unsigned> hNode;

// Intersections to call the path finding algorithm on 
extern int IntersectionPathBegin; 
extern int IntersectionPathEnd;

// Checker for if the searching algorithm has reached the street segment before
extern std::vector<bool> street_segment_visited;

// The latest found path
extern std::vector<unsigned> found_path;

//Function declarations
void set_up_node_vector(std::vector<Node>& node_vector, const unsigned intersection_id );
std::vector<unsigned> path_trace_back(unsigned intersect_id_start, unsigned intersect_id_end, std::vector<Node>& node_vector);