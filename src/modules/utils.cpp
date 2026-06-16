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

// check the file has execute permissions or not.
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

// tokenize a string into multiple tokens
std::vector<std::string> tokenize(std::string &s)
{
    vector<string> separated_args;
    std::string res;
    char quote = '\0';
    bool back_slash = false;

    for (auto &ch : s)
    {
        if (back_slash)
        {
            if (quote == '"' && ch != '\\' && ch != '"' && ch != '$' && ch != '`')
            {
                res.push_back('\\');
            }
            res.push_back(ch);
            back_slash = false;
            continue;
        }
        if (quote == '\0')
        {
            if (ch == '>')
            {
                if (!res.empty()&&res[0]=='1'&&res.size()==1)
                {
                    separated_args.push_back("1>"); 
                    res.clear(); 
                }else if(!res.empty()&&res[0]=='2'&&res.size()==1){
                    separated_args.push_back("2>"); 
                    res.clear(); 
                }
                else
                {
                    if (!res.empty())
                    {
                        separated_args.push_back(res);
                        res.clear();
                    }
                    if (!separated_args.empty() && separated_args.back() == ">")
                    {
                        separated_args.back() = ">>";
                    }else if(!separated_args.empty()&&separated_args.back()=="1>"){
                        separated_args.back() = "1>>"; 
                    }
                    else
                        separated_args.push_back(">");
                }
                continue;
            }
            else if (ch == '\'' || ch == '"')
            {
                quote = ch;
            }
            else if (ch == '\\')
            {
                back_slash = true;
            }
            else
            {
                if (std::isspace(static_cast<unsigned char>(ch)))
                {
                    if (quote == '\0' && !res.empty())
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
        else if (quote == ch)
        {
            quote = '\0';
        }
        else
        {
            if (quote == '"' && ch == '\\')
            {
                back_slash = true;
            }
            else
                res.push_back(ch);
        }
    }
    if (!res.empty())
    {
        separated_args.push_back(res);
    }

    return separated_args;
}