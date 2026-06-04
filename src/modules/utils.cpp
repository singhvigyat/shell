#include <iostream>
#include <string>
#include <filesystem>
#include <sstream>
#include "utils.hpp"
using namespace std;

// #ifdef, #else, #endif — preprocessor directives
// _WIN32 — a predefined compiler macro
// The whole block — conditional compilation
#ifdef _WIN32
char separator = ';';
#else
char separator = ':';
#endif

bool executable(string &str, bool print_required)
{
    // keep this read only(do const char*ch), as this returns the memory owned by the environment, not by our program, so we won't be modifying this, directly, as its not safe.

    // this we're not doing string = getenv("PATH") because it may return nullptr, & we can't do string of nullptr.

    // char ch* -> points to same memory (no copy )
    // string s -> points to different memory (its own copy)
    const char *ch = getenv("PATH");

    bool ba = false;
    if (ch)
    {
        // processing the string;
        stringstream ss(ch);
        string dir;
        while (getline(ss, dir, separator))
        {
            filesystem::path p = dir;
            // joins str to p => p + str;
            p /= str;
            if (filesystem::exists(p))
            {
                filesystem::perms permission = status(p).permissions();
                if ((permission & filesystem::perms::owner_exec) != filesystem::perms::none)
                {
                    if (print_required)
                        cout << str << " is " << p.string() << endl;
                    ba = true;
                    break;
                }
            }

            // cout << dir << endl;
        }
    }

    return ba;
}


std::vector<std::string> tokenize(std::string &s)
{

    vector<string> separated_args;
    bool isquoted = false;
    std::string res;

    for (auto &ch : s)
    {
        if (ch == '\'')
        {
            isquoted = !isquoted;
        }
        else if (isquoted)
        {
            res.push_back(ch);
        }
        else
        {
            if (std::isspace(static_cast<unsigned char>(ch)))
            {
                if (!isquoted && !res.empty())
                {
                    separated_args.push_back(res);
                    res.clear();
                }
            }
            else
            {
                res.push_back(ch);
            }
        }
    }

    if (!res.empty())
    {
        separated_args.push_back(res);
    }
    return separated_args;
}