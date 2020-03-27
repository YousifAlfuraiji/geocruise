#include "MAPINFO.h"
#include "m1.h"

//constexpr double EARTH_RADIUS = 6372797.560856;
//constexpr double DEG_TO_RADS = 0.017453292519943295769236907684886;

// Helper function: get the cartesian x value from Longitude.
float x_from_lon(float lon_par){
    return lon_par * DEG_TO_RAD * std::cos(LAT_AVG);
}

// Helper function: get the cartesian y value from Latitude.
float y_from_lat(float lat_par){
    return lat_par * DEG_TO_RAD;
}

// Helper function: get the Longitude from cartesian x.
float lon_from_x(float x){
    return x/(DEG_TO_RAD * std::cos(LAT_AVG));
}

// Helper function: get the Latitude from cartesian y.
float lat_from_y(float y){
    return y / DEG_TO_RAD;
}
