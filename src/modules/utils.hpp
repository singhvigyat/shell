#pragma once
#include <string>
#include <iostream>
#include <vector>
// check if file got execute permission or not & print the path, if it does. 
bool executable( std::string &str, bool print_required);


// tokenize the arguments
std::vector<std::string>tokenize(std::string&s); 

// for trimming a string from left & right 
void ltrim(std::string &s) ; 
void rtrim(std::string &s) ; 
void trim(std::string &s) ; 