#pragma once
#include <string>
#include <iostream>

// check if file got execute permission or not & print the path, if it does. 
bool executable( std::string &str, bool print_required);

// trim in string (start, end, middle)
std::string normalize_string(std::string&s); 
