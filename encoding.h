#include <string>
#include <vector>

struct encoded
{
  int encoding;
  std::string codestring;
};

std::vector<encoded> encodedlist(std::string text);
