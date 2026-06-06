#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
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

      bool redirect_operator = false;
      std::string output_file;
      std::string to_write_input;

      for (size_t i = 1; i < input.size(); ++i)
      {
        if (input[i] == ">" || input[i] == "1>")
        {
          redirect_operator = true;
        }
        if (input[i - 1] == ">" || input[i - 1] == "1>")
        {
          output_file = input[i];
        }
        if ((input[i - 1] != ">" && input[i] != ">") && (input[i - 1] != "1>" && input[i] != "1>"))
        {
          if (i > 1)
            to_write_input.push_back(' ');
          to_write_input.append(input[i]);
        }
      }

      if (redirect_operator)
      {

        // THIS IS THE WINDOWS ONLY CODE (LOW-LEVEL)
        // // Open or create a file handle
        // // first arg => LPCSTR -> long pointer const string (const char*)
        // HANDLE hFile = CreateFileA(output_file.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        // if (hFile == INVALID_HANDLE_VALUE)
        // {
        //   std::cerr << "Error opening file handle." << std::endl;
        //   return 1;
        // }

        // const char *data = to_write_input.c_str();
        // // DWORD => double word, 32 bits (bytes = 8 bits, word = 16 bits)
        // DWORD byteswritten = 0;

        // // Use the Win32 WriteFile function
        // // BOOL -> defined in windows, way before cpp's bool var
        // BOOL result = WriteFile(hFile, data, strlen(data), &byteswritten, NULL);

        // if (!result)
        // {
        //   std::cerr << "WriteFile failed. Error code: " << GetLastError() << std::endl;
        // }

        // CloseHandle(hFile); // Clean up the handle

        std::ofstream hFile(output_file);
        if (!hFile.is_open())
        {
          std::cerr << "Error opening file." << std::endl;
          continue;
        }
        hFile << to_write_input;

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

      bool redirect_operator = false;
      std::string output_file_arg;
      std::vector<std::string> input_files;

      for (size_t i = 1; i < input.size(); ++i)
      {
        if (input[i] == ">" || input[i] == "1>")
        {
          redirect_operator = true;
        }
        if (input[i - 1] == ">" || input[i - 1] == "1>")
        {
          output_file_arg = input[i];
        }
        if ((input[i - 1] != ">" && input[i] != ">") && (input[i - 1] != "1>" && input[i] != "1>"))
        {
          input_files.push_back(input[i]);
        }
      }

      if (redirect_operator)
      {

        // WINDOWS ONLY CODE (LOW-LEVEL, WILL ONLY RUN ON WINDOW)
        // char buffer[4096];
        // DWORD bytesRead, bytesWritten;

        // HANDLE output_file = CreateFileA(output_file_arg.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        // for (auto &file : input_files)
        // {
        //   HANDLE input_file = CreateFileA(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
        //                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        //   if (input_file == INVALID_HANDLE_VALUE)
        //   {
        //     std::cout << "cat: " << file << ": " << "No such file or directory\n";
        //     continue;
        //   }

        //   while (ReadFile(input_file, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
        //   {
        //     WriteFile(output_file, buffer, bytesRead, &bytesWritten, NULL);
        //   }
        //   CloseHandle(input_file);
        // }

        // CloseHandle(output_file);

        std::ofstream output_file(output_file_arg);

        if (!output_file.is_open())
        {
          std::cerr << "Error opening output file.\n";
        }
        else
        {
          for (auto &file : input_files)
          {
            // 2. Open each input file for reading
            std::ifstream input_file(file);

            if (!input_file.is_open())
            {
              // Keeping your exact error message
              std::cout << "cat: " << file << ": No such file or directory\n";
              break;
            }

            // 3. High-level magic: Pipe the entire file buffer directly to the output file.
            // This entirely replaces the 'while(ReadFile...)' 4KB buffer loop!
            output_file << input_file.rdbuf();

            // input_file automatically closes here at the end of the loop
          }
          // output_file automatically closes when it goes out of scope
        }
      }
      else
      {
        for (size_t file = 1; file < input.size(); ++file)
        {
          std::ifstream fin(input[file]);
          if (fin)
          {
            std::cout << fin.rdbuf();
          }
          else
          {
            // std::cerr << "cat: " << input[file] << ": No such file or directory\n";
          }
        }
      }
      // cout << endl;
    }
    else if (cmd == "ls")
    {
      // A structure filled by Windows is WIN32_FIND_DATAA
      // WIN32_FIND_DATAA findData;
      bool redirect_operator = false;
      std::string output_file_arg;
      std::vector<std::string> input_files;

      for (size_t i = 1; i < input.size(); ++i)
      {
        if (input[i] == ">" || input[i] == "1>")
        {
          redirect_operator = true;
        }
        if (input[i - 1] == ">" || input[i - 1] == "1>")
        {
          output_file_arg = input[i];
        }
        if ((input[i - 1] != ">" && input[i] != ">") && (input[i - 1] != "1>" && input[i] != "1>"))
        {
          input_files.push_back(input[i]);
        }
      }

      if (input_files.empty())
      {
        input_files.push_back(".");
      }

      if (redirect_operator)
      {

        // HANDLE output_file = CreateFileA(output_file_arg.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        // if (output_file == INVALID_HANDLE_VALUE)
        // {
        //   std::cerr << "Cannot open output file\n";
        //   continue;
        // }

        // for (auto &file : input_files)
        // {
        //   std::string pattern = file + "\\*";
        //   HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);

        //   if (hFind == INVALID_HANDLE_VALUE)
        //   {
        //     std::cerr
        //         << "ls: cannot access "
        //         << file
        //         << '\n';
        //   }

        //   if (hFind != INVALID_HANDLE_VALUE)
        //   {
        //     do
        //     {
        //       if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
        //         continue;

        //       // write here to the output file.
        //       const char *data = findData.cFileName;
        //       DWORD byteswritten = 0;
        //       BOOL result = WriteFile(output_file, data, strlen(data), &byteswritten, NULL);

        //       // to append new line everytime
        //       const char *nl = "\n";
        //       WriteFile(output_file, nl, 1, &byteswritten, NULL);
        //       if (!result)
        //       {
        //         std::cerr << "WriteFile failed. Error code: " << GetLastError() << std::endl;
        //       }

        //     } while (FindNextFileA(hFind, &findData));

        //     FindClose(hFind);
        //   }
        // }
        // CloseHandle(output_file);

        std::ofstream output_file(output_file_arg);

        if (!output_file.is_open())
        {
          std::cerr << "Cannot open output file\n";
          continue;
        }

        for (const auto &dir : input_files)
        {
          try
          {
            // High-level C++ directory iterator (automatically skips "." and "..")
            for (const auto &entry : std::filesystem::directory_iterator(dir))
            {
              output_file << entry.path().filename().string() << "\n";
            }
          }
          catch (const std::filesystem::filesystem_error &e)
          {
            std::cerr << "ls: cannot access '" << dir << "': No such file or directory\n";
          }
        }
      }
      else
      {
        // HANDLE hFind = FindFirstFileA(str.append("\\*").c_str(), &findData);

        // if (hFind != INVALID_HANDLE_VALUE)
        // {
        //   do
        //   {
        //     // hiding the '.' & '..' files (which means current & parent location)
        //     // so using strcmp (returns 0 if strings are equal),
        //     // using strcmp -> because this is used to compare const *char, these are not strings.
        //     if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
        //       continue;
        //     std::cout << findData.cFileName << '\n';
        //   } while (FindNextFileA(hFind, &findData));

        //   FindClose(hFind);

        for (const auto &dir : input_files)
        {
          try
          {
            for (const auto &entry : std::filesystem::directory_iterator(dir))
            {
              std::cout << entry.path().filename().string() << "\n";
            }
          }
          catch (const std::filesystem::filesystem_error &e)
          {
            std::cerr << "ls: cannot access '" << dir << "': No such file or directory\n";
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
