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
#include <iostream>
#include <string>
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "MAPINFO.h"
#include "FindPathAlgorithm.h"
#include <random>
#include <iostream>
#include <unittest++/UnitTest++.h>

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m3.h"
#include "m4.h"
#include "Optimality.h"

//#include "unit_test_util.h"
//#include "courier_verify.h"


//Program exit codes
constexpr int SUCCESS_EXIT_CODE = 0;        //Everyting went OK
constexpr int ERROR_EXIT_CODE = 1;          //An error occured
constexpr int BAD_ARGUMENTS_EXIT_CODE = 2;  //Invalid command-line usage

//The default map to load if none is specified
std::string default_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
std::string default_map_path_OSM = "/cad2/ece297s/public/maps/toronto_canada.osm.bin";


int main(int argc, char** argv) {
    
    std::string map_path;
    if(argc == 1) {
        //Use a default map
        map_path = default_map_path;
    } else if (argc == 2) {
        //Get the map from the command line
        map_path = argv[1];
    } else {
        //Invalid arguments
        std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
        return BAD_ARGUMENTS_EXIT_CODE;
    }
    
    //Load the map and related data structures
    bool load_success = load_map(map_path);

    if(!load_success) {
        std::cerr << "Failed to load map '" << map_path << "'\n";
        return ERROR_EXIT_CODE;
    }
    
    

    
        std::vector<DeliveryInfo> deliveries;
        std::vector<unsigned> depots;
        std::vector<CourierSubpath> result_path;
        float right_turn_penalty;
        float left_turn_penalty;
        float truck_capacity;



        deliveries = {DeliveryInfo(67496, 63896, 61.16725), DeliveryInfo(63711, 13045, 113.45358), DeliveryInfo(98503, 908, 28.82484), DeliveryInfo(105067, 13855, 4.02114), DeliveryInfo(101121, 48380, 104.51654), DeliveryInfo(98503, 75575, 117.75246), DeliveryInfo(63711, 22087, 165.01239), DeliveryInfo(98503, 89579, 3.76336)};
        depots = {61050, 32125, 18579};
        right_turn_penalty = 15.000000000;
        left_turn_penalty = 15.000000000;
        truck_capacity = 8867.809570312;
        result_path = traveling_courier(deliveries, depots, right_turn_penalty, left_turn_penalty, truck_capacity);
        
        
        two_opt(result_path, 2, 6, right_turn_penalty, left_turn_penalty);
        


    
    std::cout << "Successfully loaded map '" << map_path << "'\n";
    draw_map();

    //You can now do something with the map data
    
    while(switchMap){
        close_map();
        switchMap = false;
        
        //Clean-up the map data and related data structures
        std::cout << "Closing map\n"; 
        
        load_success = load_map(newMap);
        
        if(!load_success) {
            return ERROR_EXIT_CODE;
        }
        draw_map();
    }
    
    std::cout << "Closing map\n";
    close_map();
    
    return SUCCESS_EXIT_CODE;
}
