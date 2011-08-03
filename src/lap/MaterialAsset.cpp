#include "MaterialAsset.h"
#include <iostream>

namespace lap {
  std::ostream& operator<<(std::ostream& os, const Material& rhs)
  {
    os << rhs._name << "\n";
    os << "illum 4\n";
    os << "Kd "; writeVec(os, rhs.Kd); os << '\n';
    os << "Ka "; writeVec(os, rhs.Ka); os << '\n';
    os << "Tf "; writeVec(os, rhs.Tf); os << '\n';
    os << "Ni " << rhs.Ni << '\n';
    os << "d " << rhs.d << '\n';
    os << "Ns " << rhs.Ns << '\n';
    os << "Ks "; writeVec(os, rhs.Ks); os << '\n';
    if (!rhs.map_Ka.empty()) os << "map_Ka " << rhs.map_Ka << '\n';
    if (!rhs.map_Kd.empty()) os << "map_Kd " << rhs.map_Kd << '\n';
    if (!rhs.map_Ks.empty()) os << "map_Ks " << rhs.map_Ks << '\n';
    return os;
  }

}

