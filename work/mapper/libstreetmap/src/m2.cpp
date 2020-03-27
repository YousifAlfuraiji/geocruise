/*Copyright 2019 Sagar, Yousif and Maytham*/
#include "m1.h"
#include "m3.h"
#include "m2.h"
#include "OSMDatabaseAPI.h"
#include "MAPINFO.h"
#include "FindPathAlgorithm.h"
#include "ezgl/application.hpp"	 
#include "ezgl/graphics.hpp"
#include "control.hpp"
#include<bits/stdc++.h>
#include <string>
#include <cstring>
#include <cmath>
#include <sstream>
#include <set>
#include <map>
#include <gtk/gtk.h>
#include <string.h>

std::string newMap;
bool switchMap;
bool hotelOn;
bool restaurantOn;
bool subwayOn;
bool barOn;
bool bankOn;
bool darkMode = false;
bool flag_arrows = false;

// Variables for direction button AND on dialog response for direction button
int counter = 0;
TurnType turn_dir = TurnType::STRAIGHT;
TurnType previous_turn = TurnType::STRAIGHT;
GObject *window1;
unsigned conseq_segments = 0;
char* direction;

double LAT_AVG = 0;
double max_lat = 0;
double min_lat = 0;
double max_lon = 0;
double min_lon = 0;
double lat = 0;
int lastHighlightedIndex = -1;

int IntersectionPathBegin = -1;
int IntersectionPathEnd = -1;

int selectedIntersectionPathBegin = -1;
int selectedIntersectionPathEnd = -1;
// Has the path stored from the searching algorithm
std::vector<unsigned> found_path;

std::vector<Node> some_que;

//Function declarations
void draw_main_canvas_xy_fixed_world(ezgl::renderer &g);
void act_on_mouse_click(ezgl::application* app,
                    	GdkEventButton* event,
                    	double x, double y);
void act_on_key_press(ezgl::application *application,
                    	GdkEventKey *event,
                    	char *key_name);
void act_on_mouse_move(ezgl::application *application,
                    	GdkEventButton *event,
                    	double x, double y);

void initial_setup(ezgl::application *application);
void find_button(GtkWidget *widget, ezgl::application *application);
void dark_mode_button(GtkWidget *widget, ezgl::application *application);
void help_button(GtkWidget *widget, ezgl::application *application);
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void change_map_button(GtkWidget *widget, ezgl::application *application);
char* turn_type_to_char(TurnType turn);
void directions_button(GtkWidget *widget, ezgl::application *application);
void on_dialog_response_dir(GtkDialog *dialog, gint response_id, gpointer);
void clearHighlight();
void setHighlight(std::string text, ezgl::application *application);
void get_directions(unsigned intersectionId1, unsigned intersectionId2);
void get_directions_helper(std::string text1, std::string text2, ezgl::application *application_p);

void draw_map(){
    // Initialize the application window
    ezgl::application::settings settings;
    settings.main_ui_resource =
            "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";

    ezgl::application application(settings);

    // Set the border of the map, this changes from map to map.
    ezgl::rectangle initial_world({x_from_lon(min_lon), y_from_lat(min_lat)},
                            {x_from_lon(max_lon), y_from_lat(max_lat)});

    application.add_canvas("MainCanvas",
                            draw_main_canvas_xy_fixed_world,
                            initial_world);

    application.run(initial_setup, act_on_mouse_click, act_on_mouse_move, act_on_key_press);	 
}

void initial_setup(ezgl::application *application)
{
    // Create a Test button and link it with test_button callback fn.
    application->create_button("Find", 2, 1, 1, 1, find_button);
    application->create_button("Change Map", 2, 0, 1, 1, change_map_button);
    application->create_button("Dark Mode", 1, 3, 1, 1, dark_mode_button);
    application->create_button("Help", 0, 3, 1, 1, help_button);
    application->create_button("Directions", 2, 2, 1, 1, directions_button);
}

void draw_main_canvas_xy_fixed_world(ezgl::renderer &g){

    switchMap = false;

    // Adding a background color depending on darkMode/normal mode.
    if (!darkMode) g.set_color(242, 242, 242, 255);
    else g.set_color(77, 96, 89, 255);
    g.fill_rectangle(g.get_visible_world());

    // Drawing the border of the map
    g.draw_rectangle({x_from_lon(min_lon), y_from_lat(min_lat)},
                            {x_from_lon(max_lon), y_from_lat(max_lat)});


    // Various scaling numbers for displaying names, icons, etc...
    double scale_factor = (g.get_visible_world().height() / (y_from_lat(max_lat) - y_from_lat(min_lat))) * 100;
    double inverse_scale_factor  = 15 / scale_factor;
    double street_name_factor = 3.0;

    // Change the street name scaling factor for especially large cities like London, Tokyo, or Beijing.
    if(newMap == "/cad2/ece297s/public/maps/london_england.osm.bin" || newMap == "/cad2/ece297s/public/maps/tokyo_japan.osm.bin"
            || newMap == "/cad2/ece297s/public/maps/beijing_china.osm.bin")
    street_name_factor = 1.0;

    // Data structures for keeping track of which names have been printed, and where.
    std::vector<std::string> names_printed;
    std::vector<std::pair<double, double>> name_points;
    std::vector<std::pair<double, double>> feature_points;
    std::vector<std::pair<double, double>> arrow_points;
    std::vector<std::pair<double, double>> poi_points;

    //Draws the lakes
    for (int i = 0; i < getNumFeatures(); ++i) {

    // points will hold the points of the polygon to fill.
    std::vector<ezgl::point2d> points;

    double x1 = 0;
    double y1 = 0;

    double x2 = 0;
    double y2 = 0;

    // For streams, draw a line connecting its points. For everything else, create a vector
    // with all the points, which will be used later in fill_polygon.
    for (int point_count = 0; point_count < getFeaturePointCount(i)-1; ++point_count){
            g.set_line_width(0.5);
            g.set_line_cap(ezgl::line_cap::round);

            LatLon point1 = getFeaturePoint(point_count, i);
            LatLon point2 = getFeaturePoint(point_count + 1, i);

            x1 = point1.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
            y1 = point1.lat() * DEG_TO_RAD;

            x2 = point2.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
            y2 = point2.lat() * DEG_TO_RAD;

            if(getFeatureType(i) == Stream){
            if (!darkMode) g.set_color(170, 218, 255, 255);
            else g.set_color(36, 40, 43, 255);
            g.draw_line({x1, y1}, {x2, y2});
            }

    else
            points.push_back({x1, y1});

    }

    // Can't draw a polygon with 1 point. Check the type of each feature, and fill the polygon with a unique color.
    if(points.size() > 1){
            if(getFeatureType(i) == Lake){
            points.push_back({x2, y2});
            if (!darkMode) g.set_color(170, 218, 255, 255);
            else g.set_color(36, 40, 43, 255);
            g.fill_poly(points);
            }
    }
    points.clear();

    }

    //Draws the islands
    for (int i = 0; i < getNumFeatures(); ++i) {

    // points will hold the points of the polygon to fill.
    std::vector<ezgl::point2d> points;

    double x1 = 0;
    double y1 = 0;

    // For streams, draw a line connecting its points. For everything else, create a vector
    // with all the points, which will be used later in fill_polygon.
    for (int point_count = 0; point_count < getFeaturePointCount(i); ++point_count){
            g.set_line_width(0.5);
            g.set_line_cap(ezgl::line_cap::round);

            if(getFeatureType(i) != Stream){
            LatLon point1 = getFeaturePoint(point_count, i);

            x1 = point1.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
            y1 = point1.lat() * DEG_TO_RAD;

            points.push_back({x1, y1});
            }
    }


    // Can't draw a polygon with 1 point. Check the type of each feature, and fill the polygon with a unique color.
    if(points.size() > 1){
            if(getFeatureType(i) == Island){
            if (!darkMode) g.set_color(242, 242, 242, 255);
            else g.set_color(77, 96, 89, 255);
            g.fill_poly(points);
            }
    }
    points.clear();
    }

    //Draws the other features such as buildings, beaches, etc.
    for (int i = 0; i < getNumFeatures(); ++i) {

    // points will hold the points of the polygon to fill.
    std::vector<ezgl::point2d> points;

    double x1 = 0;
    double y1 = 0;

    // For streams, draw a line connecting its points. For everything else, create a vector
    // with all the points, which will be used later in fill_polygon.
    for (int point_count = 0; point_count < getFeaturePointCount(i); ++point_count){
            g.set_line_width(0.5);
            g.set_line_cap(ezgl::line_cap::round);

            if(getFeatureType(i) != Stream){

            LatLon point1 = getFeaturePoint(point_count, i);

            x1 = point1.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
            y1 = point1.lat() * DEG_TO_RAD;

            points.push_back({x1, y1});

            }
    }

    // Can't draw a polygon with 1 point. Check the type of each feature, and fill the polygon with a unique color.
    if(points.size() > 1){

            double drawBasedOnZoom = 1e-6;

            if(getFeatureType(i) == Park){
            if (!darkMode) g.set_color(192, 236, 174, 255);
            else g.set_color(0, 51, 25, 255);
            g.fill_poly(points);
            }

            else if(getFeatureType(i) == Beach){
            if (!darkMode) g.set_color(255, 235, 161, 255);
            else g.set_color(143, 156, 152, 255);
            g.fill_poly(points);
            }

            else if(getFeatureType(i) == River){
            if (!darkMode) g.set_color(170, 218, 255, 255);
            else g.set_color(36, 40, 43, 255);
            g.fill_poly(points);
            }

            else if(getFeatureType(i) == Building && g.get_visible_world().area() < drawBasedOnZoom ){
            if (!darkMode) g.set_color(ezgl::BISQUE);
            else g.set_color(143, 156, 152, 255);
            g.fill_poly(points);
            }

            else if(getFeatureType(i) == Greenspace){
            if (!darkMode) g.set_color(192, 236, 174, 255);
            else g.set_color(0, 51, 25, 255);
            g.fill_poly(points);
            }

            else if(getFeatureType(i) == Golfcourse){
            if (!darkMode) g.set_color(192, 236, 174, 255);
            else g.set_color(0, 51, 25, 255);
            g.fill_poly(points);
            }    	 

    }
    points.clear();
    }


    //Drawing street segments.
    for (int i = 0; i < getNumStreetSegments(); ++i) {
    LatLon point1 = getIntersectionPosition(getInfoStreetSegment(i).from);
    LatLon point2 = getIntersectionPosition(getInfoStreetSegment(i).to);

    // Storing the points (in x,y) of  the starting intersection and ending intersection.
    double x1 = point1.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
    double y1 = point1.lat() * DEG_TO_RAD;
    double x2 = point2.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
    double y2 = point2.lat() * DEG_TO_RAD;

    OSMID osmid = getInfoStreetSegment(i).wayOSMID;
    auto find = MAPDATA.way_OSM.find(osmid);

    // Iterate through all the types of roads, and set the text font, line width and drawBasedOnZoom accordingly.
    for (unsigned j = 0; j < getTagCount(getWayByIndex(find->second)); j++) {
            //Access the ways related to the street segment
            std::string value, key;
            key = getTagPair(getWayByIndex(find->second), j).first;
            value = getTagPair(getWayByIndex(find->second), j).second;

            //Condition for if the segment will be visible on the canvas and for checking the key
            if ((g.get_visible_world().contains(x1, y1) || g.get_visible_world().contains(x2, y2)) && (key == "highway")) {
            //Defining a value to determine the zoom level of when items should appear
            double drawBasedOnZoom;

            //Conditions for checking the value
            if (((value == "motorway") || (value == "motorway_link") || (value == "trunk") || (value == "trunk_link")) && getInfoStreetSegment(i).speedLimit >= 70) {
                    //Sets the characteristics of the item to be drawn
                    if (!darkMode) g.set_color(253, 225, 136, 255);
                    else g.set_color(127, 141, 137, 255);
                    g.set_line_width(inverse_scale_factor*4);
                    g.set_font_size(inverse_scale_factor*1.2);
                    drawBasedOnZoom = 1;
            }
            else if ((value == "primary") || (value == "secondary") || (value == "tertiary")) {
                    //Sets the characteristics of the item to be drawn
                    if (!darkMode) g.set_color(ezgl::WHITE);
                    else g.set_color(143, 156, 152, 255);
                    g.set_line_width(inverse_scale_factor);
                    g.set_line_cap(ezgl::line_cap::round);
                    g.set_font_size(inverse_scale_factor);
                    drawBasedOnZoom = 1e-5*5;
            }
            else {
                    //Sets the characteristics of the item to be drawn
                    if (!darkMode) g.set_color(ezgl::WHITE);
                    else g.set_color(143, 156, 152, 255);
                    g.set_line_width(inverse_scale_factor);
                    g.set_line_cap(ezgl::line_cap::round);
                    g.set_font_size(inverse_scale_factor);
                    drawBasedOnZoom = 1e-6;
            }

            // Color the intersection if the user clicks on (or near) it.
            if(MAPDATA.streetsegments_highlighter[i]){
                    g.set_color(ezgl::RED);
                    drawBasedOnZoom = 1;
            }

            // Only display the street if zoomed in enough, otherwise the map will look cluttered.
            if (g.get_visible_world().area() < drawBasedOnZoom) {
                    // Draw a straight street segment.
                    if (getInfoStreetSegment(i).curvePointCount == 0)
                    g.draw_line({x1, y1}, {x2, y2});

                    // Draw a curved street segment.
                    if(getInfoStreetSegment(i).curvePointCount > 0){

                    LatLon first_curve_point = getStreetSegmentCurvePoint(0, i);
                    double x = first_curve_point.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
                    double y = first_curve_point.lat() * DEG_TO_RAD;

                    //Draw a line from first intersection to first curve point.
                    g.draw_line({x1, y1}, {x, y});

                    //Draw lines between all the curve points.
                    int ic = 0;
                    for(ic = 0; ic < getInfoStreetSegment(i).curvePointCount-1; ic++){
                            LatLon curve_point1 = getStreetSegmentCurvePoint(ic, i);
                            LatLon curve_point2 = getStreetSegmentCurvePoint(ic+1, i);

                            double x1_c = curve_point1.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
                            double y1_c = curve_point1.lat() * DEG_TO_RAD;

                            double x2_c = curve_point2.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
                            double y2_c = curve_point2.lat() * DEG_TO_RAD;

                            g.draw_line({x1_c, y1_c}, {x2_c, y2_c});
                            g.set_text_rotation(std::atan((y2-y1)/(x2-x1)) /DEG_TO_RAD );
                    }

                    LatLon curve_point = getStreetSegmentCurvePoint(ic, i);
                    double x2_l = curve_point.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
                    double y2_l = curve_point.lat() * DEG_TO_RAD;

                    //Draw line from last curve point to last intersection.
                    g.draw_line({x2_l, y2_l}, {x2, y2});
                    }
            }

            }
    }
    }


    //Drawing intersections
    for (unsigned i = 0; i < MAPDATA.intersections.size(); ++i) {
    float x = x_from_lon(MAPDATA.intersections[i].position.lon());
    float y = y_from_lat(MAPDATA.intersections[i].position.lat());


    if (MAPDATA.intersections[i].highlight){
            g.set_color(ezgl::RED);
            g.fill_arc({x, y}, 0.000001, 0, 360);
    }
    else {
            if (!darkMode) g.set_color(ezgl::WHITE);
            else g.set_color(143, 156, 152, 255);
    }
    }




    // Display the street names after the streets, so the street names appear on top of the street.
    for (int i = 0; i < getNumStreetSegments(); ++i) {
    LatLon point1 = getIntersectionPosition(getInfoStreetSegment(i).from);
    LatLon point2 = getIntersectionPosition(getInfoStreetSegment(i).to);

    double x1 = point1.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
    double y1 = point1.lat() * DEG_TO_RAD;
    double x2 = point2.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
    double y2 = point2.lat() * DEG_TO_RAD;

    bool already_printed = false;
    // Only display the name if the street is inside the visible map world.
    if (g.get_visible_world().contains((x1 + (x2-x1) / 2), (y1 + (y2-y1) / 2))) {

            // Check if the name has already been printed on the visible map world.
            auto it = std::find(names_printed.begin(), names_printed.end(), getStreetName(getInfoStreetSegment(i).streetID));

            if (it != names_printed.end())
            already_printed = true;

            else
            already_printed = false;
    }

    // Only print the name if the point where the name will be printed (the midpoint of the street segment) is inside
    // the visible world.
    // Checking for: appropriate zoom, unknown road names, street segment length and already printed names.
    // Reducing clutter significantly.
    if((g.get_visible_world().contains((x1 + (x2-x1) / 2), (y1 + (y2-y1) / 2))) && (g.get_visible_world().area() < 0.0000001) &&
            ((getStreetName(getInfoStreetSegment(i).streetID)) != "<unknown>") &&
            ((find_street_segment_length(i) / scale_factor)  > 17 ) && (!already_printed)){

            // Print the name of a straight street segment.
            if(getInfoStreetSegment(i).curvePointCount == 0){
            g.set_text_rotation(0);
            g.set_text_rotation(std::atan((y2-y1)/(x2-x1)) /DEG_TO_RAD);
            double x_mid = x1+(x2-x1)/2.0;
            double y_mid = y1+(y2-y1)/2.0;

            // Check if there is another street name that's too close.
            bool too_close = false;
            for(unsigned p = 0; p < name_points.size(); ++p){
                    if(std::sqrt((y_mid - name_points[p].second)*(y_mid - name_points[p].second) +
                            (x_mid - name_points[p].first)*(x_mid - name_points[p].first)) < g.get_visible_world().height()/street_name_factor)
                    too_close = true;
            }
            // Print the name, and add it to a vector that keeps track of what names have been printed.
            if(!too_close){
                    if (!darkMode) g.set_color(ezgl::BLACK);
                    else g.set_color(ezgl::WHITE);

                    g.draw_text({x_mid, y_mid},
                            getStreetName(getInfoStreetSegment(i).streetID));
                    names_printed.push_back(getStreetName(getInfoStreetSegment(i).streetID));
                    name_points.push_back(std::make_pair(x1+(x2-x1)/2, y1+(y2-y1)/2));
            }

            // Print the arrow for one way straight streets.
            if(getInfoStreetSegment(i).oneWay){

                    // Check if there is another arrow that's too close.
                    bool too_close_arrow = false;
                    for(unsigned p = 0; p < arrow_points.size(); ++p){
                    if(std::sqrt((y_mid - arrow_points[p].second)*(y_mid - arrow_points[p].second) +
                            (x_mid - arrow_points[p].first)*(x_mid - arrow_points[p].first)) < g.get_visible_world().height()/(street_name_factor*5))
                            too_close_arrow = true;
                    }

                    // Print the name, and add it to a vector that keeps track of what names have been printed.
                    if(!too_close_arrow){
                    if (!darkMode) g.set_color(ezgl::GREY_55);
                    else g.set_color(255, 255, 255, 255);

                    if(x2 > x1)
                            g.draw_text({x_mid, y_mid},
                            ">");
                    else
                            g.draw_text({x_mid, y_mid},
                            "<");
                    arrow_points.push_back(std::make_pair(x_mid, y_mid));
                    }
            }
            }

            // Print the name of a curved street.
            else if (getInfoStreetSegment(i).curvePointCount > 2){
            LatLon curve_point_text1 = getStreetSegmentCurvePoint(getInfoStreetSegment(i).curvePointCount/2, i);
            LatLon curve_point_text2 = getStreetSegmentCurvePoint(getInfoStreetSegment(i).curvePointCount/2 + 1, i);


            double curve_text_x1 = curve_point_text1.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
            double curve_text_y1 = curve_point_text1.lat() * DEG_TO_RAD;

            double curve_text_x2 = curve_point_text2.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
            double curve_text_y2 = curve_point_text2.lat() * DEG_TO_RAD;

            g.set_text_rotation(std::atan((curve_text_y2 - curve_text_y1)/(curve_text_x2 - curve_text_x1)) /DEG_TO_RAD);              	 

            double x_mid = curve_text_x1 + (curve_text_x2 - curve_text_x1)/2;
            double y_mid = curve_text_y1 + (curve_text_y2 - curve_text_y1)/2;

            // Check if there is another street name that's too close.
            bool too_close = false;
            for(unsigned p = 0; p < name_points.size(); ++p){
                    if(std::sqrt((y_mid - name_points[p].second)*(y_mid - name_points[p].second) +
                            (x_mid - name_points[p].first)*(x_mid - name_points[p].first)) < g.get_visible_world().height()/street_name_factor)
                    too_close = true;
            }

            // Print the name, and add it to a vector that keeps track of what names have been printed.
            if(!too_close){
                    if (!darkMode) g.set_color(ezgl::BLACK);
                    else g.set_color(ezgl::WHITE);
                    g.draw_text({x_mid, y_mid},
                            getStreetName(getInfoStreetSegment(i).streetID));
                    names_printed.push_back(getStreetName(getInfoStreetSegment(i).streetID));
                    name_points.push_back(std::make_pair(x_mid, y_mid));
            }

            // Print the arrow for one way curved streets.
            if(getInfoStreetSegment(i).oneWay){

                    // Check if there is another arrow that's too close.
                    bool too_close_arrow = false;
                    for(unsigned p = 0; p < arrow_points.size(); ++p){
                    if(std::sqrt((y_mid - arrow_points[p].second)*(y_mid - arrow_points[p].second) +
                            (x_mid - arrow_points[p].first)*(x_mid - arrow_points[p].first)) < g.get_visible_world().height()/(street_name_factor*5))
                            too_close_arrow = true;
                    }

                    // Print the name, and add it to a vector that keeps track of what names have been printed.
                    if(!too_close_arrow){
                    if (!darkMode) g.set_color(ezgl::GREY_55);
                    else g.set_color(255, 255, 255, 255);

                    if(x2 > x1)
                            g.draw_text({x_mid, y_mid},
                            ">");
                    else
                            g.draw_text({x_mid, y_mid},
                            "<");
                    arrow_points.push_back(std::make_pair(x_mid, y_mid));
                    }
            }
            }
    }


    }
    // Clear the vectors.
    names_printed.clear();
    arrow_points.clear();

    // Displaying the POIs
    for (int i = 0; i < getNumPointsOfInterest(); ++i) {
    float x = x_from_lon(getPointOfInterestPosition(i).lon());
    float y = y_from_lat(getPointOfInterestPosition(i).lat());

    //Obtain the OSM id
    OSMID osmid = getPointOfInterestOSMNodeID(i);
    auto find = MAPDATA.node_OSM.find(osmid);

    for (unsigned j = 0; j < getTagCount(getNodeByIndex(find->second)); ++j) {
            //Access the nodes related to the POI
            std::string value, key;
            key = getTagPair(getNodeByIndex(find->second), j).first;
            value = getTagPair(getNodeByIndex(find->second), j).second;
            //Condition for it the POI is within the canvas range and for accessing the key
            if ((g.get_visible_world().contains(x, y)) && (key == "amenity")) {
            //Value that determines when to draw an item based on the zoom
            double drawBasedOnZoom;


            //Conditions for accessing the value
            if (value == "restaurant" || value == "cafe") {
                    drawBasedOnZoom = 0.0000001;

                    // Check if there is another street name or another POI name or another that's too close.
                    bool too_close = false;
                    for(unsigned p = 0; p < poi_points.size(); ++p){
                    if(std::sqrt((y - poi_points[p].second)*(y - poi_points[p].second) +
                            (x - poi_points[p].first)*(x - poi_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }
                    for(unsigned p = 0; p < name_points.size(); ++p){
                    if(std::sqrt((y - name_points[p].second)*(y - name_points[p].second) +
                            (x - name_points[p].first)*(x - name_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    } 	 
                    for(unsigned p = 0; p < feature_points.size(); ++p){
                    if(std::sqrt((y - feature_points[p].second)*(y - feature_points[p].second) +
                            (x - feature_points[p].first)*(x - feature_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }

                    // Display the POI icon if the user has zoomed in enough and there's nothing too close.
                    if ((g.get_visible_world().area() < drawBasedOnZoom) && (!too_close) && restaurantOn) {
                    //Draws the restaurants icon
                    ezgl::surface* image = g.load_png("/homes/c/chadhas7/ece297/work/mapper/libstreetmap/resources/restaurant.png");
                    g.draw_surface(image, {x, y});
                    g.free_surface(image);
                    }

                    // Print the name only if there is space (there's nothing that's too close), and the user
                    // has zoomed in enough.
                    if((!too_close) && (g.get_visible_world().area() < drawBasedOnZoom/2)){
                    //Sets the characteristics and draws the restaurants
                    poi_points.push_back(std::make_pair(x, y));
                    g.set_text_rotation(0);
                    if (!darkMode) g.set_color(50, 50, 50, 255);
                    else g.set_color(ezgl::WHITE);
                    g.set_font_size(inverse_scale_factor/2);
                    g.draw_text({x, y+0.0000004}, getPointOfInterestName(i));
                    }
            }

            if (value == "bank" || value == "atm") {
                    drawBasedOnZoom = 0.0000001;

                    // Check if there is another street name OR another POI name that's too close.
                    bool too_close = false;
                    for(unsigned p = 0; p < poi_points.size(); ++p){
                    if(std::sqrt((y - poi_points[p].second)*(y - poi_points[p].second) +
                            (x - poi_points[p].first)*(x - poi_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }

                    for(unsigned p = 0; p < name_points.size(); ++p){
                    if(std::sqrt((y - name_points[p].second)*(y - name_points[p].second) +
                            (x - name_points[p].first)*(x - name_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }

                    for(unsigned p = 0; p < feature_points.size(); ++p){
                    if(std::sqrt((y - feature_points[p].second)*(y - feature_points[p].second) +
                            (x - feature_points[p].first)*(x - feature_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }

                    // Display the POI icon if the user has zoomed in enough and there's nothing too close.
                    if ((g.get_visible_world().area() < drawBasedOnZoom) && (!too_close) && bankOn) {
                    //Draws the bank icon
                    ezgl::surface* image = g.load_png("/homes/c/chadhas7/ece297/work/mapper/libstreetmap/resources/bank.png");
                    g.draw_surface(image, {x, y});
                    g.free_surface(image);
                    }

                    // Print the name only if there is space (there's nothing that's too close), and the user
                    // has zoomed in enough.
                    if((!too_close) && (g.get_visible_world().area() < drawBasedOnZoom/2)){
                    //Sets the characteristics and draws the banks
                    poi_points.push_back(std::make_pair(x, y));
                    g.set_text_rotation(0);
                    if (!darkMode) g.set_color(50, 50, 50, 255);
                    else g.set_color(ezgl::WHITE);
                    g.set_font_size(inverse_scale_factor/2);
                    g.draw_text({x, y+0.0000004}, getPointOfInterestName(i));
                    }
            }

            if (value == "nightclub" || value == "stripclub" || value == "bar") {
                    drawBasedOnZoom = 0.0000001;

                    // Check if there is another street name OR another POI name that's too close.
                    bool too_close = false;
                    for(unsigned p = 0; p < poi_points.size(); ++p){
                    if(std::sqrt((y - poi_points[p].second)*(y - poi_points[p].second) +
                            (x - poi_points[p].first)*(x - poi_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }

                    for(unsigned p = 0; p < name_points.size(); ++p){
                    if(std::sqrt((y - name_points[p].second)*(y - name_points[p].second) +
                            (x - name_points[p].first)*(x - name_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    } 	 

                    for(unsigned p = 0; p < feature_points.size(); ++p){
                    if(std::sqrt((y - feature_points[p].second)*(y - feature_points[p].second) +
                            (x - feature_points[p].first)*(x - feature_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }

                    // Display the POI icon if the user has zoomed in enough and there's nothing too close.
                    if ((g.get_visible_world().area() < drawBasedOnZoom) && (!too_close) && barOn) {
                    //Draws the bar icon
                    ezgl::surface* image = g.load_png("/homes/c/chadhas7/ece297/work/mapper/libstreetmap/resources/bar.png");
                    g.draw_surface(image, {x, y});
                    g.free_surface(image);
                    }

                    // Print the name only if there is space (there's nothing that's too close), and the user
                    // has zoomed in enough.
                    if((!too_close) && (g.get_visible_world().area() < drawBasedOnZoom/2)){
                    //Sets the characteristics and draws the bars
                    poi_points.push_back(std::make_pair(x, y));
                    g.set_text_rotation(0);
                    if (!darkMode) g.set_color(50, 50, 50, 255);
                    else g.set_color(ezgl::WHITE);
                    g.set_font_size(inverse_scale_factor/2);
                    g.draw_text({x, y+0.0000004}, getPointOfInterestName(i));
                    }
            }

            // All other POIs.
            else {
                    drawBasedOnZoom = 0.00000001;

                    // Check if there is another street name OR another POI name that's too close.
                    bool too_close = false;
                    for(unsigned p = 0; p < poi_points.size(); ++p){
                    if(std::sqrt((y - poi_points[p].second)*(y - poi_points[p].second) +
                            (x - poi_points[p].first)*(x - poi_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }

                    for(unsigned p = 0; p < name_points.size(); ++p){
                    if(std::sqrt((y - name_points[p].second)*(y - name_points[p].second) +
                            (x - name_points[p].first)*(x - name_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    } 	 

                    for(unsigned p = 0; p < feature_points.size(); ++p){
                    if(std::sqrt((y - feature_points[p].second)*(y - feature_points[p].second) +
                            (x - feature_points[p].first)*(x - feature_points[p].first)) < g.get_visible_world().height()/(street_name_factor*3))
                            too_close = true;
                    }

                    // Display the POI icon if the user has zoomed in enough and there's nothing too close.
                    if ((g.get_visible_world().area() < drawBasedOnZoom) && (!too_close)) {
                    g.set_color(ezgl::FIRE_BRICK);
                    g.fill_rectangle({x-0.0000002, y-0.0000002}, {x+0.0000002, y+0.0000002});

                    // Print the name only if there is space (there's nothing that's too close), and the user
                    // has zoomed in enough.
                    if((!too_close) && (g.get_visible_world().area() < drawBasedOnZoom/2)){
                            poi_points.push_back(std::make_pair(x, y));
                            g.set_text_rotation(0);
                            if (!darkMode) g.set_color(50, 50, 50, 255);
                            else g.set_color(ezgl::WHITE);
                            g.set_font_size(inverse_scale_factor/2);
                            g.draw_text({x, y+0.0000004}, getPointOfInterestName(i));
                    }
                    }
            }
            }
    }
    }


    //Goes through the features and draws POIs that are buildings
    for (int i = 0; i < getNumFeatures(); ++i) {

    // points will hold the points of the polygon to fill.
    double x1 = 0;
    double y1 = 0;

    // For streams, draw a line connecting its points. For everything else, create a vector
    // with all the points, which will be used later in fill_polygon.
    for (int point_count = 0; point_count < getFeaturePointCount(i)-1; ++point_count){
            g.set_line_width(0.5);
            g.set_line_cap(ezgl::line_cap::round);

            LatLon point1 = getFeaturePoint(point_count, i);

            x1 = point1.lon() * DEG_TO_RAD * std::cos(LAT_AVG);
            y1 = point1.lat() * DEG_TO_RAD;
    }

    //Obtain the OSM id
    OSMID osmid = getFeatureOSMID(i);
    auto find = MAPDATA.way_OSM.find(osmid);

    // Iterate through all the feature types, check for the key of each feature and display information accordingly.
    for (unsigned j = 0; j < getTagCount(getWayByIndex(find->second)); ++j) {
            //Accessing the ways that relate to the feature
            std::string value, key;
            key = getTagPair(getWayByIndex(find->second), j).first;
            value = getTagPair(getWayByIndex(find->second), j).second;

            //Condition for accessing the key and checking in-bounds
            if ((g.get_visible_world().contains(x1, y1)) && (key == "building")) {
            //Value for drawing a feature on a specific zoom
            double drawBasedOnZoom;

            //Conditions for accessing the value
            if (value == "hotel") {
                    drawBasedOnZoom = 0.0000001;

                    // If there's nothing too close, and the user has zoomed in enough, then display the icon, Reducing clutter.
                    if ((g.get_visible_world().area() < drawBasedOnZoom) && hotelOn) {
                    //Draws the hotel icon
                    ezgl::surface* image = g.load_png("/homes/c/chadhas7/ece297/work/mapper/libstreetmap/resources/hotel.png");
                    g.draw_surface(image, {x1, y1});
                    g.free_surface(image);
                    }

                    if((g.get_visible_world().area() < drawBasedOnZoom/2) && (getFeatureName(i) != "<noname>")){
                    //Sets the characteristics and draws the hotel
                    feature_points.push_back(std::make_pair(x1, y1));
                    g.set_text_rotation(0);
                    if (!darkMode) g.set_color(50, 50, 50, 255);
                    else g.set_color(ezgl::WHITE);
                    g.set_font_size(inverse_scale_factor/2);
                    g.draw_text({x1, y1+0.0000004}, getFeatureName(i));
                    }
            }

            if (value == "train_station") {
                    drawBasedOnZoom = 0.0000001;

                    // If there's nothing too close, and the user has zoomed in enough, then display the icon, Reducing clutter.
                    if ((g.get_visible_world().area() < drawBasedOnZoom) && subwayOn) {
                    //Draws the subway icon
                    ezgl::surface* image = g.load_png("/homes/c/chadhas7/ece297/work/mapper/libstreetmap/resources/metro.png");
                    g.draw_surface(image, {x1, y1});
                    g.free_surface(image);
                    }

                    if((g.get_visible_world().area() < drawBasedOnZoom/2) && (getFeatureName(i) != "<noname>")){
                    //Sets the characteristics and draws the subway stations
                    feature_points.push_back(std::make_pair(x1, y1));
                    g.set_text_rotation(0);
                    if (!darkMode) g.set_color(50, 50, 50, 255);
                    else g.set_color(ezgl::WHITE);
                    g.set_font_size(inverse_scale_factor/2);
                    g.draw_text({x1, y1+0.0000004}, getFeatureName(i));
                    }

            }
            }
    }
    }

    if(selectedIntersectionPathBegin != -1){

        ezgl::point2d beginning_point = {x_from_lon(getIntersectionPosition(selectedIntersectionPathBegin).lon()), 
                                            y_from_lat(getIntersectionPosition(selectedIntersectionPathBegin).lat())};

        if(darkMode) g.set_color(ezgl::YELLOW);
        else g.set_color(ezgl::BLACK);
        g.set_text_rotation(0);
        g.set_font_size(10);
        g.draw_text(beginning_point, "Source");
    }
    if(selectedIntersectionPathEnd != -1){ 

        ezgl::point2d end_point = {x_from_lon(getIntersectionPosition(selectedIntersectionPathEnd).lon()), 
                                            y_from_lat(getIntersectionPosition(selectedIntersectionPathEnd).lat())};

        if(darkMode) g.set_color(ezgl::YELLOW);
        else g.set_color(ezgl::BLACK);
        g.set_text_rotation(0);
        g.set_font_size(10);
        g.draw_text(end_point, "Destination");
    }




    // Clear the vectors for POI points and street names.
    poi_points.clear();
    name_points.clear();
    feature_points.clear();
}

//Function to consider the actions of the program upon a mouse click
void act_on_mouse_click(ezgl::application* app,
                    	GdkEventButton* event,
                    	double x, double y) {
    
	if(event->button == 1) return;
    
 	// Need proper scaling of the screen
	double screen_height = app->get_canvas("MainCanvas")->get_camera().get_initial_world().height();
	double screen_width = app->get_canvas("MainCanvas")->get_camera().get_initial_world().width();
	double screen_scale = screen_height / screen_width;
    
    
	std::cout << "Mouse clicked at (" << x << "," << y << ")\n";
    
	clearHighlight();
       	 
	LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));
	unsigned id = find_closest_intersection(pos);
    
	MAPDATA.intersections[id].highlight = true;
    
	if(event->button == 3){
            if(IntersectionPathBegin == -1){
                    IntersectionPathBegin = id;
                    selectedIntersectionPathBegin = id;
                    selectedIntersectionPathEnd = -1;
                    flag_arrows = false;
                    found_path.clear();
            }
            else{
                    IntersectionPathEnd = id;
                    get_directions(IntersectionPathBegin, IntersectionPathEnd);
                    MAPDATA.intersections[IntersectionPathBegin].highlight = true;
                    MAPDATA.intersections[IntersectionPathEnd].highlight = true;
                    IntersectionPathBegin = -1;
                    IntersectionPathEnd = -1;

                    flag_arrows = true;

                    if(found_path.size() > 1){
                    LatLon intersectionPosition_h = getIntersectionPosition(getInfoStreetSegment(found_path[found_path.size()/2]).from);
                    ezgl::point2d intersection2dpoint_h = {x_from_lon(intersectionPosition_h.lon()), y_from_lat(intersectionPosition_h.lat())};

                    int size = found_path.size();
                    //std::cout << screen_scale << std::endl;
                    ezgl::zoom_fit(app->get_canvas("MainCanvas"),
                            ezgl::rectangle(operator+(intersection2dpoint_h,{-0.000008*size,-0.000008*size*screen_scale}),
                                    operator+(intersection2dpoint_h, {0.000008*size,0.000008*size*screen_scale})));
                    }
                    app->refresh_drawing();
            }
            return;
	}
        
	std::cout << "Closest Intersection: " << MAPDATA.intersections[id].name << "\n";
	std::cout << "Closest Intersection ID: " << id << "\n";
	app->update_message(MAPDATA.intersections[id].name);

	LatLon intersectionPosition = MAPDATA.intersections[id].position;

	ezgl::point2d intersection2dpoint = {x_from_lon(intersectionPosition.lon()), y_from_lat(intersectionPosition.lat())};

	// Zoom onto the found intersection
	if(event->button == 2){
    	ezgl::zoom_fit(app->get_canvas("MainCanvas"),
            	ezgl::rectangle(operator+(intersection2dpoint,{-0.00005,-0.00005*screen_scale}), operator+(intersection2dpoint, {0.00005,0.00005*screen_scale})));
	}
        
	app->refresh_drawing();
}


// Function to search for a street name or intersection
void find_button(GtkWidget *, ezgl::application *application_p){
    
    // Clear Previously highlighted roads or intersections if they exist
    clearHighlight();

    //Obtains the text from the text box and converts it to a string
    GtkEntry* text_entry = (GtkEntry*) application_p->get_object("TextInput");
    std::string text = (std::string) gtk_entry_get_text(text_entry);

    GtkEntry* text_entry2 = (GtkEntry*) application_p->get_object("TextInput2");
    std::string text2 = (std::string) gtk_entry_get_text(text_entry2);

    if(text.empty() && text2.empty()){
        application_p->update_message("No text inputted");
        std::cout << "No text inputted" << std::endl;
        return;
    }
    else if (!text.empty() && text2.empty()) {
        // Highlight all possible intersections or streets segments
        setHighlight(text, application_p);
    }
    else if (text.empty() && !text2.empty()) {
        // Highlight all possible intersections or streets segments
        setHighlight(text2, application_p);
    }
    else {
        get_directions_helper(text, text2, application_p);
    }
}



    
// Opens a pop up window displaying text that help with keyboard shortcuts
void help_button(GtkWidget *, ezgl::application *application){

    application->update_message("Help Menu Opened");
    application->refresh_drawing();

    GObject *window; // the parent window over which to add the dialog
    GtkWidget *content_area; // the content area of the dialog
    GtkWidget *label; // the label we will create to display a message in the content area
    GtkWidget *dialog; // the dialog box we will create

    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());

    // Create the dialog window.
    // Modal windows prevent interaction with other windows in the same application
    dialog = gtk_dialog_new_with_buttons(
    "The Interface",
    (GtkWindow*) window,
    GTK_DIALOG_MODAL,
    ("OK"),
    GTK_RESPONSE_ACCEPT,
    NULL
    );

    // Create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // The main 'Help' text
    label = gtk_label_new(
            "\t\t\t\t\t\t\t\tChange Map\n"
            "\n"
            "Click on the drop-down menu, select the new map, then click 'Change Map'.\n"
            "\n"
            "\t\t\t\t\t\t\t\tFind\n"
            "\n"
            "Type in two streets to find their intersection and highlight it in either of the search boxes.\n"
            "Type in a street name to find and highlight it.\n"
            "Type in two intersection names in the search boxes to find the shortest path.\n"
            "\n"
            "\t\t\t\t\t\t\t\tDark Mode\n"
            "\n"
            "Click 'Dark Mode' for darker map colours.\n"
            "\n"
            "\t\t\t\t\t\t\t\tToggles\n"
            "\n"
            "Click on the toggle switch next to the icon you want to display.\n"
            "\n"
            "\t\t\t\t\t\t\t\tDirections\n"
            "\n"
            "Right click on two intersections to find the shortest path.\n"
            "Click on 'Directions' for directions.\n"
            "\n"
            "\t\t\t\t\t\t\t\tKeyboard Shortcuts\n"
            "\n"
            "'*' to turn on keyboard shortcuts.\n"
            "'q' to quit the application.\n"
            "\n"
            "'wasd' or arrow keys to pan.\n"
            "'+' to zoom in.\n"
            "'-' to zoom out.\n"
            "\n"
            "'c' to change map.\n"
            "'Enter' to search.\n"
            "\n"
            "'1' to toggle the hotels filter.\n"
            "'2' to toggle the restaurants filter.\n"
            "'3' to toggle the subway filter.\n"
            "'4' to toggle the bars and nightclubs filter.\n"
            "'5' to toggle the bank and atm filter.\n"
            "\n"
            "\n"

            );


    gtk_container_add(GTK_CONTAINER(content_area), label);

    // The main purpose of this is to show dialog’s child widget, label
    gtk_widget_show_all(dialog);

    g_signal_connect(
    GTK_DIALOG(dialog),
    "response",
    G_CALLBACK(on_dialog_response),
    NULL
    );
}

// Needed to supplement the help button pop up
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer)
{
    switch(response_id) {
    case GTK_RESPONSE_ACCEPT:
            std::cout << "Pressed OK";
            break;
    case GTK_RESPONSE_DELETE_EVENT:
            std::cout << "Exit Menu";
            break;
    default:
            std::cout << "UNKNOWN ";
            break;
    }

    // This will cause the dialog to be destroyed and close
    // without this line the dialog remains open unless the
    // response_id is GTK_RESPONSE_DELETE_EVENT which
    // automatically closes the dialog without the following line.
    gtk_widget_destroy(GTK_WIDGET (dialog));
}

void act_on_key_press(ezgl::application *application, GdkEventKey *, char *key_name)
{
    // Create a switch that turns on keyboard shortcuts
    GtkSwitch* switch_entry = (GtkSwitch*) application->get_object("KeyboardSwitch");

    GtkEntry* text_entry = (GtkEntry*) application->get_object("TextInput");
    std::string text = (std::string) gtk_entry_get_text(text_entry);

    std::string key_name_string = (std::string)key_name;

    // Different functionalities for selected keyboard keys
    if (key_name_string == "asterisk" && gtk_switch_get_active(switch_entry)){
    gtk_switch_set_active(switch_entry, false);
    application->update_message("Keyboard Shortcuts Off");
    }
    else if (key_name_string == "asterisk" && !gtk_switch_get_active(switch_entry)){
    gtk_switch_set_active(switch_entry, true);
    application->update_message("Keyboard Shortcuts On");
    }

    if (gtk_switch_get_active(switch_entry)) {

    // These pan the screen
    if (key_name_string == "Right" || key_name_string == "d"){
            ezgl::translate_right(application->get_canvas("MainCanvas"), (application->get_canvas("MainCanvas"))->width()*0.01);
            application->update_message("Panning");
    }
    if (key_name_string == "Left" || key_name_string == "a"){
            ezgl::translate_left(application->get_canvas("MainCanvas"), (application->get_canvas("MainCanvas"))->width()*0.01);
            application->update_message("Panning");
    }
    if (key_name_string == "Up" || key_name_string == "w"){
            ezgl::translate_up(application->get_canvas("MainCanvas"), (application->get_canvas("MainCanvas"))->height()*0.01);
            application->update_message("Panning");
    }
    if (key_name_string == "Down" || key_name_string == "s"){
            ezgl::translate_down(application->get_canvas("MainCanvas"), (application->get_canvas("MainCanvas"))->height()*0.01);
            application->update_message("Panning");
    }

    // These zoom in onto the screen
    if (key_name_string == "plus"){
            ezgl::zoom_in(application->get_canvas("MainCanvas"), 2);
            application->update_message("Zooming");
    }
    if (key_name_string == "minus"){
            ezgl::zoom_out(application->get_canvas("MainCanvas"), 2);
            application->update_message("Zooming");
    }

    // Finds function caller
    if (key_name_string == "Return"){
            clearHighlight();
            setHighlight(text, application);
            application->update_message("Found All possible roads and intersections.");
    }
    // Change the map
    if (key_name_string == "c") {
            //Obtains the text from the drop down menu labeled as "ChangeMap' and converts it to a string
            GtkComboBoxText* change_map_entry = (GtkComboBoxText*) application->get_object("ChangeMap");
            std::string map_text = (std::string) gtk_combo_box_text_get_active_text(change_map_entry);
            std::string map_file_name;

            //Selects the file path name based upon the text from the drop down menu
            if (map_text == "Beijing, China")
            map_file_name = "beijing_china";
            else if (map_text == "Cairo, Egypt")
            map_file_name = "cairo_egypt";
            else if (map_text == "Cape Town, South Africa")
            map_file_name = "cape-town_south-africa";
            else if (map_text == "Golden Horseshoe, Canada")
            map_file_name = "golden-horseshoe_canada";
            else if (map_text == "Hamilton, Canada")
            map_file_name = "hamilton_canada";
            else if (map_text == "Hong Kong, China")
            map_file_name = "hong-kong_china";
            else if (map_text == "Iceland")
            map_file_name = "iceland";
            else if (map_text == "Interlaken, Switzerland")
            map_file_name = "interlaken_switzerland";
            else if (map_text == "London, England")
            map_file_name = "london_england";
            else if (map_text == "Moscow, Russia")
            map_file_name = "moscow_russia";
            else if (map_text == "New Delhi, India")
            map_file_name = "new-delhi_india";
            else if (map_text == "New York, USA")
            map_file_name = "new-york_usa";
            else if (map_text == "Rio de Janeiro, Brazil")
            map_file_name = "rio-de-janeiro_brazil";
            else if (map_text == "Saint Helena")
            map_file_name = "saint-helena";
            else if (map_text == "Singapore")
            map_file_name = "singapore";
            else if (map_text == "Sydney, Australia")
            map_file_name = "sydney_australia";
            else if (map_text == "Tehran, Iran")
            map_file_name = "tehran_iran";
            else if (map_text == "Tokyo, Japan")
            map_file_name = "tokyo_japan";
            else if (map_text == "Toronto, Canada")
            map_file_name = "toronto_canada";
            else {
            std::cout << "Invalid map. Please try again\n";
            application->update_message("Invalid map. Please try again");
            return;
            }

            //Outputs which map is loaded to the room in a message
            std::cout << map_text << std::endl;
            std::stringstream ss;
            ss << map_text << " Map Loaded";
            std::string map_loaded_text = ss.str();

            //Combines the obtained data with the different file names
            std::stringstream newSS;
            newSS << "/cad2/ece297s/public/maps/" << map_file_name << ".streets.bin";
            newMap = newSS.str();

            newMapOSM = "/cad2/ece297s/public/maps/" + map_file_name + ".osm.bin";

            //Quits the application and turns on the global boolean
            application->quit();
            switchMap = true;
            return;
    }

    if (key_name_string == "q") {
            application->quit();
            switchMap = false;
    }

    if (key_name_string == "1" && gtk_switch_get_active((GtkSwitch*) application->get_object("HotelSwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("HotelSwitch"), false);
            application->update_message("Hotel Filter Turned Off");
            hotelOn = false;
    }
    else if (key_name_string == "1" && !gtk_switch_get_active((GtkSwitch*) application->get_object("HotelSwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("HotelSwitch"), true);
            application->update_message("Hotel Filter Turned On");
            hotelOn = true;
    }

    if (key_name_string == "2" && gtk_switch_get_active((GtkSwitch*) application->get_object("RestaurantSwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("RestaurantSwitch"), false);
            application->update_message("Restaurant Filter Turned Off");
            restaurantOn = false;
    }
    else if (key_name_string == "2" && !gtk_switch_get_active((GtkSwitch*) application->get_object("RestaurantSwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("RestaurantSwitch"), true);
            application->update_message("Restaurant Filter Turned On");
            restaurantOn = true;
    }

    if (key_name_string == "3" && gtk_switch_get_active((GtkSwitch*) application->get_object("SubwaySwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("SubwaySwitch"), false);
            application->update_message("Subway Filter Turned Off");
            subwayOn = false;
    }
    else if (key_name_string == "3" && !gtk_switch_get_active((GtkSwitch*) application->get_object("SubwaySwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("SubwaySwitch"), true);
            application->update_message("Subway Filter Turned On");
            subwayOn = true;
    }

    if (key_name_string == "4" && gtk_switch_get_active((GtkSwitch*) application->get_object("BarSwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("BarSwitch"), false);
            application->update_message("Bar and Nightclub Filter Turned Off");
            barOn = false;
    }
    else if (key_name_string == "4" && !gtk_switch_get_active((GtkSwitch*) application->get_object("BarSwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("BarSwitch"), true);
            application->update_message("Bar and Nightclub Filter Turned On");
            barOn = true;
    }

    if (key_name_string == "5" && gtk_switch_get_active((GtkSwitch*) application->get_object("BankSwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("BankSwitch"), false);
            application->update_message("Bank and ATM Filter Turned Off");
            bankOn = false;
    }
    else if (key_name_string == "5" && !gtk_switch_get_active((GtkSwitch*) application->get_object("BankSwitch"))){
            gtk_switch_set_active((GtkSwitch*) application->get_object("BankSwitch"), true);
            application->update_message("Bank and ATM Filter Turned On");
            bankOn = true;
    }
    application->refresh_drawing();
    }
}

// Highlights all the roads and intersections then zooms onto them
void setHighlight(std::string text, ezgl::application *application) {
    if(text.empty()){
        std::cout << "No text inputted" << std::endl;
        application->update_message("No text inputted");
        return;
    }

    std::string first_street = text.substr(0, text.find('&'));
    std::string second_street = text.substr(text.find('&') + 1, text.length() - 1);

    if(first_street[first_street.length()-1] == ' ')
        first_street.erase(first_street.end()-1);
    if(second_street[0] == ' ')
        second_street.erase(second_street.begin());

    std::vector<unsigned> first_street_id = find_street_ids_from_partial_street_name(first_street);
    std::vector<unsigned> second_street_id = find_street_ids_from_partial_street_name(second_street);

    // Find All matching intersections
    std::vector<unsigned> intersections_to_highlight;
    for(unsigned i = 0; i < first_street_id.size(); ++i){
        for(unsigned j = 0; j < second_street_id.size(); ++j){
                std::vector<unsigned> ith_jth_vector_of_ids = find_intersection_ids_from_street_ids(first_street_id[i], second_street_id[j]);
                for(unsigned k = 0; k < ith_jth_vector_of_ids.size(); ++k){
                intersections_to_highlight.push_back(ith_jth_vector_of_ids[k]);
                }
        }
    }

    if(intersections_to_highlight.size() == 0){
        std::cout << "No matches" << std::endl;
        application->update_message("No Matches");
        return;
    }

    // Determine the bounds of the found intersections
    double max_top = application->get_canvas("MainCanvas")->get_camera().get_initial_world().bottom();
    double max_left = application->get_canvas("MainCanvas")->get_camera().get_initial_world().right();
    double max_right = application->get_canvas("MainCanvas")->get_camera().get_initial_world().left();
    double max_bottom = application->get_canvas("MainCanvas")->get_camera().get_initial_world().top();

    for(unsigned id = 0; id < intersections_to_highlight.size() ; ++id){
    MAPDATA.intersections[intersections_to_highlight[id]].highlight = true;
    application->update_message(MAPDATA.intersections[intersections_to_highlight[id]].name);
    std::cout<< "Found: " << MAPDATA.intersections[intersections_to_highlight[id]].name << std::endl;
    lastHighlightedIndex = intersections_to_highlight[id];

    if(y_from_lat(MAPDATA.intersections[intersections_to_highlight[id]].position.lat()) < max_bottom)
            max_bottom = y_from_lat(MAPDATA.intersections[intersections_to_highlight[id]].position.lat());
    if(y_from_lat(MAPDATA.intersections[intersections_to_highlight[id]].position.lat()) > max_top)
            max_top = y_from_lat(MAPDATA.intersections[intersections_to_highlight[id]].position.lat());
    if(x_from_lon(MAPDATA.intersections[intersections_to_highlight[id]].position.lon()) < max_left)
            max_left = x_from_lon(MAPDATA.intersections[intersections_to_highlight[id]].position.lon());
    if(x_from_lon(MAPDATA.intersections[intersections_to_highlight[id]].position.lon()) > max_right)
            max_right = x_from_lon(MAPDATA.intersections[intersections_to_highlight[id]].position.lon());
    }

    // Find all matching street segments
    for(unsigned ss_id = 0; ss_id < MAPDATA.streetsegments_highlighter.size() ; ++ss_id){
    bool from_highlight = MAPDATA.intersections[getInfoStreetSegment(ss_id).from].highlight;
    bool to_highlight = MAPDATA.intersections[getInfoStreetSegment(ss_id).to].highlight;
    if(from_highlight && to_highlight)
            MAPDATA.streetsegments_highlighter[ss_id] = true;
    }

    // Determine the scale for the zoom
    double screen_height = application->get_canvas("MainCanvas")->get_camera().get_initial_world().height();
    double screen_width = application->get_canvas("MainCanvas")->get_camera().get_initial_world().width();
    double scale_factor = screen_height/screen_width;


    // Zoom onto all the intersections found
    if(std::abs(max_top - max_bottom) > std::abs(max_right - max_left)){
    ezgl::zoom_fit(application->get_canvas("MainCanvas"),
            ezgl::rectangle({max_left-0.00002, max_bottom-0.00002}, std::abs(max_top-max_bottom+0.00005)/scale_factor, std::abs(max_top-max_bottom+0.00005)));
    }
    else{
    ezgl::zoom_fit(application->get_canvas("MainCanvas"),
            ezgl::rectangle({max_left-0.00002, max_bottom-0.00002}, std::abs(max_right-max_left+0.00005), std::abs(max_right-max_left+0.00005)*scale_factor));
    }

    application->refresh_drawing();
    intersections_to_highlight.clear();
}

// Remove all the highlight flags
void clearHighlight() {
    for (int i = 0; i < getNumIntersections(); ++i)
    MAPDATA.intersections[i].highlight = false;
    for (int i = 0; i < getNumStreetSegments(); ++i)
    MAPDATA.streetsegments_highlighter[i] = false;
}

// Function to switch to the selected map in a drop down menu
void change_map_button(GtkWidget *, ezgl::application *application){

    //Obtains the text from the drop down menu labeled as "ChangeMap' and converts it to a string
    GtkComboBoxText* change_map_entry = (GtkComboBoxText*) application->get_object("ChangeMap");
    std::string map_text = (std::string) gtk_combo_box_text_get_active_text(change_map_entry);
    std::string map_file_name;

    //Selects the file path name based upon the text from the drop down menu
    if (map_text == "Beijing, China")
    map_file_name = "beijing_china";
    else if (map_text == "Cairo, Egypt")
    map_file_name = "cairo_egypt";
    else if (map_text == "Cape Town, South Africa")
    map_file_name = "cape-town_south-africa";
    else if (map_text == "Golden Horseshoe, Canada")
    map_file_name = "golden-horseshoe_canada";
    else if (map_text == "Hamilton, Canada")
    map_file_name = "hamilton_canada";
    else if (map_text == "Hong Kong, China")
    map_file_name = "hong-kong_china";
    else if (map_text == "Iceland")
    map_file_name = "iceland";
    else if (map_text == "Interlaken, Switzerland")
    map_file_name = "interlaken_switzerland";
    else if (map_text == "London, England")
    map_file_name = "london_england";
    else if (map_text == "Moscow, Russia")
    map_file_name = "moscow_russia";
    else if (map_text == "New Delhi, India")
    map_file_name = "new-delhi_india";
    else if (map_text == "New York, USA")
    map_file_name = "new-york_usa";
    else if (map_text == "Rio de Janeiro, Brazil")
    map_file_name = "rio-de-janeiro_brazil";
    else if (map_text == "Saint Helena")
    map_file_name = "saint-helena";
    else if (map_text == "Singapore")
    map_file_name = "singapore";
    else if (map_text == "Sydney, Australia")
    map_file_name = "sydney_australia";
    else if (map_text == "Tehran, Iran")
    map_file_name = "tehran_iran";
    else if (map_text == "Tokyo, Japan")
    map_file_name = "tokyo_japan";
    else if (map_text == "Toronto, Canada")
    map_file_name = "toronto_canada";
    else {
    std::cout << "Invalid map. Please try again\n";
    application->update_message("Invalid map. Please try again");
    return;
    }

    //Outputs which map is loaded to the room in a message
    std::cout << map_text << std::endl;
    std::stringstream ss;
    ss << map_text << " Map Loaded";
    std::string map_loaded_text = ss.str();

    //Combines the obtained data with the different file names
    std::stringstream newSS;
    newSS << "/cad2/ece297s/public/maps/" << map_file_name << ".streets.bin";
    newMap = newSS.str();

    newMapOSM = "/cad2/ece297s/public/maps/" + map_file_name + ".osm.bin";

    //Quits the application and turns on the global boolean
    application->quit();
    switchMap = true;
    return;
}

// Function to switch the Screen to dark mode
void dark_mode_button(GtkWidget *, ezgl::application *application)
{
    //Turns on dark mode and sends a message in the status bar
    if (darkMode == false) {
        darkMode = true;
        application->update_message("Dark Mode Turned On");
        application->refresh_drawing();
    }
    //Turns off dark mode and sends a message in the status bar
    else if (darkMode) {
        darkMode = false;
        application->update_message("Dark Mode Turned Off");
        application->refresh_drawing();
    }
}

// Function to consider the actions of the program upon mouse motion
void act_on_mouse_move(ezgl::application *application, GdkEventButton *, double , double ) {
    //Global booleans for toggling the filters are updated based on the switch values
    hotelOn = gtk_switch_get_active((GtkSwitch*) application->get_object("HotelSwitch"));
    restaurantOn = gtk_switch_get_active((GtkSwitch*) application->get_object("RestaurantSwitch"));
    subwayOn = gtk_switch_get_active((GtkSwitch*) application->get_object("SubwaySwitch"));
    barOn = gtk_switch_get_active((GtkSwitch*) application->get_object("BarSwitch"));
    bankOn = gtk_switch_get_active((GtkSwitch*) application->get_object("BankSwitch"));

    //Updates the icons shown or not shown on the map
    application->refresh_drawing();
}

// Converts the TurnType into a string for g_strdup_printf to work
char* turn_type_to_char(TurnType turn){
    char * str;
    if (turn == TurnType::LEFT){
        str = "left";
        return str;
    }
    else if (turn == TurnType::RIGHT){
        str = "right";
        return str;
    }
    else if (turn == TurnType::STRAIGHT){
        str = "STRAIGHT";
        return str;
    }
    else{
        str = "NO TURN";
        return str;
    }
}


void directions_button(GtkWidget *, ezgl::application *application){
    
    // If the route is empty, then don't do anything.
    if(found_path.size() == 0){
    std::cout << "Invalid path. Try again\n" << std::endl;
    application->update_message("Invalid path. Try again");
    return;

    }
    // Counter for looping through all the straight segments together
    counter = found_path.size();

    // Initialize the turn directions, distance and iterator i
    turn_dir = TurnType::STRAIGHT;
    previous_turn = TurnType::STRAIGHT;
    double distance = 0;
    conseq_segments = 0;

    // Combine all the straight street segments into one instruction
    while((conseq_segments < found_path.size()-1) && (turn_dir == previous_turn)){

    previous_turn = turn_dir;

    distance += find_street_segment_length(found_path[conseq_segments]);

    turn_dir = find_turn_type(found_path[conseq_segments], found_path[conseq_segments+1]);


    conseq_segments++;
    counter -- ;
    }


    direction = turn_type_to_char(turn_dir);

    // Converting the street name from string into char* for g_strdup_printf to work
    std::string street_name_str = getStreetName(getInfoStreetSegment(found_path[conseq_segments]).streetID);
    char *street_name_c = new char[street_name_str.length() + 1];
    std::strcpy(street_name_c, street_name_str.c_str());

    application->update_message("Directions");
    application->refresh_drawing();

    GtkWidget *content_area; // the content area of the dialog
    GtkWidget *label; // the label we will create to display a message in the content area
    GtkWidget *dialog; // the dialog box we will create

    // get a pointer to the main application window
    window1 = application->get_object(application->get_main_window_id().c_str());


    // Create the dialog window.
    // Modal windows prevent interaction with other windows in the same application
    if(counter <= 1){
            dialog = gtk_dialog_new_with_buttons(
            "Directions",
            (GtkWindow*) window1,
            GTK_DIALOG_MODAL,
            ("OK"),
            GTK_RESPONSE_ACCEPT,
            NULL
    );
    }
    else{
            dialog = gtk_dialog_new_with_buttons(
            "Directions",
            (GtkWindow*) window1,
            GTK_DIALOG_MODAL,
            ("CANCEL"),
            GTK_RESPONSE_ACCEPT,
            ("NEXT"),
            GTK_RESPONSE_YES,
            NULL
    );
    }


    // Create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    label = gtk_label_new("0");

    gchar* display;

    // Update the dialog message according to different cases
    if(counter <= 1)
            display = g_strdup_printf("In %.0f meters you will arrive at your destination", distance);

    else
            if(street_name_str == "<unknown>")
            display = g_strdup_printf("In %.0f meters turn %s.", distance, direction);
            else
            display = g_strdup_printf("In %.0f meters turn %s on %s.", distance, direction, street_name_c);

    // Free up the pointer
    delete [] street_name_c;

    gtk_label_set_text (GTK_LABEL(label), display); //set label to "display"
    g_free(display);

    gtk_container_add(GTK_CONTAINER(content_area), label);

    // The main purpose of this is to show dialog’s child widget, label
    gtk_widget_show_all(dialog);

    g_signal_connect(
    GTK_DIALOG(dialog),
    "response",
    G_CALLBACK(on_dialog_response_dir),
    NULL
    );
}

void on_dialog_response_dir(GtkDialog *dialog, gint response_id, gpointer)
{
    switch(response_id) {
    case GTK_RESPONSE_ACCEPT:
            //std::cout << "Pressed OK";
            //found_path.clear();
            break;
    case GTK_RESPONSE_DELETE_EVENT:
            //std::cout << "Exit Menu";
            break;
    case GTK_RESPONSE_YES:
            //std::cout << "GTK_RESPONSE_YES";
            break;
    default:
            //std::cout << "UNKNOWN ";
            break;
    }

    // This will cause the dialog to be destroyed and close
    // without this line the dialog remains open unless the
    // response_id is GTK_RESPONSE_DELETE_EVENT which
    // automatically closes the dialog without the following line.
    gtk_widget_destroy(GTK_WIDGET (dialog));

    if(response_id == GTK_RESPONSE_YES){
        // Decrement the counter for every segment.
        counter--;

        // Initialize the distance and turns for this chunk of street segments (that are all going
        //    	in the same direction).
        double distance = 0;
        previous_turn = TurnType::STRAIGHT;
        turn_dir = TurnType::STRAIGHT;

        // Combine all the straight street segments into one instruction
        while((conseq_segments < found_path.size()-1) && (turn_dir == previous_turn)){

                previous_turn = turn_dir;

                distance += find_street_segment_length(found_path[conseq_segments]);

                turn_dir = find_turn_type(found_path[conseq_segments], found_path[conseq_segments+1]);

                conseq_segments++;
                counter -- ;
        }

        direction = turn_type_to_char(turn_dir);

        // Converting the street name from string into char* for g_strdup_printf to work
        std::string street_name_str = getStreetName(getInfoStreetSegment(found_path[conseq_segments]).streetID);
        char *street_name_c = new char[street_name_str.length() + 1];
        std::strcpy(street_name_c, street_name_str.c_str());


        GtkWidget *content_area; // the content area of the dialog
        GtkWidget *label; // the label we will create to display a message in the content area
        GtkWidget *dialog1; // the dialog box we will create


        // Create the dialog window.
        // Modal windows prevent interaction with other windows in the same application

        // Create a different dialog window if the instructions are finished
        if(counter <= 1){
                dialog1 = gtk_dialog_new_with_buttons(
                "Directions",
                (GtkWindow*) window1,
                GTK_DIALOG_MODAL,
                ("OK"),
                GTK_RESPONSE_ACCEPT,
                NULL
        );
        }
        else{
                dialog1 = gtk_dialog_new_with_buttons(
                "Directions",
                (GtkWindow*) window1,
                GTK_DIALOG_MODAL,
                ("CANCEL"),
                GTK_RESPONSE_ACCEPT,
                ("NEXT"),
                GTK_RESPONSE_YES,
                NULL
        );
        }

        // Create a label and attach it to the content area of the dialog
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog1));

        label = gtk_label_new("0");

        gchar* display;

        // Update the dialog message according to different cases.
        if(counter <= 1)
                display = g_strdup_printf("In %.0f meters you will arrive at your destination", distance);
        else
                if(street_name_str == "<unknown>")
                display = g_strdup_printf("In %.0f meters turn %s.", distance, direction);
                else
                display = g_strdup_printf("In %.0f meters turn %s on %s.", distance, direction, street_name_c);

        // Free up the pointer
        delete [] street_name_c;

        gtk_label_set_text (GTK_LABEL(label), display); //set label to "display"
        g_free(display);

        gtk_container_add(GTK_CONTAINER(content_area), label);

        // The main purpose of this is to show dialog’s child widget, label
        gtk_widget_show_all(dialog1);

        g_signal_connect(
                GTK_DIALOG(dialog1),
                "response",
                G_CALLBACK(on_dialog_response_dir),
                NULL
        );
    }
}

//Function to obtain the directions for a given path
void get_directions(unsigned intersect_id_start, unsigned intersect_id_end){
    //Clears data structures before manipulating them
    clearHighlight();
    found_path.clear();
    
    selectedIntersectionPathBegin = intersect_id_start;
    selectedIntersectionPathEnd = intersect_id_end;
    
    //Finds the path that connects the two intersections
    found_path = find_path_between_intersections(intersect_id_start, intersect_id_end, right_turn_penalty_global, left_turn_penalty_global);
    
    //Updates the street segments to be highlighted
    for(unsigned ss_id = 0; ss_id < found_path.size(); ++ss_id){
        MAPDATA.streetsegments_highlighter[found_path[ss_id]] = true;
    }
    return;
}

//Helper function for the get_directions to convert strings to the start and end intersections
void get_directions_helper(std::string text1, std::string text2, ezgl::application *application_p) {
    //Collects the street segments from the intersection strings
    std::string text1_first_street = text1.substr(0, text1.find('&'));
    std::string text1_second_street = text1.substr(text1.find('&') + 1, text1.length() - 1);
    std::string text2_first_street = text2.substr(0, text2.find('&'));
    std::string text2_second_street = text2.substr(text2.find('&') + 1, text2.length() - 1);
    
    //Removes the spaces from the strings
    if(text1_first_street[text1_first_street.length()-1] == ' ')
        text1_first_street.erase(text1_first_street.end()-1);
    if(text1_second_street[0] == ' ')
        text1_second_street.erase(text1_second_street.begin());
    if(text2_first_street[text2_first_street.length()-1] == ' ')
        text2_first_street.erase(text2_first_street.end()-1);
    if(text2_second_street[0] == ' ')
        text2_second_street.erase(text2_second_street.begin());
    
    //Collects the street segment IDs
    std::vector<unsigned> text1_first_street_id = find_street_ids_from_partial_street_name(text1_first_street);
    std::vector<unsigned> text1_second_street_id = find_street_ids_from_partial_street_name(text1_second_street);
    std::vector<unsigned> text2_first_street_id = find_street_ids_from_partial_street_name(text2_first_street);
    std::vector<unsigned> text2_second_street_id = find_street_ids_from_partial_street_name(text2_second_street);
    
    // Find the starting intersection
    std::vector<unsigned> text1_intersections_to_highlight;
    for(unsigned i = 0; i < text1_first_street_id.size(); ++i){
        for(unsigned j = 0; j < text1_second_street_id.size(); ++j){
                std::vector<unsigned> ith_jth_vector_of_ids = find_intersection_ids_from_street_ids(text1_first_street_id[i], text1_second_street_id[j]);
                for(unsigned k = 0; k < ith_jth_vector_of_ids.size(); ++k){
                text1_intersections_to_highlight.push_back(ith_jth_vector_of_ids[k]);
                }
        }
    }
    
    // Find the destination intersection
    std::vector<unsigned> text2_intersections_to_highlight;
    for(unsigned i = 0; i < text2_first_street_id.size(); ++i){
        for(unsigned j = 0; j < text2_second_street_id.size(); ++j){
                std::vector<unsigned> ith_jth_vector_of_ids = find_intersection_ids_from_street_ids(text2_first_street_id[i], text2_second_street_id[j]);
                for(unsigned k = 0; k < ith_jth_vector_of_ids.size(); ++k){
                text2_intersections_to_highlight.push_back(ith_jth_vector_of_ids[k]);
                }
        }
    }

    //In case, the street segments don't match to an intersection, return from the function
    if(text1_intersections_to_highlight.size() == 0 or text2_intersections_to_highlight.size() == 0){
        application_p->update_message("No matching intersections");
        return;
    }
    
    //Obtains the path given the start and end point
    get_directions(text1_intersections_to_highlight[0],text2_intersections_to_highlight[0]);
}
