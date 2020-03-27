/* 
 * File:   TSPHelpers.h
 * Author: furaijiy, mahroos2, chadhas7
 *
 * Created on March 29, 2019, 5:54 PM
 */

#pragma once

#include <map>
#include "m4.h"

// Location Type: Specifies whether a location is a dropoff or pickup
enum class type_of_location
{
    pickup,
    dropoff,
    done
};

class delivery_status {
private:
    //Location information 
    unsigned pickup_intersection;
    unsigned dropoff_intersection;
    
    //Conditions of the delivery status
    type_of_location location_type = type_of_location::pickup;
    bool delivery_done = false;
    
    //Other information
    int pickup_index;
    float package_weight;
    
    
public:
    //Constructors
    delivery_status(unsigned intersection1, unsigned intersection2, int index, float weight) :
    pickup_intersection(intersection1), dropoff_intersection(intersection2), pickup_index(index), package_weight(weight){    
    }
    
    //Accessors
    unsigned get_pickup_intersection(){
        return pickup_intersection;
    }
    
    unsigned get_dropoff_intersection(){
        return dropoff_intersection;
    }
    
    type_of_location get_location_type(){
        return location_type;
    }
    
    bool get_delivery_done(){
        return delivery_done;
    }
    
    int get_pickup_index(){
        return pickup_index;
    }
    
    float get_package_weight(){
        return package_weight;
    }
    
    //Mutators
    void set_pickup_intersection(unsigned id){
        pickup_intersection = id;
    }
    
    void set_dropoff_intersection(unsigned id){
        dropoff_intersection = id;
    }
    
    void set_location_type(type_of_location type){
        location_type = type;
    }
    
    void set_delivery_done(bool status){
        delivery_done = status;
    }
    
    void set_pickup_index(int index){
        pickup_index = index;
    }
    
    void set_package_weight(float weight){
        package_weight = weight;
    }  
};

std::vector<delivery_status> setup_delivery_status_vector(const std::vector<DeliveryInfo>& deliveries);

std::pair<std::vector<unsigned>, std::vector<unsigned>> find_path(
                const unsigned intersect_id_start, 
                std::vector<delivery_status>& delivery_status_vector,
                const double right_turn_penalty, 
                const double left_turn_penalty,
                const float truck_capacity,
                const float current_capacity);

std::pair<unsigned, std::vector<unsigned>> find_path_final_depot(
                const unsigned intersect_id_start, 
                const std::vector<unsigned>& depots,
                const double right_turn_penalty, 
                const double left_turn_penalty);

double find_path_first_depot(
                const unsigned intersect_id_start, 
                std::vector<delivery_status>& delivery_status_vector,
                const double right_turn_penalty, 
                const double left_turn_penalty);


// SOURCE: 297 m4 unit tester

double compute_courier_path_travel_time(const std::vector<CourierSubpath>& courier_route, 
                                        const float right_turn_penalty, 
                                        const float left_turn_penalty);