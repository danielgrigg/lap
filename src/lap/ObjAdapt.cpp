#include "ObjAdapt.h"

namespace lap {

  typedef unordered_map<uint64_t, uint32_t> IndexMap;

  VertexP makeVertexP(const obj::ModelPtr& obj, uint1 index)
  {
    return VertexP(obj->positions()[index[0]]);
  }

  VertexPTN makeVertexPTN(const obj::ModelPtr& obj, uint3 index)
  {
    return VertexPTN(obj->positions()[index[0]],
        obj->uvs()[index[1]],
        obj->normals()[index[2]]);
  }

  VertexPN makeVertexPN(const obj::ModelPtr& obj, uint2 index)
  {
    return VertexPN(obj->positions()[index[0]], obj->normals()[index[1]]);
  }

  VertexPT makeVertexPT(const obj::ModelPtr& obj, uint2 index)
  {
    return VertexPT(obj->positions()[index[0]], obj->uvs()[index[1]]);
  }


  Group meshGroupFromObj(const Group& objGroup, uint32_t components)
  {
    return Group(objGroup.name(), objGroup.begin() / components, 
        objGroup.count() / components);
  }

  template<typename V, typename I>
  shared_ptr<Mesh<V> > meshFromObj(const obj::ModelPtr& obj,
        boost::function<V (obj::ModelPtr const&, I)> makeVertex)
  {
    shared_ptr<Mesh<V> > mesh(new Mesh<V>());
    Range<I> r = make_range<I>(&obj->_faceIndices[0], obj->_faceIndices.size());
    mesh->_vertices.reserve(r.count());
    transform(r.begin(), r.end(),
              back_inserter(mesh->_vertices), bind(makeVertex, ref(obj), _1));
    adaptGroups<I>(obj->_geometryGroups, mesh->_geometryGroups, meshGroupFromObj);
    adaptGroups<I>(obj->_materialGroups, mesh->_materialGroups, meshGroupFromObj);
    mesh->_materials = obj->materials();
    return mesh;
  }

  template <>
  shared_ptr<Mesh<VertexP> > meshFromObj(const obj::ModelPtr& obj)
  {
    return meshFromObj<VertexP, uint1>(obj, makeVertexP);
  }

  template <>
  shared_ptr<Mesh<VertexPT> > meshFromObj(const obj::ModelPtr& obj)
  {
    return meshFromObj<VertexPT, uint2>(obj, makeVertexPT);
  }

  template <>
  shared_ptr<Mesh<VertexPN> > meshFromObj(const obj::ModelPtr& obj)
  {
    return meshFromObj<VertexPN, uint2>(obj, makeVertexPN);
  }

  template <>
  shared_ptr<Mesh<VertexPTN> > meshFromObj(const obj::ModelPtr& obj)
  {
    return meshFromObj<VertexPTN, uint3>(obj, makeVertexPTN);
  }

  template <typename V, typename I>
    shared_ptr<Mesh<V> > indexedMeshFromObj(const obj::ModelPtr& obj,
        boost::function<uint64_t (I const& idx, obj::ModelPtr const& m)> keygen,
        boost::function<V (obj::ModelPtr const&, I)> vertexGen)
    {
      shared_ptr<Mesh<V> > mesh(new Mesh<V>());
      Range<I> is = make_range<I>(&obj->_faceIndices[0], obj->_faceIndices.size());
      mesh->_indices.reserve(is.count());
      uint32_t largestIndex = 0;
      IndexMap indexMap;
      for (const I* p = is.begin(); p != is.end(); ++p)
      {
        std::pair<IndexMap::iterator, bool> pib = 
          indexMap.insert(IndexMap::value_type(keygen(*p, obj), largestIndex));
        if (pib.second)
        {
          mesh->_vertices.push_back(vertexGen(ref(obj), (*p)));
          mesh->_indices.push_back(largestIndex++);
        }
        else
        {
          uint32_t priorIndex = pib.first->second;
          mesh->_indices.push_back(priorIndex);
        }
      }

      adaptGroups<I>(obj->_geometryGroups, mesh->_geometryGroups, meshGroupFromObj);
      adaptGroups<I>(obj->_materialGroups, mesh->_materialGroups, meshGroupFromObj);
      mesh->_materials = obj->materials();
      return mesh;
    }

  uint64_t keyPTN(const uint3& index, const obj::ModelPtr& obj)
  {
    uint64_t zStride = obj->uvs().size() * obj->normals().size();
    uint64_t yStride = obj->normals().size();
    return index[0] * zStride + index[1] * yStride + index[2];
  }
  uint64_t keyPT(const uint2& index, const obj::ModelPtr& obj)
  {
    uint64_t yStride = obj->uvs().size();
    return index[0] * yStride + index[1];
  }
  uint64_t keyPN(const uint2& index, const obj::ModelPtr& obj)
  {
    uint64_t yStride = obj->normals().size();
    return index[0] * yStride + index[1];
  }


  template <>
  shared_ptr<Mesh<VertexPT> > indexedMeshFromObj(const obj::ModelPtr& obj)
  {
    return indexedMeshFromObj<VertexPT, uint2>(obj, keyPT, makeVertexPT);
  }

  template <>
  shared_ptr<Mesh<VertexPN> > indexedMeshFromObj(const obj::ModelPtr& obj)
  {
    return indexedMeshFromObj<VertexPN, uint2>(obj, keyPN, makeVertexPN);
  }

  template <>
  shared_ptr<Mesh<VertexPTN> > indexedMeshFromObj(const obj::ModelPtr& obj)
  {
    return indexedMeshFromObj<VertexPTN, uint3>(obj, keyPTN, makeVertexPTN);
  }

  template <>
  shared_ptr<Mesh<VertexP> > indexedMeshFromObj(const obj::ModelPtr& obj)
  {
    shared_ptr<Mesh<VertexP> > mesh(new Mesh<VertexP>());
    mesh->_vertices.assign(obj->positions().begin(), obj->positions().end());
    mesh->_indices = obj->faceIndices();
    return mesh;
  }



}

