#include <map>
#include <string>
#include <iostream>

using std::string;

int main() {
  std::map<string, string> mapTest;
  mapTest.insert(std::pair<string, string>("one", "one_value"));
  mapTest.insert(std::pair<string, string>("one", "one_value_cover"));
  std::cout << "key -- value: " << "one" << " -- " << mapTest["one"] << std::endl;
  std::cout << "After modify: " << std::endl;
  mapTest["one"] = "one_value_cover";
  std::cout << "key -- value: " << "one" << " -- " << mapTest["one"] << std::endl;
  return 0;
}
