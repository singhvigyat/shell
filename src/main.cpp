#include <iostream>
#include <string>
#include <sstream>
using namespace std;

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

    //condition checking 
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
        cout << str  << ": not found" << endl;
      }
    }
    else
    {
      cout << s << ": command not found" << endl;
    }
  }
}
