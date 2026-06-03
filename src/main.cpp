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
        if (!executable(str, 1))
        {
          cout << str << ": not found" << endl;
        }
      }
    }
    else if (cmd=="pwd"){
      cout<<filesystem::current_path().string()<<endl; 
    }
    else
    {
      // the first string would be the executable name, we called it cmd before, we reusing it.
      string exe_name = cmd;
      if (executable(exe_name, 0))
      {
        system(s.c_str());
      }
      else
        cout << s << ": command not found" << endl;
    }
  }
}
