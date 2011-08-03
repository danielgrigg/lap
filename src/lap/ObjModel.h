#ifndef OBJ_IMPORT_HPP
#define OBJ_IMPORT_HPP

#include <iosfwd>
#include <set>
#include <stdint.h>
#include <string>
#include <tr1/memory>
#include <tr1/unordered_map>
#include <vector>
#include "MeshMath.h"
#include "MaterialAsset.h"

namespace lap {
  namespace obj {

    using std::set;

  class Model;
  typedef std::tr1::shared_ptr<Model> ModelPtr;

    enum VertexFormat
    {
      kNone,
      kPosition,
      kPositionUV,
      kPositionNormal,
      kPositionUVNormal,
      kVertexFormatMax
    };

    std::string normalizeMaterialName(const std::string& material);
    std::string normalizeGroupName(const std::string& group);

    class Model
    {
      public:
        void addPosition(const float3& v) { _positions.push_back(v); }
        void addNormal(const float3& n) { _normals.push_back(n); }
        void addUV(const float2& uv) { _uvs.push_back(uv); }

        const MaterialMap& materials()const { return _materials; }

        const std::string& name()const { return _name; }
        VertexFormat vertexFormat()const 
        {
          if (!(_positions.empty() || _uvs.empty() || _normals.empty()))
          {
            return kPositionUVNormal;
          }
          if (!(_positions.empty() || _normals.empty()))
          {
            return kPositionNormal;
          }
          if (!(_positions.empty() || _uvs.empty()))
          {
            return kPositionUV;
          }
          if (!_positions.empty())
          {
            return kPosition;
          }
          return kNone;
        }

        const std::vector<float3>& positions()const { return _positions; }
        const std::vector<float3>& normals()const { return _normals; }
        const std::vector<float2>& uvs()const { return _uvs; }
        const std::vector<uint32_t>& faceIndices()const { return _faceIndices; }

        uint32_t numTriangles()const 
        { return _faceIndices.size () / numComponents() / 3; }

        void addMaterial(const Material& m) { _materials[m.name()] = m; }

        std::vector<uint32_t> _faceIndices;
        std::vector<Group> _geometryGroups;
        std::vector<Group> _materialGroups;
        MaterialMap _materials;
        std::string _name;

      private:
        std::vector<float3> _positions;
        std::vector<float2> _uvs;
        std::vector<float3> _normals;

        uint32_t numComponents()const
        {
          uint32_t components = 1;
          if (!_normals.empty()) components += 1;
          if (!_uvs.empty()) components += 1;
          return components;
        }
    };

    std::ostream& operator<<(std::ostream& os, VertexFormat vf);

    std::ostream& operator<<(std::ostream& os, const Model& rhs);

    class MtlTranslator
    {
      public:
        bool importFile(const std::string& filename, MaterialMap* found);
        bool exportFile(const ModelPtr& model, const std::string& filename);
      private:
        void parseLine(char* line);

        std::string _working;
        Material& working() { return (*_materials)[_working]; }
        MaterialMap* _materials;
  };

  class ObjTranslator
  {
    public:
      ModelPtr importFile(const std::string& filename);
      bool exportFile(const ModelPtr& model, const std::string& filename);

    private:
      int parseCluster(std::istream& cluster);
      void parseLine(char* line);
      uint32_t parseFace(char* context);
      ModelPtr _model;
      std::string mtllib; // Obj-format token for a Obj-material file.

      Group* geometryGroup() 
      { 
        return _model->_geometryGroups.empty() ? NULL :  &(_model->_geometryGroups.back()); 
      }

      Group* materialGroup() 
      { 
        return _model->_materialGroups.empty() ? NULL : &(_model->_materialGroups.back()); 
      }

      void addGeometryGroup(const std::string& name)
      {
        uint32_t start = _model->_geometryGroups.empty() ? 0 : 
          _model->_geometryGroups.back().end();
        _model->_geometryGroups.push_back(Group(name, start));
      }

      void addMaterialGroup(const std::string& material)
      {
        uint32_t start = _model->_materialGroups.empty() ? 0 : 
          _model->_materialGroups.back().end();
        _model->_materialGroups.push_back(
            Group(normalizeMaterialName(material), start));
      }
  };
}
}

#endif
