#include "ObjModel.h"
#include <sstream>
#include <fstream>
#include <cassert>
#include <iostream>
#include <iterator>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp
#include <boost/filesystem/fstream.hpp>    // ditto
#include <boost/filesystem/convenience.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/function.hpp>

using namespace boost::lambda;
using namespace boost;

namespace lap {
namespace obj {
  inline bool CStringEqual(const char* a, const char* b)
  {
    return strcmp(a, b) == 0;
  }

  std::string normalizeMaterialName(const std::string& material)
  {
    return replace_all_copy(material.substr(0, material.rfind("__Grp")), " ", "_");
  }
  std::string normalizeGroupName(const std::string& group)
  {
    return replace_all_copy(group, " ", "_");
  }

  void MtlTranslator::parseLine(char* line)
  {
    char* context;
    char* token = strtok_r(line, " ", &context);
    if (token == NULL) return;

    if (token[0] == '#') return;

    if (CStringEqual(token, "newmtl"))
    {
      _working = normalizeMaterialName(strtok_r(NULL,"\n",&context));
      (*_materials)[_working] = Material(_working);
    }
    else if (CStringEqual(token, "illum"))
    {
      //      std::cerr << "illum unsupported - skipping" << std::endl;
    }
    else if (CStringEqual(token, "Kd"))
    {
      working().Kd = parseVec<3>(context);
    }
    else if (CStringEqual(token, "Ka"))
    {
      working().Ka = parseVec<3>(context);
    }
    else if (CStringEqual(token, "Tf"))
    {
      working().Tf = parseVec<3>(context);
    }
    else if (CStringEqual(token, "Ni"))
    {
      working().Ni = strtof(strtok_r(NULL, "\n", &context), NULL);
    }
    else if (CStringEqual(token, "Ns"))
    {
      working().Ns = strtof(strtok_r(NULL, "\n", &context), NULL);
    }
    else if (CStringEqual(token, "Ks"))
    {
      working().Ks = parseVec<3>(context);
    }
    else if (CStringEqual(token, "map_Ka"))
    {
      working().map_Ka = strtok_r(NULL, "\n", &context);
    }
    else if (CStringEqual(token, "map_Kd"))
    {
      working().map_Kd = strtok_r(NULL, "\n", &context);
    }
    else if (CStringEqual(token, "map_Ks"))
    {
      working().map_Ks = strtok_r(NULL, "\n", &context);
    }
    else
    {
      std::cerr << "ObjMaterial import error: unrecognised token '" 
        << token << "'\n";
    }
  }

  bool MtlTranslator::importFile(const std::string& filename, MaterialMap* found)
  {
    std::fstream fs (filename.c_str(), std::fstream::in);
    if (!fs.is_open() || !found) return false;
    _materials = found;

    char line[256];
    while (fs.getline(line, 256))
    {
      parseLine(line);
    }
    fs.close();
    return true;
  }

  bool MtlTranslator::exportFile(const ModelPtr& model, const std::string& filename)
  {
    if (!model) return false;
    std::fstream fs(filename.c_str(), std::fstream::out);
    for_each(model->materials().begin(), model->materials().end(), 
        fs << constant("newmtl ") << bind(&MaterialMap::value_type::second, cref(_1)));
    return true;
  }

  bool ObjTranslator::exportFile(const ModelPtr& model, const std::string& filename)
  {
    if (!model) return false;

    boost::filesystem::path outPath(filename);
    std::fstream fs(filename.c_str(), std::fstream::out);

    fs << "mtllib " << outPath.stem().string() << ".mtl\n";
    fs << *model;
    fs.close();

    MtlTranslator mt;
    outPath.replace_extension(".mtl");
    return mt.exportFile(model, outPath.string());
  }

  int ObjTranslator::parseCluster(std::istream& cluster)
  {
    // Faces can be:
    // a) Vertex only (f V)
    // b) Vertex and UV (f V/T)
    // c) Vertex and Normal (f V//N)
    // d) Vertex, UV and Normal (f V/T/N)
    // We ignore the ordering here because it's dependent on 
    // what vertex data has been parsed.
    char buffer[128];
    int found = 0;
    while (cluster.getline(buffer, 128, '/'))
    {
      uint32_t idx = strtol(buffer, NULL, 10);
      if (idx > 0) 
      {
        _model->_faceIndices.push_back(idx-1);
        ++found;
      }
    }
    return found;
  }

  void ObjTranslator::parseLine(char* line)
  {
    char* context ;
    char* token = strtok_r(line, " ", &context);
    if (token == NULL) return;

    if (token[0] == '#') return;

    if (CStringEqual(token, "v"))
    {
      _model->addPosition(parseVec<3>(context));
    }
    else if (CStringEqual(token, "vt"))
    {
      _model->addUV(parseVec<2>(context));
    }
    else if (CStringEqual(token, "vn"))
    {
      _model->addNormal(parseVec<3>(context));
    }
    else if (CStringEqual(token, "g"))
    {
      std::string groupName = normalizeGroupName(strtok_r(NULL, "\n", &context));
      if (groupName != "default")
      {
        addGeometryGroup(groupName);
      }
    }
    else if (CStringEqual(token, "mtllib"))
    {
      mtllib = strtok_r(NULL, "\n", &context);
    }
    else if (CStringEqual(token, "usemtl"))
    {
      addMaterialGroup(strtok_r(NULL, "\n", &context));
    }
    else if (CStringEqual(token, "f"))
    {
      uint32_t indicesAdded = parseFace(context);
      if (materialGroup()) materialGroup()->setCount(materialGroup()->count() + indicesAdded);
      if (geometryGroup()) geometryGroup()->setCount(geometryGroup()->count() + indicesAdded);
    }
  } 

  uint32_t ObjTranslator::parseFace(char* context)
  {
    int found = 0;
    char* c = strtok_r(NULL, " ", &context);
    int sizes[3] = {0,0,0};
    uint32_t indicesBefore = _model->faceIndices().size();
    while (c != NULL && found < 3) 
    {
      std::istringstream ss(c);
      sizes[found] = parseCluster(ss);
      c = strtok_r(NULL, " ", &context);
      ++found;
    }
    // Face is a quad, triangulate it now.
    if (c != NULL)
    {
      uint32_t faceSize = sizes[0]+sizes[1]+sizes[2];
      // cluster parser will push the last vertex
      _model->_faceIndices.resize(_model->faceIndices().size() + sizes[0] + sizes[1]);
      uint32_t* p0 = &_model->_faceIndices[_model->faceIndices().size()] - faceSize -
        sizes[0] - sizes[1];
      uint32_t* p1 = p0 + sizes[0];
      uint32_t* p2 = p1 + sizes[1];
      uint32_t* p3 = p2 + sizes[2];
      uint32_t* p4 = p3 + sizes[0];

      std::copy(p0, p1, p3);
      std::copy(p2, p3, p4);
      std::istringstream ss(c);
      parseCluster(ss);
    }
    uint32_t indicesAdded =  (_model->faceIndices().size() - indicesBefore);
    return indicesAdded;
  }

  ModelPtr ObjTranslator::importFile(const std::string& filename)
  {
    _model = ModelPtr(new Model());
    boost::filesystem::path objPath(filename);
    std::fstream fs (filename.c_str(), std::fstream::in);
    if (!fs.is_open()) return ModelPtr();
    char line[256];
    while (fs.getline(line, 256))
    {
      parseLine(line);
    }
    fs.close();
    std::sort(_model->_geometryGroups.begin(), _model->_geometryGroups.end());
    std::sort(_model->_materialGroups.begin(), _model->_materialGroups.end());

    _model->_name = objPath.stem().string();
    boost::filesystem::path mtlPath(objPath.parent_path() / mtllib);
    MtlTranslator mt;
    MaterialMap importedMaterials;
    if (!mt.importFile(mtlPath.string(), &importedMaterials))
    {
      std::cerr << "error importing mtl " << mtlPath << std::endl;
      return ModelPtr();
    }
    for (std::vector<Group>::const_iterator i = _model->_materialGroups.begin();
        i != _model->_materialGroups.end(); ++i)
    {
      _model->addMaterial(importedMaterials[i->name()]);
    }
    return _model;
  }

  void writeIndexP(std::ostream& os, const uint1& index)
  {
    os << index[0]+1;
  }

  void writeIndexPN(std::ostream& os, const uint2& index)
  {
    os << index[0]+1 << "//" << index[1]+1;
  }

  void writeIndexPT(std::ostream& os, const uint2& index)
  {
    os << index[0]+1 << "/" << index[1]+1;
  }

  void writeIndexPTN(std::ostream& os, const uint3& index)
  {
    os << index[0]+1 << "/" << index[1]+1 << '/' << index[2]+1;
  }

  Group styleObjGroup(const Group& g)
  {
    return Group(std::string("g ") + g.name(), g.begin(), g.count());
  }

  Group styleObjMaterial(const Group& g)
  {
    return Group(std::string("usemtl ") + g.name(), g.begin(), g.count());
  }

  template <typename I>
    void writeFacesGeneric(std::ostream& os, const uint32_t* start, uint32_t count,
        boost::function<void (std::ostream&, I const&)> vertexWriter)
    {
      Range<I> is = make_range<I>(start, count);
      for (const I* p = is.begin(); p != is.end(); p += 3)
      {
        os << "f "; vertexWriter(os, p[0]);
        os << ' '; vertexWriter(os, p[1]);
        os << ' '; vertexWriter(os, p[2]);
        os << '\n';
      }
    }

  void writeFaces(std::ostream& os, VertexFormat vf, 
      const uint32_t* start, uint32_t count)
  {
    switch (vf)
    {
      case kPositionUVNormal: 
        writeFacesGeneric<uint3>(os, start, count, writeIndexPTN);
        break;

      case kPosition: 
        writeFacesGeneric<uint1>(os, start, count, writeIndexP);
        break;

      case kPositionUV: 
        writeFacesGeneric<uint2>(os, start, count, writeIndexPT);
        break;

      case kPositionNormal: 
        writeFacesGeneric<uint2>(os, start, count, writeIndexPN);
        break;
      default:
        break;
    }
  }

  std::ostream& operator<<(std::ostream& os, const Model& rhs)
  {
    if (rhs.positions().empty()) return os;

    if (!rhs._geometryGroups.empty()) os << "g default\n";

    std::for_each(rhs.positions().begin(), rhs.positions().end(), 
        bind(writeVec<float,3>, ref(os), cref(_1), "v ", "\n"));
    std::for_each(rhs.uvs().begin(), rhs.uvs().end(), 
        bind(writeVec<float,2>, ref(os), cref(_1), "vt ", "\n"));
    std::for_each(rhs.normals().begin(), rhs.normals().end(), 
        bind(writeVec<float,3>, ref(os), cref(_1), "vn ", "\n"));

    std::vector<Group> groups;
    groups.reserve(rhs._geometryGroups.size() + rhs._materialGroups.size());
    std::transform(rhs._geometryGroups.begin(), rhs._geometryGroups.end(), 
        back_inserter(groups), bind(styleObjGroup, ref(_1)));
    std::transform(rhs._materialGroups.begin(), rhs._materialGroups.end(), 
        back_inserter(groups), bind(styleObjMaterial, ref(_1)));
    sort(groups.begin(), groups.end());

    VertexFormat vf = rhs.vertexFormat();
    for (std::vector<Group>::const_iterator g = groups.begin();
        g != groups.end(); ++g)
    {
      uint32_t count = (g+1) == groups.end() ? g->count() : (g+1)->begin() - g->begin();
      os << g->name() << '\n';
      writeFaces(os, vf, &rhs.faceIndices()[0] + g->begin(), count);
    }

    if (groups.empty())
    {
      writeFaces(os, vf, &rhs.faceIndices()[0], rhs.faceIndices().size());
    }
    return os;
  }

  std::ostream& operator<<(std::ostream& os, VertexFormat vf)
  {
    switch (vf)
    {
      case kNone: os << "kNone"; break;
      case kPosition: os << "kPosition"; break;
      case kPositionUV: os << "kPositionUV"; break;
      case kPositionNormal: os << "kPositionNormal"; break;
      case kPositionUVNormal: os << "kPositionUVNormal"; break;
      case kVertexFormatMax: os << "kVertexFormatMax"; break;
      default: os << "Invalid"; break;
    }
    return os;
  }
}
}
