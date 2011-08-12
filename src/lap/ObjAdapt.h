#ifndef LAP_OBJ_ADAPT_H
#define LAP_OBJ_ADAPT_H

#include "MeshAsset.h"

namespace lap {

  template <typename V>
    obj::ModelPtr objFromMesh(const shared_ptr<Mesh<V> > mesh);

  template <typename V>
    shared_ptr<Mesh<V> > meshFromObj(const obj::ModelPtr& model);

  template <> shared_ptr<Mesh<VertexP> > meshFromObj(const obj::ModelPtr& model);
  template <> shared_ptr<Mesh<VertexPT> > meshFromObj(const obj::ModelPtr& model);
  template <> shared_ptr<Mesh<VertexPN> > meshFromObj(const obj::ModelPtr& model);
  template <> shared_ptr<Mesh<VertexPTN> > meshFromObj(const obj::ModelPtr& model);

  template <typename V>
    shared_ptr<Mesh<V> > indexedMeshFromObj(const obj::ModelPtr& model);
  template <> shared_ptr<Mesh<VertexP> > indexedMeshFromObj(const obj::ModelPtr& model);
  template <> shared_ptr<Mesh<VertexPT> > indexedMeshFromObj(const obj::ModelPtr& model);
  template <> shared_ptr<Mesh<VertexPN> > indexedMeshFromObj(const obj::ModelPtr& model);
  template <> shared_ptr<Mesh<VertexPTN> > indexedMeshFromObj(const obj::ModelPtr& model);

  template<typename V, typename A>
    void objVertices(const shared_ptr<Mesh<V> >& mesh, 
        boost::function<void (A)> vertexGen,
        boost::function<A (V const&)> getAttrib,
        boost::function<void (uint32_t, uint32_t)> indexGen)
    {
      typedef unordered_map<A, uint32_t> AttribIndexer;
      AttribIndexer indexer;
      uint32_t largestIndex = 0;
      for (uint32_t v = 0; v < mesh->vertices().size(); ++v)
      {
        A attrib = getAttrib(cref(mesh->vertices()[v]));
        pair<typename AttribIndexer::iterator, bool> pib = 
          indexer.insert(typename AttribIndexer::value_type(attrib, largestIndex));
        if (pib.second)
        {
          vertexGen(attrib);
          indexGen(v, largestIndex++);
        }
        else
        {
          indexGen(v, pib.first->second);
        }
      }
    }

  inline Group objGroupFromMesh(const Group& meshGroup, uint32_t components)
  {
    return Group(meshGroup.name(), meshGroup.begin() * components, 
        meshGroup.count() * components);
  }

  template <typename V, typename I>
    void adaptGroupsToObj(const shared_ptr<Mesh<V> >& mesh, obj::ModelPtr& model)
    {
      adaptGroups<I>(mesh->_geometryGroups, model->_geometryGroups, objGroupFromMesh);
      adaptGroups<I>(mesh->_materialGroups, model->_materialGroups, objGroupFromMesh);
    }

  template<typename I>
    void setRangeAttrib(Range<I>& is, uint32_t attrib, uint32_t index, uint32_t value)
    {
      is[index][attrib] = value;
    }


  inline void objFromMeshImp(const MeshPPtr mesh,
      obj::ModelPtr& model)
  {
    Range<uint1> is = allocateRange<uint1>(model->_faceIndices, mesh->vertices().size());
    objVertices<VertexP, float3>(mesh, 
        bind(&obj::Model::addPosition, model.get(), _1),
        position<VertexP>, 
        bind(setRangeAttrib<uint1>, ref(is), 0, _1, _2));
    adaptGroupsToObj<VertexP, uint1>(mesh, model);
  }

  inline void objFromMeshImp(const MeshPNPtr mesh, 
      obj::ModelPtr& model)
  {
    Range<uint2> is = allocateRange<uint2>(model->_faceIndices, mesh->vertices().size());
    objVertices<VertexPN, float3>(mesh, 
        bind(&obj::Model::addPosition, model.get(), _1),
        position<VertexPN>, 
        bind(setRangeAttrib<uint2>, ref(is), 0, _1, _2));

    objVertices<VertexPN, float3>(mesh, 
        bind(&obj::Model::addNormal, model.get(), _1),
        normal<VertexPN>, 
        bind(setRangeAttrib<uint2>, ref(is), 1, _1, _2));
    adaptGroupsToObj<VertexPN, uint2>(mesh, model);
  }

  inline void objFromMeshImp(const MeshPTPtr mesh, 
      obj::ModelPtr& model)
  {
    Range<uint2> is = allocateRange<uint2>(model->_faceIndices, mesh->vertices().size());
    objVertices<VertexPT, float3>(mesh, 
        bind(&obj::Model::addPosition, model.get(), _1),
        position<VertexPT>, 
        bind(setRangeAttrib<uint2>, ref(is), 0, _1, _2));

    objVertices<VertexPT, float2>(mesh, 
        bind(&obj::Model::addUV, model.get(), _1),
        uv<VertexPT>, 
        bind(setRangeAttrib<uint2>, ref(is), 1, _1, _2));
    adaptGroupsToObj<VertexPT, uint2>(mesh, model);
  }

  inline void objFromMeshImp(const MeshPTNPtr mesh, 
      obj::ModelPtr& model)
  {
    Range<uint3> is = allocateRange<uint3>(model->_faceIndices, mesh->vertices().size());
    objVertices<VertexPTN, float3>(mesh, 
        bind(&obj::Model::addPosition, model.get(), _1),
        position<VertexPTN>, 
        bind(setRangeAttrib<uint3>, ref(is), 0, _1, _2));

    objVertices<VertexPTN, float2>(mesh, 
        bind(&obj::Model::addUV, model.get(), _1),
        uv<VertexPTN>, 
        bind(setRangeAttrib<uint3>, ref(is), 1, _1, _2));

    objVertices<VertexPTN, float3>(mesh, 
        bind(&obj::Model::addNormal, model.get(), _1),
        normal<VertexPTN>, 
        bind(setRangeAttrib<uint3>, ref(is), 2, _1, _2));
    adaptGroupsToObj<VertexPTN, uint3>(mesh, model);
  }

  //! Make an obj-model from a Mesh.
  template <typename V>
    obj::ModelPtr objFromMesh(const shared_ptr<Mesh<V> > mesh)
    {
      assert(mesh->_indices.empty());
      obj::ModelPtr model(new obj::Model());
      objFromMeshImp(mesh, model);
      model->_materials.insert(mesh->materials().begin(), mesh->materials().end());
      return model;
    }
}

#endif
