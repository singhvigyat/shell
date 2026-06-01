#include <iostream>
#include <string>
using namespace std; 

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage

  while(true){
    std::cout << "$ ";
    std::string s; 
    std::getline(cin, s); 
    
    cout<<s; 
    cout<<": command not found"<< endl; 
  }

}
