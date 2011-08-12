#include "MeshAsset.h"
#include <iostream>

namespace lap
{
  void sliceGroups(const vector<Group>& src, const Group& g, vector<Group>& sliced)
  {
    remove_copy_if(src.begin(), src.end(), back_inserter(sliced), 
        !bind(&Group::intersects, boost::cref(g), boost::cref(_1)));
    transform(sliced.begin(), sliced.end(), sliced.begin(), 
        bind(intersection, cref(_1), cref(g)));

    transform(sliced.begin(), sliced.end(), sliced.begin(),
          bind(makeOffsetGroup, boost::cref(g), boost::cref(_1)));
    sort(sliced.begin(), sliced.end());
  }

  std::ostream& operator<<(std::ostream& os, const VertexP& rhs)
  {
    os << rhs.position;
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const VertexPT& rhs)
  {
    os << '[' << rhs.position << ", " << rhs.uv << ']';
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const VertexPN& rhs)
  {
    os << '[' << rhs.position << ", " << rhs.normal << ']';
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const VertexPTN& rhs)
  {
    os << '[' << rhs.position << ", " << rhs.uv << ", " 
      << rhs.normal << ']';
    return os;
  }

}
