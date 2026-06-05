#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include "modules/utils.hpp"

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

    // tokenize the user input, to get the first executalbe name correctly, if the user is passing the name of the executable in quotes.
    std::vector<std::string> input = tokenize(s);

    // stringstream ss(s);
    string cmd;
    // getline(ss, cmd, ' ');
    cmd = input[0];
    string str;
    for (size_t i = 1; i < input.size(); i++)
    {
      str.append(input[i]);
      if (i != input.size() - 1)
        str.push_back(' ');
    }
    // getline(ss, str);

    // condition checking
    if (cmd == "echo")
    {
      vector<string> tokenized_input = tokenize(str);
      for (auto &i : tokenized_input)
      {
        cout << i << " ";
      }
      cout << endl;
    }
    else if (cmd == "type")
    {
      // string str;
      // getline(ss, str);
      if (str == "type" || str == "echo" || str == "exit" || str == "pwd" || str == "cd")
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
    else if (cmd == "pwd")
    {
      cout << filesystem::current_path().string() << endl;
    }
    else if (cmd == "cd")
    {
      string new_directory_path_string = s.substr(3);

      if (new_directory_path_string == "~")
      {
        // go to the home directory
        const char *ch = getenv("HOME");

        if (ch)
        {
          std::string path_to_home(ch);
          filesystem::current_path(path_to_home);
        }
      }
      else
      {
        filesystem::path new_directory_path(new_directory_path_string);
        if (filesystem::exists(new_directory_path))
        {
          filesystem::current_path(new_directory_path);
        }
        else
        {
          cout << "cd: " << s.substr(3) << ": " << "No such file or directory" << endl;
        }
      }
    }
    else if (cmd == "cat")
    {
      std::string input_file = (str);

      std::vector<std::string> tokenized_input = tokenize(input_file);

      for (auto &file : tokenized_input)
      {
        std::ifstream fin(file);
        if (fin)
        {
          std::cout << fin.rdbuf();
        }
      }
    }
    else
    {
      // the first string would be the executable name, we called it cmd before, we reusing it.
      string exe_name = cmd;

      // for treating already present double quotes in the existing .exe file name. 
      for(size_t i = 0;i<exe_name.size();++i){
        if(exe_name[i]=='"'){
          exe_name.replace(i, 1, "\\\""); 
          i+=1; 
        }
      }

      cout<<exe_name<<endl; 
      // create a system() compatible command, which will have "" dquotes, around the exe file name, along with the arguments after that. 
      string to_execute_command = "\"" + exe_name + "\"" + " "+ str;
      // cout<<to_execute_command<<endl;
      // cout << exe_name << endl;
      // cout << str << endl;
      if (executable(exe_name, 0))
      {
        system(to_execute_command.c_str());
      }
      else
        cout << s << ": command not found" << endl;
    }
  }
}
