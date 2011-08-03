#include "MeshMath.h"

namespace lap
{

 std::ostream& operator<<(std::ostream& os, const Group& rhs)
  {
    os << rhs.name() << '[' << rhs.begin() << ' ' << rhs.end() << ']';
    return os;
  }

}

