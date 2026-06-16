#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <algorithm>
// #include <windows.h>
#include "modules/utils.hpp"

using namespace std;
namespace filesystem = std::filesystem;

int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

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
      bool stdout_redirect_operator = false;
      bool error_redirect_operator = false;
      bool stdout_append_operator = false;
      std::string output_file;
      std::string to_write_input;

      for (size_t i = 1; i < input.size(); ++i)
      {
        if (input[i] == ">" || input[i] == "1>")
        {
          stdout_redirect_operator = true;
        }
        else if (input[i] == "2>")
        {
          error_redirect_operator = true;
        }
        else if (input[i] == ">>" || input[i] == "1>>")
        {
          stdout_append_operator = true;
        }
        else if (input[i - 1] == ">" || input[i - 1] == "1>" || input[i - 1] == "2>" || input[i - 1] == "1>>" || input[i - 1] == ">>")
        {
          output_file = input[i];
        }
        else if ((input[i - 1] != ">" && input[i] != ">") && (input[i - 1] != "1>" && input[i] != "1>" && input[i - 1] != "2>" && input[i - 1] != "1>>" && input[i] != ">>"))
        {
          if (i > 1)
            to_write_input.push_back(' ');
          to_write_input.append(input[i]);
        }
      }

      if (stdout_redirect_operator)
      {
        std::ofstream hFile(output_file);
        if (!hFile.is_open())
        {
          std::cerr << "Error opening file." << std::endl;
          continue;
        }
        hFile << to_write_input << "\n";

        if (hFile.fail())
        {
          std::cerr << "Write failed." << std::endl;
        }

        hFile.close();
      }
      else if (error_redirect_operator)
      {
        std::ofstream hFile(output_file);
        if (!hFile.is_open())
        {
          std::cerr << "Error opening file." << std::endl;
          continue;
        }
        cout << to_write_input << endl;
      }
      else if (stdout_append_operator)
      {
        std::ofstream hFile(output_file, std::ios::app);
        if (!hFile.is_open())
        {
          std::cerr << "Error opening file." << std::endl;
          continue;
        }
        hFile << to_write_input << "\n";

        if (hFile.fail())
        {
          std::cerr << "Write failed." << std::endl;
        }

        hFile.close();
      }
      else
        cout << str << endl;
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
      bool stdout_redirect_operator = false;
      bool error_redirect_operator = false;

      std::string output_file_arg;
      std::vector<std::string> input_files;

      for (size_t i = 1; i < input.size(); ++i)
      {
        if (input[i] == ">" || input[i] == "1>")
        {
          stdout_redirect_operator = true;
        }
        else if (input[i] == "2>")
        {
          error_redirect_operator = true;
        }
        else if (input[i - 1] == ">" || input[i - 1] == "1>" || input[i - 1] == "2>")
        {
          output_file_arg = input[i];
        }
        else if (input[i] != ">" && input[i] != "1>" && input[i] != "2>")
        {
          input_files.push_back(input[i]);
        }
      }

      if (stdout_redirect_operator)
      {
        std::ofstream output_file(output_file_arg);

        if (!output_file.is_open())
        {
          // Codecrafters standard error format for missing folders
          std::cerr << "bash: " << output_file_arg << ": No such file or directory\n";
        }
        else
        {
          for (const auto &file : input_files)
          {
            std::ifstream input_file(file);

            if (!input_file.is_open())
            {
              std::cout << "cat: " << file << ": No such file or directory\n";
              continue;
            }

            // HIGH-LEVEL CAT MAGIC: Pipes the whole file buffer directly
            output_file << input_file.rdbuf();
          }
        }
      }
      else if (error_redirect_operator)
      {
        // stderr output put in the file (in case of >2)
        std::ofstream output_file(output_file_arg);
        if (!output_file.is_open())
        {
          // Codecrafters standard error format for missing folders
          std::cerr << "bash: " << output_file_arg << ": No such file or directory\n";
        }
        else
        {
          // std::ofstream out_file(file);
          for (const auto &file : input_files)
          {
            std::ifstream in_file(file);

            if (!in_file.is_open())
            {
              output_file << "cat: " << file << ": No such file or directory\n";
              continue;
            }
            cout << in_file.rdbuf();
          }
        }
      }
      else // Terminal Output
      {
        // Using your parsed input_files vector here instead of raw input array
        for (const auto &file : input_files)
        {
          std::ifstream fin(file);
          if (fin)
          {
            std::cout << fin.rdbuf();
          }
          else
          {
            // Uncommented so Codecrafters doesn't fail you on missing files!
            std::cerr << "cat: " << file << ": No such file or directory\n";
          }
        }
      }
    }
    else if (cmd == "ls")
    {
      // A structure filled by Windows is WIN32_FIND_DATAA
      // WIN32_FIND_DATAA findData;
      bool stdout_redirect_operator = false;
      bool error_redirect_operator = false;
      bool stdout_append_operator = false;

      std::string output_file_arg;
      std::vector<std::string> input_files;
      bool flag_one_per_line = false;

      for (size_t i = 1; i < input.size(); ++i)
      {
        if (input[i] == ">" || input[i] == "1>")
        {
          stdout_redirect_operator = true;
        }
        else if (input[i] == "2>")
        {
          error_redirect_operator = true;
        }
        else if (input[i] == ">>" || input[i] == "1>>")
        {
          stdout_append_operator = true;
        }
        else if (input[i - 1] == ">" || input[i - 1] == "1>" || input[i - 1] == "2>" || input[i - 1] == "1>>")
        {
          output_file_arg = input[i];
        }
        else if ((input[i - 1] != ">" && input[i] != ">") && (input[i - 1] != "1>" && input[i] != "1>" && input[i - 1] != "2>" && input[i] != "2>" && input[i - 1] != "1>>" && input[i] != ">>"))
        {
          if (input[i] == "-1")
          {
            flag_one_per_line = true;
          }
          else if (!input[i].empty() && input[i][0] == '-')
          {
            // Safely ignore any other flags (like -a, -l) so they aren't treated as folders
            continue;
          }
          else
          {
            input_files.push_back(input[i]);
          }
        }
      }

      if (input_files.empty())
      {
        input_files.push_back(".");
      }

      if (stdout_redirect_operator)
      {
        std::ofstream output_file(output_file_arg);

        if (!output_file.is_open())
        {
          std::cerr << "bash: " << output_file_arg << ": No such file or directory\n";
          continue;
        }

        for (const auto &dir : input_files)
        {
          if (std::filesystem::is_regular_file(dir))
          {
            output_file << dir << "\n";
          }
          else if (std::filesystem::is_directory(dir))
          {

            // 1. Collect files
            std::vector<std::string> sorted_files;
            for (const auto &entry : std::filesystem::directory_iterator(dir))
            {
              sorted_files.push_back(entry.path().filename().string());
            }

            // 2. Sort alphabetically
            std::sort(sorted_files.begin(), sorted_files.end());

            // 3. Write to file
            for (const auto &file : sorted_files)
            {
              output_file << file << "\n";
            }
          }
          else
          {
            std::cerr << "ls: " << dir << ": No such file or directory\n";
          }
        }
      }
      else if (error_redirect_operator)
      {
        std::ofstream output_file(output_file_arg);

        if (!output_file.is_open())
        {
          std::cerr << "bash: " << output_file_arg << ": No such file or directory\n";
          continue;
        }

        for (const auto &dir : input_files)
        {
          if (std::filesystem::is_regular_file(dir))
          {
            cout << dir << "\n";
          }
          else if (std::filesystem::is_directory(dir))
          {

            // 1. Collect files
            std::vector<std::string> sorted_files;
            for (const auto &entry : std::filesystem::directory_iterator(dir))
            {
              sorted_files.push_back(entry.path().filename().string());
            }

            // 2. Sort alphabetically
            std::sort(sorted_files.begin(), sorted_files.end());

            // 3. Write to file
            for (const auto &file : sorted_files)
            {
              cout << file << "\n";
            }
          }
          else
          {
            output_file << "ls: " << dir << ": No such file or directory\n";
          }
        }
      }
      else if (stdout_append_operator)
      {
        std::ofstream output_file(output_file_arg, std::ios::app);

        if (!output_file.is_open())
        {
          std::cerr << "bash: " << output_file_arg << ": No such file or directory\n";
          continue;
        }

        for (const auto &dir : input_files)
        {
          if (std::filesystem::is_regular_file(dir))
          {
            output_file << dir << "\n";
            // cout << dir << "\n";
          }
          else if (std::filesystem::is_directory(dir))
          {

            // 1. Collect files
            std::vector<std::string> sorted_files;
            for (const auto &entry : std::filesystem::directory_iterator(dir))
            {
              sorted_files.push_back(entry.path().filename().string());
            }

            // 2. Sort alphabetically
            std::sort(sorted_files.begin(), sorted_files.end());

            // 3. Write to file
            for (const auto &file : sorted_files)
            {
              output_file << file << "\n";
            }
          }
          else
          {
            std::cerr << "ls: " << dir << ": No such file or directory\n";
          }
        }
      }
      else
      {
        for (const auto &dir : input_files)
        {
          if (std::filesystem::is_regular_file(dir))
          {
            cout << dir << "\n";
          }
          else if (std::filesystem::is_directory(dir))
          {

            // 1. Collect files
            std::vector<std::string> sorted_files;
            for (const auto &entry : std::filesystem::directory_iterator(dir))
            {
              sorted_files.push_back(entry.path().filename().string());
            }

            // 2. Sort alphabetically
            std::sort(sorted_files.begin(), sorted_files.end());

            // 3. Print out
            for (const auto &file : sorted_files)
            {
              if (flag_one_per_line)
              {
                std::cout << file << "\n";
              }
              else
              {
                std::cout << file << "  ";
              }
            }

            if (!flag_one_per_line && !sorted_files.empty())
            {
              std::cout << "\n";
            }
          }
          else
          {

            std::cerr << "ls: " << dir << ": No such file or directory\n";
          }
        }
      }
    }
    else
    {
      // the first string would be the executable name, we called it cmd before, we reusing it.
      string exe_name = cmd;

      // for treating already present double quotes in the existing .exe file name.
      for (size_t i = 0; i < exe_name.size(); ++i)
      {
        if (exe_name[i] == '"')
        {
          exe_name.replace(i, 1, "\\\"");
          i += 1;
        }
      }

      // create a system() compatible command, which will have "" dquotes, around the exe file name, along with the arguments after that.
      string to_execute_command = "\"" + exe_name + "\"" + " " + str;

      if (executable(cmd, 0))
      {
        system(to_execute_command.c_str());
      }
      else
        cout << s << ": command not found" << endl;
    }
  }
}
