#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
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
        const char *ch = getenv("PATH");

        bool ba = false;
        if (ch)
        {
          // string PATH(ch);
          // processing the string;
          stringstream ss(ch);
          string dir;
          while (getline(ss, dir, ';'))
          {
            filesystem::path p = dir;
            // joins str to p => p + str;
            p /= str;
            if (filesystem::exists(p))
            {
              filesystem::perms permission = status(p).permissions();
              if ((permission & filesystem::perms::owner_exec) != filesystem::perms::none)
              {
                cout << str << " is " << p << endl;
                ba = true;
                break;
              }
            }

            // cout << dir << endl;
          }
        }
        if (!ba)
          cout << str << ": not found" << endl;
      }
    }
    else
    {
      cout << s << ": command not found" << endl;
    }
  }
}
