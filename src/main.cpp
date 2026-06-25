#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <termios.h>
#include <sys/wait.h>
#include <unistd.h>
#include "modules/utils.hpp"

using namespace std;
namespace filesystem = std::filesystem;

#ifdef _WIN32
char separator = ';';
#else
char separator = ':';
#endif

struct termios old, raw; // for storing terminal's old & raw/modified settings

void enableRawMode()
{
  struct termios raw;
  tcgetattr(STDIN_FILENO, &old);
  raw = old;
  raw.c_lflag &= ~(ICANON | ECHO); // disable canonical mode & echo
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode()
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &old);
}

std::string readLine()
{
  std::string buffer;
  char c;
  char prev_c;
  while (read(STDIN_FILENO, &c, 1) == 1)
  {
    // check for builtin_commands
    vector<string> builtin_commands = {"echo", "exit", "pwd", "type"};
    if (c == '\t')
    {

      vector<string> input = tokenize(buffer);

      std::vector<std::string> matches;
      std::vector<std::string> directories_matches;
      if (input.size() > 1 || ((input.size() == 1 && buffer.back() == ' ')))
      {
        string to_complete;
        if (buffer.back() == ' ')
        {
          to_complete = "";
        }
        else
        {
          to_complete = input.back();
        }
        string dir;
        string prefix;

        auto pos = to_complete.find_last_of('/');

        if (pos == string::npos)
        {
          dir = ".";
          prefix = to_complete;
        }
        else
        {
          dir = to_complete.substr(0, pos);
          prefix = to_complete.substr(pos + 1);
        }
        if (!filesystem::exists(dir) ||
            !filesystem::is_directory(dir))
        {
          continue;
        }
        for (const auto &entry : filesystem::directory_iterator(dir))
        {
          auto file_name = entry.path().filename().string();
          if (entry.is_regular_file())
          {
            if (file_name.starts_with(prefix))
            {
              matches.push_back(file_name);
            }
          }
          else if (entry.is_directory())
          {
            if (file_name.starts_with(prefix))
              directories_matches.push_back(file_name);
          }
        }

        size_t total = matches.size() + directories_matches.size();

        if (total == 0)
        {
          write(STDOUT_FILENO, "\a", 1);
        }
        else if (total == 1)
        {
          bool is_dir = !directories_matches.empty();
          string to_write = matches.empty() ? directories_matches[0] : matches[0];
          string remaining = to_write.substr(prefix.size());
          remaining.push_back(is_dir ? '/' : ' ');
          buffer.append(remaining);
          write(STDOUT_FILENO, remaining.c_str(), remaining.size());
          prev_c = '\0';
          continue;
        }
        else
        {
          // std::vector<std::string> all_matches = matches;
          // all_matches.insert(all_matches.end(), directories_matches.begin(), directories_matches.end());
          std::vector<std::pair<std::string, bool>> all_entries;
          vector<string> all_entries_string;
          for (const auto &i : matches)
          {
            all_entries.push_back({i, false});
          }
          for (const auto &i : directories_matches)
          {
            all_entries.push_back({i, true});
          }

          sort(all_entries.begin(), all_entries.end());

          for (auto &[name, is_dir] : all_entries)
            all_entries_string.push_back(name);

          string lcp = get_lcp(all_entries_string[0], all_entries_string);

          if (lcp.size() > prefix.size())
          {
            string remaining = lcp.substr(prefix.size());
            write(STDOUT_FILENO, remaining.c_str(), remaining.size());
            buffer.append(remaining); 
          }
          else
          {
            if (prev_c != '\t')
            {
              write(STDOUT_FILENO, "\a", 1); // FIRST tab on ambiguous match -> exactly one beep
            }
            else
            {
              write(STDOUT_FILENO, "\n", 1); // SECOND tab -> no beep, list instead

              for (auto &[m, n] : all_entries)
              {
                write(STDOUT_FILENO, m.c_str(), m.size());
                if (n)
                  write(STDOUT_FILENO, "/", 1);
                write(STDOUT_FILENO, "  ", 2);
              }
              write(STDOUT_FILENO, "\n$ ", 3);
              write(STDOUT_FILENO, buffer.c_str(), buffer.size());
            }
          }
        }
      }
      else
      {

        for (auto &cmd : builtin_commands)
        {
          if (cmd.starts_with(buffer))
          {
            matches.push_back(cmd);
          }
        }

        // check for executables in $PATH variable
        const char *ch = getenv("PATH");
        if (ch)
        {
          // process the string
          stringstream ss(ch);
          string dir;
          while (getline(ss, dir, separator))
          {
            if (dir.empty())
              continue;

            // 1. check if its a file itself
            filesystem::path p = dir;
            if (filesystem::is_regular_file(p))
            {
              if (is_executable(p) && p.filename().string().starts_with(buffer))
              {
                std::string name_of_file = p.filename().string();
                matches.push_back(name_of_file);
              }
            }

            // 2. check if its a directory, check inside that directory, to find that executable.
            std::error_code ec;
            for (const auto &entry : filesystem::directory_iterator(dir, ec))
            {
              if (ec)
                break;
              std::string name =
                  entry.path().filename().string();
              if (is_executable(entry) && name.starts_with(buffer))
              {
                std::string name_of_file = name;
                matches.push_back(name_of_file);
              }
            }
          }
        }

        sort(matches.begin(), matches.end());
        matches.erase(
            unique(matches.begin(), matches.end()),
            matches.end());

        if (matches.size() == 0)
        {
          write(STDOUT_FILENO, "\a", 1);
        }
        else if (matches.size() == 1)
        {
          std::string match = matches[0];
          std::string remaining = match.substr(buffer.size());
          remaining.push_back(' ');
          write(STDOUT_FILENO, remaining.c_str(), remaining.size());
          buffer = match;
          buffer.push_back(' ');
        }
        else if (matches.size() > 1)
        {
          string lcp = get_lcp(matches[0], matches);

          if (lcp.size() > buffer.size())
          {
            std::string remaining = lcp.substr(buffer.size());
            write(STDOUT_FILENO, remaining.c_str(), remaining.size());

            buffer = lcp;
            prev_c = '\0';
          }
          else
          {
            if (prev_c != '\t')
            {
              write(STDOUT_FILENO, "\a", 1);
            }
            else
            {
              cout << "\n";

              for (auto &i : matches)
              {
                cout << i << "  ";
              }
              write(STDOUT_FILENO, "\n$ ", 3);
              write(STDOUT_FILENO, buffer.c_str(), buffer.size());
            }
          }
        }
      }
    }
    else if (c == '\n')
    {
      write(STDOUT_FILENO, "\n", 1);
      break;
    }
    else if (c == 127)
    {
      // backspace
      if (!buffer.empty())
      {
        buffer.pop_back();
        write(STDOUT_FILENO, "\b \b", 3);
        // "\b"-> go one step back(move pointer to left)
        // " " -> replace by blank space
        // "\b" -> again move one step back
      }
    }
    else
    {
      buffer += c;
      write(STDOUT_FILENO, &c, 1);
    }
    prev_c = c;
  }
  return buffer;
}

int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  enableRawMode();
  atexit(disableRawMode);

  while (true)
  {
    write(STDOUT_FILENO, "$ ", 2);
    std::string s = readLine();
    // doing the left right trimming
    trim(s);
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

    // condition checking
    if (cmd == "echo")
    {
      bool stdout_redirect_operator = false;
      bool error_redirect_operator = false;
      bool stdout_append_operator = false;
      bool error_append_operator = false;
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
        else if (input[i] == "2>>")
        {
          error_append_operator = true;
        }
        else if (input[i - 1] == ">" || input[i - 1] == "1>" || input[i - 1] == "2>" || input[i - 1] == "1>>" || input[i - 1] == ">>" || input[i - 1] == "2>>")
        {
          output_file = input[i];
        }
        else if ((input[i - 1] != ">" && input[i] != ">") && (input[i - 1] != "1>" && input[i] != "1>" && input[i - 1] != "2>" && input[i - 1] != "1>>" && input[i] != ">>" && input[i - 1] != "2>>" && input[i] != "2>>"))
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
      else if (error_append_operator)
      {
        std::ofstream hFile(output_file, std::ios::app);
        if (!hFile.is_open())
        {
          std::cerr << "Error opening file." << std::endl;
          continue;
        }
        cout << to_write_input << endl;
      }
      else
        cout << str << endl;
    }
    else if (cmd == "type")
    {
      // string str;
      // getline(ss, str);
      if (str == "type" || str == "echo" || str == "exit" || str == "pwd" || str == "cd" || str =="complete")
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
      bool error_append_operator = false;

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
        else if (input[i] == "2>>")
        {
          error_append_operator = true;
        }
        else if (input[i - 1] == ">" || input[i - 1] == "1>" || input[i - 1] == "2>" || input[i - 1] == "2>>")
        {
          output_file_arg = input[i];
        }
        else if (input[i] != ">" && input[i] != "1>" && input[i] != "2>" && input[i] != "2>>")
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
      else if (error_append_operator)
      {
        std::ofstream output_file(output_file_arg, std::ios::app);
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
      bool error_append_operator = false;

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
        else if (input[i] == "2>>")
        {
          error_append_operator = true;
        }
        else if (input[i] == ">>" || input[i] == "1>>")
        {
          stdout_append_operator = true;
        }
        else if (input[i - 1] == ">" || input[i - 1] == "1>" || input[i - 1] == "2>" || input[i - 1] == "1>>" || input[i - 1] == ">>" || input[i - 1] == "2>>")
        {
          output_file_arg = input[i];
        }
        else if ((input[i - 1] != ">" && input[i] != ">") && (input[i - 1] != "1>" && input[i] != "1>" && input[i - 1] != "2>" && input[i] != "2>" && input[i - 1] != "1>>" && input[i] != "1>>" && input[i - 1] != ">>" && input[i] != ">>" && input[i - 1] != "2>>" && input[i] != "2>>"))
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
      else if (error_append_operator)
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

      // FOR USING SYSTEM()
      // create a system() compatible command, which will have "" dquotes, around the exe file name, along with the arguments after that.
      // string to_execute_command = "\"" + exe_name + "\"" + " " + str;

      // pid > 0 → Parent process receives the child's PID.
      // pid == 0 → Child process.
      // pid < 0 → Fork failed.

      if (executable(cmd, 0))
      {

        std::vector<char *> c_args;
        c_args.reserve(input.size() + 1);

        for (auto &i : input)
        {
          c_args.push_back(const_cast<char *>(i.c_str()));
        }

        // CRITICAL: The array MUST be null-terminated
        c_args.push_back(nullptr);

        // FOR USING FORK(),EXEC()
        pid_t pid = fork();

        if (pid == -1)
        {
          std::cerr << "Fork failed\n";
        }
        else if (pid == 0)
        {
          // Child process
          execvp(c_args[0], c_args.data());

          // If execvp returns, it must have failed
          perror("execvp");
          // after fork(), using exit() can flush buffered streams inherited from the parent, so use _exit() instead
          _exit(1);
        }
        else
        {
          // Parent process (the shell)
          int status;
          waitpid(pid, &status, 0); // Wait for the child to finish
        }
      }
      else
      {
        cout << s << ": command not found" << endl;
      }
    }
  }
}
