#ifndef LAP_MATERIAL_ASSET_H
#define LAP_MATERIAL_ASSET_H

#include <iosfwd>
#include <string>
#include <tr1/unordered_map>
#include "MeshMath.h"

namespace lap {

  // Only support obj-style materials for now.
  struct Material
  {
    Material() { init(); }
    Material(const std::string& name):_name(name){ init(); }

    void init() 
    { 
      Ni = 1.0f;
      Ns = 1.0f; 
      d = 1.0f;
    }
    const std::string& name()const { return _name; }
    void setName(const std::string& s) { _name = s; }

    // For simplicity we store copies of all standard MTL values
    // and maps. Maps, if given, override non-maps.
    float3 Kd; // Diffuse-coefficient (RGB 0-1)
    float Ni; // Refraction index (0.001 - 10)
    float3 Ka; // Ambient-coefficient (RGB 0-1)
    float d; // Dissolve (0-1)
    float3 Tf; // Transmission-filter (RGB 0-1)
    float Ns; // Specular-exponent (0 - 1000)
    float3 Ks; // Specular-coefficient (RGB) 0-1
    // illum unsupported
    std::string _name;
    std::string map_Ka;
    std::string map_Kd;
    std::string map_Ks;

  };

  typedef std::tr1::unordered_map<std::string, Material> MaterialMap;

  std::ostream& operator<<(std::ostream& os, const Material& rhs);

  inline bool operator<(const Material& lhs, const Material& rhs)
  {
    return lhs.name() < rhs.name();
  }
  inline bool operator==(const Material& lhs, const Material& rhs)
  {
    return lhs.name() == rhs.name();
  }
}
#endif

