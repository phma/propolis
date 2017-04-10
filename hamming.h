/******************************************************/
/*                                                    */
/* hamming.h - Hamming codes                          */
/*                                                    */
/******************************************************/

#include <vector>

class Hamming
{
protected:
  std::vector<signed char> code;
  /* signed char is used for belief propagation, where the soft bit ranges
   * from -127 for 1 to 127 for 0. -127 means 1 so that * can mean xor.
   * code can also contain hard bits, normally one per byte.
   */
public:
  std::vector<signed char> getCode();
  void setCode(std::vector<signed char> Code);
  void push_back(char byte);
};
