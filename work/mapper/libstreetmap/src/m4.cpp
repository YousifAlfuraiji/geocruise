

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m4.h"
#include "m3.h"
#include "TSPHelpers.h"
#include "Optimality.h"
#include <chrono>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */


using namespace std;
using namespace std::chrono;

bool setUpSubpathAndCheckLegality(std::vector<CourierSubpath>& full_path, 
                                int startIntersection, int endIntersection, 
                                const double right_turn_penalty, const double left_turn_penalty);

std::vector<CourierSubpath> traveling_courier(
		const std::vector<DeliveryInfo>& deliveries,
	       	const std::vector<unsigned>& depots, 
		const float right_turn_penalty, 
		const float left_turn_penalty, 
		const float truck_capacity) {
    
    
    auto startTime = std::chrono::high_resolution_clock::now();
    double TIME_LIMIT = 43;
    //double DEPOT_SWITCHING_TIME_LIMIT = 40;
    bool timeOut = false;
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto wallClock = duration_cast<seconds>(currentTime - startTime).count();
    
    // this is the final result
    std::vector<CourierSubpath> full_path;
    
    //If there is any item greater than the truck capacity, then return empty vector
    for(unsigned j = 0; j < deliveries.size(); ++j){ 
        if(deliveries[j].itemWeight > truck_capacity){
            full_path.clear();
            return full_path;
        }   
    }
    std::vector <std::pair<unsigned,double>> depot_travel_times;
    std::vector<delivery_status> delivery_status_vector2 = setup_delivery_status_vector(deliveries);
    for (unsigned i = 0; i < depots.size(); ++i) {
        depot_travel_times.push_back(std::make_pair(depots[i],find_path_first_depot(depots[i], delivery_status_vector2, right_turn_penalty, left_turn_penalty)));
    }
    
    delivery_status_vector2.clear();
   
    std::map<double, std::vector<CourierSubpath>> courier_path_map;   
    //std::vector<std::pair<std::vector<type_of_location>, std::vector<unsigned>>> delivery_order_vector;
    
    unsigned index = 0;
    
    while(!timeOut && index < depots.size() && !depot_travel_times.empty()) {
        //std::cout << "///////////////////////Accessing a depot." << std::endl;
        unsigned best_index = 0;
        for (unsigned i = 1; i < depot_travel_times.size(); ++i) {
            if (depot_travel_times[i].second < depot_travel_times[best_index].second) {
                best_index = i;
            }
        }
        
        std::vector<delivery_status> delivery_status_vector = setup_delivery_status_vector(deliveries);
    
        float current_truck_load = 0;
        
        std::vector<unsigned> indices_vector;
        
        std::pair<std::vector<unsigned>, std::vector<unsigned>> current_path;
        CourierSubpath first_path_courier;
        
        current_path = find_path(depot_travel_times[best_index].first, delivery_status_vector, right_turn_penalty, left_turn_penalty, truck_capacity, current_truck_load);
        if (current_path.second.empty()) break;
        
        first_path_courier.start_intersection = depot_travel_times[best_index].first;
        first_path_courier.end_intersection = delivery_status_vector[current_path.first[0]].get_pickup_intersection();
        first_path_courier.subpath = current_path.second;
        full_path.push_back(first_path_courier);
        
        depot_travel_times.erase(depot_travel_times.begin() + best_index);
        
        if (full_path[full_path.size() - 1].subpath.empty() && full_path[full_path.size() - 1].start_intersection != full_path[full_path.size() - 1].end_intersection) {
                delivery_status_vector.clear();
                full_path.clear();
                //return full_path;
                break;
        }
        
        //Update the truck load value and switch the condition of pickup to dropoff status
        for (unsigned i = 0; i < current_path.first.size(); ++i){
            current_truck_load += delivery_status_vector[current_path.first[i]].get_package_weight();
            delivery_status_vector[current_path.first[i]].set_location_type(type_of_location::dropoff);
        }
        
        indices_vector = current_path.first;
        
//        std::vector<type_of_location> pickup_or_dropoff_indcator;
//        std::vector<unsigned> pickup_indices_indicator;
//        
//        pickup_or_dropoff_indcator.resize(indices_vector.size(), type_of_location::pickup);
//        delivery_order_vector.push_back(std::make_pair(pickup_or_dropoff_indcator, indices_vector));
//        pickup_or_dropoff_indcator.clear();
        
        while(!delivery_status_vector.empty()) { 
            currentTime = std::chrono::high_resolution_clock::now();
            wallClock = duration_cast<seconds>(currentTime - startTime).count();
            if(wallClock > TIME_LIMIT) {
                timeOut = true;
                delivery_status_vector.clear();
                full_path.clear();
                indices_vector.clear();
                //delivery_order_vector.clear();
                break;
            }
            
            //Find the path from the pickup to the dropoff 
            CourierSubpath current_path_courier;
            //current_path_courier.start_intersection = delivery_status_vector[current_path.first[0]].get_pickup_intersection();
            current_path_courier.start_intersection = full_path[full_path.size()-1].end_intersection;
            current_path = find_path(current_path_courier.start_intersection, delivery_status_vector, right_turn_penalty, 
                                left_turn_penalty, truck_capacity, current_truck_load);
            current_path_courier.subpath = current_path.second;
            
            current_path_courier.pickUp_indices = indices_vector;
            indices_vector.clear();
            
            for (unsigned i = 0; i < current_path.first.size(); ++i) {
                if (delivery_status_vector[current_path.first[i]].get_location_type() == type_of_location::dropoff) {
                    
                    current_path_courier.end_intersection = delivery_status_vector[current_path.first[i]].get_dropoff_intersection();
                    
                    // Remove this package from the current load in the truck
                    current_truck_load -= delivery_status_vector[current_path.first[i]].get_package_weight();
                    
//                    pickup_or_dropoff_indcator.push_back(type_of_location::dropoff);
//                    pickup_indices_indicator.push_back(delivery_status_vector[current_path.first[i]].get_pickup_index());
//                    
                    // Keep track of what the truck needs to drop off
                    delivery_status_vector.erase(delivery_status_vector.begin() + current_path.first[i]);
                    for (unsigned j = i; j < current_path.first.size(); ++j) {
                        current_path.first[j] = current_path.first[j] - 1;    
                    }
                    current_path.first.erase(current_path.first.begin() + i);
                }
            }
            
            if(wallClock > TIME_LIMIT) {
                timeOut = true;
                delivery_status_vector.clear();
                full_path.clear();
                indices_vector.clear();
                break;
            }
            
            for (unsigned i = 0; i < current_path.first.size(); ++i) {
                if (delivery_status_vector[current_path.first[i]].get_location_type() == type_of_location::pickup) {
                    if (delivery_status_vector[current_path.first[i]].get_package_weight() < truck_capacity - current_truck_load) {
                        current_path_courier.end_intersection = delivery_status_vector[current_path.first[i]].get_pickup_intersection();

                        // Add this package to the current load in the truck
                        current_truck_load += delivery_status_vector[current_path.first[i]].get_package_weight();

                        // Keep track of what the truck needs to drop off
                        delivery_status_vector[current_path.first[i]].set_location_type(type_of_location::dropoff);

                        indices_vector.push_back(delivery_status_vector[current_path.first[i]].get_pickup_index());
                        
//                        pickup_or_dropoff_indcator.push_back(type_of_location::pickup);
//                        pickup_indices_indicator.push_back(delivery_status_vector[current_path.first[i]].get_pickup_index());
                    }
                }
            }
            
//            delivery_order_vector.push_back(std::make_pair(pickup_or_dropoff_indcator, pickup_indices_indicator));
//            pickup_or_dropoff_indcator.clear();
//            pickup_indices_indicator.clear();
//            
            full_path.push_back(current_path_courier);
            if (full_path[full_path.size() - 1].subpath.empty() && full_path[full_path.size() - 1].start_intersection != full_path[full_path.size() - 1].end_intersection) {
                delivery_status_vector.clear();
                full_path.clear();
//                delivery_order_vector.clear();
                break;
            }
        }
        if (timeOut) break;
        
        //Path to last depot is considered to be done (added 0 as pickup index)
//        pickup_or_dropoff_indcator.push_back(type_of_location::done);
//        pickup_indices_indicator.push_back(0);
//        delivery_order_vector.push_back(std::make_pair(pickup_or_dropoff_indcator, pickup_indices_indicator));
//        pickup_or_dropoff_indcator.clear();
//        pickup_indices_indicator.clear();
        
        CourierSubpath final_depot_path_courier;
        final_depot_path_courier.start_intersection = full_path[full_path.size()-1].end_intersection;

        std::pair<unsigned, std::vector<unsigned>> final_path;
        final_path = find_path_final_depot(final_depot_path_courier.start_intersection, depots, right_turn_penalty, left_turn_penalty);
        final_depot_path_courier.end_intersection = final_path.first;
        final_depot_path_courier.subpath = final_path.second;
        full_path.push_back(final_depot_path_courier);

        current_path.first.clear();
        current_path.second.clear(); 
        delivery_status_vector.clear();

        for(unsigned i = 0; i < full_path.size()-2; ++i) {
            if((full_path[i+1].pickUp_indices.size() !=0 && full_path[i+2].pickUp_indices.size() !=0) ||
                    (full_path[i+1].pickUp_indices.size() == 0 && full_path[i+2].pickUp_indices.size() ==0)){ 
                //Need to edit this case
//                    || (delivery_order_vector[i].first[0] == type_of_location::pickup 
//                    && delivery_order_vector[i+1].first[0] == type_of_location::pickup
//                    && delivery_order_vector[i+2].first[0] == type_of_location::dropoff
//                    && delivery_order_vector[i].second[0] == delivery_order_vector[i+2].second[0])){

                currentTime = std::chrono::high_resolution_clock::now();
                wallClock = duration_cast<seconds>(currentTime - startTime).count();
                if(wallClock > TIME_LIMIT) {
                    timeOut = true;
                    break;
                }
                if (timeOut) break;
                
                double check_travel_time = compute_courier_path_travel_time(full_path, right_turn_penalty, left_turn_penalty);
                readjust(full_path, i, i+1, i+2, right_turn_penalty, left_turn_penalty);
                if(check_travel_time < compute_courier_path_travel_time(full_path, right_turn_penalty, left_turn_penalty)){
                    readjust(full_path, i, i+1, i+2, right_turn_penalty, left_turn_penalty);
                }
            }
        }
        
        double travel_time = 0;
        for (unsigned k = 0; k < full_path.size(); ++k) {
            travel_time += compute_path_travel_time(full_path[k].subpath, right_turn_penalty, left_turn_penalty);
        }
        courier_path_map.insert(std::make_pair(travel_time, full_path));
        full_path.clear();
        //delivery_order_vector.clear();
        
        ++index;
        currentTime = std::chrono::high_resolution_clock::now();
        wallClock = duration_cast<seconds>(currentTime - startTime).count();
        
        if(wallClock > TIME_LIMIT) {
            timeOut = true;
            break;
        }
    }
    
    std::vector<CourierSubpath> courierPath = courier_path_map.begin()->second;
    
    //auto currentTime = std::chrono::high_resolution_clock::now();
    //auto wallClock = duration_cast<seconds>(currentTime - startTime).count();
    //std::cout << "///////////////Starting 2-opt now: Time traversed is " << wallClock << " seconds." << std::endl;
    //Sorted by travel time and contains the path
    //std::map<double, std::vector<CourierSubpath>> two_opt_map;
    
    //two_opt_map.insert(std::make_pair(compute_courier_path_travel_time(courierPath, right_turn_penalty, right_turn_penalty), courierPath));
    
//    std::vector<CourierSubpath> two_opt_copy_of_courierPath;
//    
//    unsigned first_subpath_index = courierPath.size()/8;
//    unsigned second_subpath_index = courierPath.size()/8 + 2;
//    double current_best_time_traveled;
//    //int counter = 0;
//    int range = 5;
//    
//    while(wallClock < TIME_LIMIT && first_subpath_index < courierPath.size()-range && courierPath.size() > 3){
//        //++counter;
//        //std::cout << "Current time: " << wallClock << " seconds." << std::endl;
//        
//        //first_subpath_index = rand() % courierPath.size();
//        //second_subpath_index = first_subpath_index + 3;
//        
//        
//
//        
//        if(second_subpath_index == first_subpath_index + 1){
//            //std::cout << "here now " << std::endl;
//            continue;
//        }
//        
//        /*for(int i = 0; i < courierPath.size(); ++i) {
//            CourierSubpath tempSubpath;
//            tempSubpath.start_intersection = courierPath[i].start_intersection;
//            tempSubpath.end_intersection = courierPath[i].end_intersection;
//            tempSubpath.subpath = courierPath[i].subpath;
//            tempSubpath.pickUp_indices = courierPath[i].pickUp_indices;
//            
//            two_opt_copy_of_courierPath.push_back(tempSubpath);
//            
//        }*/
//        two_opt_copy_of_courierPath = courierPath;
//        
//        
//
//        
//        //std::cout << "Made a 2-opt operation with indices " << first_subpath_index << " and " << second_subpath_index 
//        //<< " with size " << two_opt_copy_of_courierPath.size() << std::endl;
//
//        
//        two_opt(two_opt_copy_of_courierPath, first_subpath_index, second_subpath_index, right_turn_penalty, left_turn_penalty);
//        //currentTime = std::chrono::high_resolution_clock::now();
//        //wallClock = duration_cast<seconds>(currentTime - startTime).count();
//        
//        if(!courier_path_is_legal_with_capacity(deliveries, depots, two_opt_copy_of_courierPath, truck_capacity)){
//            
//            two_opt_copy_of_courierPath.clear();
//            currentTime = std::chrono::high_resolution_clock::now();
//            wallClock = duration_cast<seconds>(currentTime - startTime).count();
//            ++first_subpath_index;
//            second_subpath_index = first_subpath_index+2;
//            continue;
//        }
//        
//        
//        
//        current_best_time_traveled = compute_courier_path_travel_time(two_opt_copy_of_courierPath, right_turn_penalty, left_turn_penalty);
//        //time_traveled = compute_courier_path_travel_time(two_opt_copy_of_courierPath, right_turn_penalty, right_turn_penalty);
//        
//        
//        
//        
//        
//        if(/*courier_path_is_legal_with_capacity(deliveries, depots, two_opt_copy_of_courierPath, truck_capacity)
//                //&& first_subpath_index != second_subpath_index
//                && */current_best_time_traveled < compute_courier_path_travel_time(courierPath, right_turn_penalty, left_turn_penalty)){
//            
//            /*for(int i = 0; i < courierPath.size(); ++i) {
//            
//                courierPath[i].start_intersection = two_opt_copy_of_courierPath[i].start_intersection;
//                courierPath[i].end_intersection = two_opt_copy_of_courierPath[i].end_intersection;
//                courierPath[i].subpath = two_opt_copy_of_courierPath[i].subpath;
//                courierPath[i].pickUp_indices = two_opt_copy_of_courierPath[i].pickUp_indices;
//            
//            }*/
//            courierPath = two_opt_copy_of_courierPath;
//            
//            //std::cout << "*************Operation succeeded" << std::endl;
//            //std::cout << "*************Current best time is " <<  current_best_time_traveled << std::endl;
//            
//            //if(full_path.size() < 60);
//            first_subpath_index = full_path.size()/8;
//            second_subpath_index = full_path.size()/8 + 1;
//        }
//        
//        ++second_subpath_index;
//        if(second_subpath_index >= courierPath.size()-range) {//first_subpath_index + range) {
//           
//            ++first_subpath_index;
//            second_subpath_index = first_subpath_index+2;
//        }
//        
//        
//        
//        two_opt_copy_of_courierPath.clear();
//        currentTime = std::chrono::high_resolution_clock::now();
//        wallClock = duration_cast<seconds>(currentTime - startTime).count();
//        
//    }
    
    //courier_path_map.clear();
    courier_path_map.clear();
    

   //std::cout << "///////////////////////Time to run traveling_courier is " << wallClock << " seconds." << std::endl;
   //std::cout << counter << " 2-opt iterations." << std::endl;

    return courierPath;
}

bool setUpSubpathAndCheckLegality(std::vector<CourierSubpath>& full_path, 
                                int startIntersection, int endIntersection, 
                                const double right_turn_penalty, const double left_turn_penalty) {
    
    // Set up the CourierSubpath
    CourierSubpath sub_route;
    sub_route.start_intersection = startIntersection;
    sub_route.end_intersection = endIntersection;
    sub_route.subpath = find_path_between_intersections(sub_route.start_intersection, 
                  sub_route.end_intersection, right_turn_penalty, left_turn_penalty);
    
    // Checks if the path is legal 
    // If the beginning and end intersections are not the same along with a path size of zero
    // indicates that no legal path was found
    if ((sub_route.subpath.size() == 0) && (sub_route.start_intersection != sub_route.end_intersection)){
            full_path.clear();
            return false;
        }
    
    full_path.push_back(sub_route);
    return true;
    
}
