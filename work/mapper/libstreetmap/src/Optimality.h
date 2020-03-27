/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Optimality.h
 * Author: furaijiy, mahroos2, chadhas7
 *
 * Created on April 1, 2019, 7:46 PM
 */



#pragma once

#include "m4.h"
#include "m3.h"

void readjust(std::vector<CourierSubpath>& full_path,
        int first_subpath_index, int second_subpath_index, int third_subpath_index,
        const int right_turn_penalty, const int left_turn_penalty);

void two_opt(std::vector<CourierSubpath>& full_path, 
        int first_subpath_index, int second_subpath_index,
        const int right_turn_penalty, const int left_turn_penalty);