/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "FindPathAlgorithm.h"
#include "MAPINFO.h"
#include "m1.h"

//Function to set up the vector of Nodes
void set_up_node_vector(std::vector<Node>& node_vector, const unsigned intersection_id ) {
    //Loops through the intersections and creates Nodes based upon the intersection ids
    for (signed i = 0; i < getNumIntersections(); ++i) {
        node_vector.push_back(Node(i));
    }
    //Sets the time value of the source intersection to 0
    node_vector[intersection_id].set_best_time_from_source(0);
}

//Function to trace the path from the source intersection to the destination
std::vector<unsigned> path_trace_back(unsigned intersect_id_start, unsigned intersect_id_end, std::vector<Node>& node_vector) {
    
    //Creates a final path vector to be returned (if no path, then empty vector is returned)
    std::vector<unsigned> final_path;
    if(node_vector[intersect_id_end].get_best_reaching_edge() == NO_EDGE) return final_path;
    
    //The focus begins at the destination
    unsigned current_id = intersect_id_end;
    
    //Loop goes backwards by accessing the reaching edge and intersection until the intersection is reached
    while ((current_id != intersect_id_start)) {
        final_path.insert(final_path.begin(), node_vector[current_id].get_best_reaching_edge());
        current_id = node_vector[current_id].get_best_reaching_intersection();
    }
    return final_path;
}