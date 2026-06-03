#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include "modules/executable.hpp"
using namespace std;
namespace filesystem = std::filesystem;

int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage

  while (true)
  {
    std::cout << "$ ";
    std::string s;
    std::getline(cin, s);
    if (s == "exit")
    {
      break;
    }

    stringstream ss(s);
    string cmd;
    getline(ss, cmd, ' ');

    // condition checking
    if (cmd == "echo")
    {
      string str;
      getline(ss, str);
      cout << str << endl;
    }
    else if (cmd == "type")
    {
      string str;
      getline(ss, str);
      if (str == "type" || str == "echo" || str == "exit")
      {
        cout << str << " " << "is a shell builtin" << endl;
      }
      else
      {
        // keep this read only(do const char*ch), as this returns the memory owned by the environment, not by our program, so we won't be modifying this, directly, as its not safe.

        // this we're not doing string = getenv("PATH") because it may return nullptr, & we can't do string of nullptr.

        // char ch* -> points to same memory (no copy )
        // string s -> points to different memory (its own copy)
        if (!executable(str, 0))
        {
          cout << str << ": not found" << endl;
        }
      }
    }
    else
    {
      // the first string would be the executable name, we called it cmd before, we reusing it.
      string exe_name = cmd;
      if (executable(exe_name, 0))
      {
        system(s.c_str());
      }
      // cout << s << ": command not found" << endl;
    }
  }
}
