#include "one.h"
#include <fstream>
#include <string>
namespace neko
{
    using oneIof = one::one<std::fstream,std::string>; 
} // namespace neko
