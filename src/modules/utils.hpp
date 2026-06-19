#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>


// this i've done bcz i was getting error saying two copies of separator (one in utils.cpp, other one in the main.cpp, two copies was ambigous, so we told it don't allocate memory here, the seperator is defined somewhere else)
// so just define it in utils.cpp file, the main.cpp imports it automatically while importing the header file
extern char separator; 

// check if file got execute permission or not & print the path, if it does. 
bool executable( std::string &str, bool print_required);


// tokenize the arguments
std::vector<std::string>tokenize(std::string&s); 

// for trimming a string from left & right 
void ltrim(std::string &s) ; 
void rtrim(std::string &s) ; 
void trim(std::string &s) ; 

// Simplified executable check
bool is_executable(const std::filesystem::path& entry) ; 

std::string get_lcp(std::string s, const std::vector<std::string>&matches); 