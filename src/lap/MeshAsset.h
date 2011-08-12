#ifndef LAP_MESH_ASSET_H
#define LAP_MESH_ASSET_H

#include <algorithm>
#include <boost/function.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <cassert>
#include <iosfwd>
#include <iterator>
#include <tr1/memory>
#include <tr1/unordered_map>
#include "MaterialAsset.h"
#include "MeshMath.h"
#include "ObjModel.h"

namespace lap
{
  using namespace boost::lambda;
  using namespace std::tr1;
  using namespace std;
  using boost::ref;
  using boost::cref;
  using std::tr1::unordered_map;
  template <typename V> struct Mesh;
  struct VertexP; typedef shared_ptr<Mesh<VertexP> > MeshPPtr;
  struct VertexPN; typedef shared_ptr<Mesh<VertexPN> > MeshPNPtr;
  struct VertexPT; typedef shared_ptr<Mesh<VertexPT> > MeshPTPtr;
  struct VertexPTN; typedef shared_ptr<Mesh<VertexPTN> > MeshPTNPtr;

  typedef vector<Group>::const_iterator GroupConstIter;
  typedef vector<Group>::iterator GroupIter;

  // Public 
  template <typename V>
    class Mesh
    {
      public:
        typedef shared_ptr<Mesh<V> > MeshPtr;
        Mesh(){}

        const V& vertexAtIndex(uint32_t index)const 
        { 
          return _vertices[index]; 
        }

        uint32_t triangles()const 
        { 
          return _indices.empty() ? _vertices.size() / 3: _indices.size() / 3; 
        }
        const std::vector<V>& vertices()const { return _vertices; }
        const MaterialMap& materials()const { return _materials; }
        const std::vector<uint32_t>& indices()const { return _indices; }

        GroupConstIter beginGeometryGroups()const { return _geometryGroups.begin(); }
        GroupConstIter endGeometryGroups()const { return _geometryGroups.end(); }
        GroupConstIter beginMaterialGroups()const { return _materialGroups.begin(); }
        GroupConstIter endMaterialGroups()const { return _materialGroups.end(); }

        //! Extract a mesh with geometry/material groups clipped to spec.
        MeshPtr slice(const Group& spec);

        //! Flattens the mesh into a single group with combined materials.
        MeshPtr flatten()const;

        std::vector<uint32_t> _indices;
        std::vector<V> _vertices;
        std::vector<Group> _geometryGroups;
        std::vector<Group> _materialGroups;
        MaterialMap _materials;
      private:
    };

  template<typename V>
    shared_ptr<Mesh<V> > indexedMeshFromMesh(shared_ptr<Mesh<V> > flatMesh);

  template<typename V>
    shared_ptr<Mesh<V> > meshFromIndexedMesh(shared_ptr<Mesh<V> > indexedMesh);

  struct VertexP
  {
    VertexP(){}
    VertexP(const float3& p): position(p) {}
    bool equals(const VertexP& rhs)const { return position.equals(rhs.position); }

    float3 position;
  };

  std::ostream& operator<<(std::ostream& os, const VertexP& rhs);

  struct VertexPN : public VertexP
  {
    VertexPN(){}
    VertexPN(const float3& p, const float3& n): 
      VertexP(p),
      normal(n)
    {}
    bool equals(const VertexPN& rhs)const 
    {
      return position.equals(rhs.position) && normal.equals(rhs.normal); 
    }

    float3 normal;
  };

  std::ostream& operator<<(std::ostream& os, const VertexPN& rhs);

  struct VertexPT : public VertexP
  {
    VertexPT(const float3& p, const float2& t): 
      VertexP(p),
      uv(t)
    {}

    bool equals(const VertexPT& rhs)const 
    {
      return position.equals(rhs.position) && uv.equals(rhs.uv); 
    }
    float2 uv;
  };
  std::ostream& operator<<(std::ostream& os, const VertexPT& rhs);

  struct VertexPTN : public VertexP
  {
    VertexPTN(){}
    VertexPTN(const float3& p, const float2& t, const float3& n): 
      VertexP(p),
      uv(t),
      normal(n)
    {}

    bool equals(const VertexPTN& rhs)const 
    {
      return position.equals(rhs.position) && 
        uv.equals(rhs.uv) && normal.equals(rhs.normal); 
    }
    float2 uv;
    float3 normal;
  };

  // Private
  //
  //
  inline Group makeOffsetGroup(const Group& base, const Group& offset)
  {
    return Group(offset.name(), offset.begin() - base.begin(), offset.count());
  }

  void sliceGroups(const vector<Group>& src, const Group& g, vector<Group>& sliced);

  template <typename V>
    shared_ptr<Mesh<V> > Mesh<V>::slice(const Group& spec)
    {
      assert(_indices.empty());
      MeshPtr mesh(new Mesh<V>());
      mesh->_vertices.assign(_vertices.begin() + spec.begin(), 
          _vertices.begin() + spec.end());
      sliceGroups(_geometryGroups, spec, mesh->_geometryGroups);
      sliceGroups(_materialGroups, spec, mesh->_materialGroups);
      for (GroupConstIter i = beginMaterialGroups(); i != endMaterialGroups(); ++i)
      {
        mesh->_materials[i->name()] = _materials[i->name()];
      }

      return mesh;
    }

  std::ostream& operator<<(std::ostream& os, const VertexPTN& rhs);

  template <typename V> float3 position(const V& v) { return v.position; }
  template <typename V> float3 normal(const V& v) { return v.normal; }
  template <typename V> float2 uv(const V& v) { return v.uv; }

  template <typename V>
    shared_ptr<Mesh<V> > Mesh<V>::flatten()const
    {
      assert(_indices.empty());
      MeshPtr mesh(new Mesh<V>());

      mesh->_vertices.reserve(_vertices.size());
      vector<Group> mgs = _materialGroups;
      GroupIter lower = mgs.begin();

      // Merge all material groups by name.
      // TODO - Handle zero material groups
      while (lower != mgs.end())
      {
        Group mg(lower->name(), mesh->_vertices.size(), 0);
        GroupIter upper = partition(lower, mgs.end(), bind(&Group::name, _1) == mg.name());

        for (GroupIter iter = lower; iter != upper; ++iter)
        {
          mesh->_vertices.insert(mesh->_vertices.end(),
              &_vertices[iter->begin()],
              &_vertices[iter->end()]);
          mg.setCount(mg.count() + iter->count());
        }
        mesh->_materialGroups.push_back(mg);
        lower = upper;
      }
      mesh->_geometryGroups.push_back(Group("default", 0, _vertices.size()));
      mesh->_materials = _materials;
      return mesh;
    }

  template <typename V>
    std::ostream& operator<<(std::ostream& os, const Mesh<V>& rhs)
    {
      for_each(rhs._vertices.begin(), rhs._vertices.end(), 
          os << _1 << '\n');

      for_each(rhs._indices.begin(), rhs._indices.end(),
          os << _1 << '\n');

      os << "geometry-groups\n";
      for_each(rhs.beginGeometryGroups(), rhs.endGeometryGroups(), os << _1 << '\n');
      os << "material-groups\n";
      for_each(rhs.beginMaterialGroups(), rhs.endMaterialGroups(), os << _1 << '\n');

      os << "materials\n";
      for_each(rhs.materials().begin(), rhs.materials().end(), 
          os << bind(&MaterialMap::value_type::second, _1) << '\n');

      return os;
    }

  template <typename I>
    void adaptGroups(const std::vector<Group>& from, std::vector<Group>& to,
        boost::function<Group(Group const&, uint32_t)> adapter)
    {
      const uint32_t components = sizeof(I) / sizeof(uint32_t);
      to.reserve(from.size());
      transform(from.begin(), from.end(),
          back_inserter(to), bind(adapter, _1, components));
    }

  // Slow but simple indexing method, deprecated by the kdtree-indexer.
  template <typename V>
    void doScanIndexer(const shared_ptr<Mesh<V> >& flatMesh, shared_ptr<Mesh<V> >& IM) 
    {
      IM->_indices.reserve(flatMesh->_vertices.size());
      uint32_t nextIdx = 0;
      for (typename std::vector<V>::const_iterator v = flatMesh->_vertices.begin();
          v != flatMesh->_vertices.end(); ++v)
      {
        bool matchedVertex = false;
        for (std::vector<uint32_t>::const_iterator i = IM->_indices.begin();
            i != IM->_indices.end(); ++i)
        {
          if (v->equals(IM->_vertices[*i]))
          {
            IM->_indices.push_back(*i);
            matchedVertex = true;
            break;
          }
        }
        if (!matchedVertex)
        {
          IM->_vertices.push_back(*v);
          IM->_indices.push_back(nextIdx);
          ++nextIdx;
        }
      }
    }

  template <typename V>
    struct KdContext
    {
      V* _vertices;
      uint32_t _index;
      shared_ptr<Mesh<V> > _mesh;

      V& fromVertex() { return _vertices[_index]; }
      V& fromVertexAt(uint32_t idx) { return _vertices[idx]; }

      uint32_t addVertex()
      {
        uint32_t index = _mesh->vertices().size();
        _mesh->_indices.push_back(index);
        _mesh->_vertices.push_back(fromVertex());

        return index;
      }

      V& toVertex(uint32_t idx)
      {
        return _mesh->_vertices[idx];
      }

      void addIndexed(uint32_t idx) 
      { 
        _mesh->_indices.push_back(idx); 
      }
    };

  // Very basic KdTree structure allowing incremental vertex adds.
  // Todo - the KdContext business is ugly.
  // Could likely improve performance substantially by partitioning
  // the whole point-set at at once into a balanced tree stored in a 
  // linear memory block.  For now though, it's intended for offline
  // use only so it doesn't matter...
  template <typename V>
    class KdNode
    {
      public:
        KdNode(uint32_t splitAxis = 0):
          _splitAxis(splitAxis),
          _splitPosition(float()),
          _left(NULL),
          _right(NULL),
          _index(~0)
      {}
        ~KdNode()
        {
          delete _left;
          delete _right;
        }

        void addPoint(KdContext<V>& context)
        {
          if (_index == ~(uint32_t)0)
          {
            _index = context.addVertex();
            _splitPosition = context.fromVertex().position[_splitAxis];
            _left = new KdNode<V>(nextSplitAxis());
            _right = new KdNode<V>(nextSplitAxis());
            return;
          }

          if (context.toVertex(_index).equals(context.fromVertex()))
          {
            context.addIndexed(_index);
            return;
          }

          float d = _splitPosition - context.fromVertex().position[_splitAxis];

          if (d < 0.0f)
          {
            _left->addPoint(context);
          }
          else
          {
            _right->addPoint(context);
          }
        }
      private:
        uint32_t _splitAxis;
        float _splitPosition;
        KdNode<V>* _left;
        KdNode<V>* _right;
        uint32_t _index;

        // Just rotate the axes
        uint32_t nextSplitAxis()const { return (_splitAxis + 1) % 3; }
    };

  template <typename V>
    class KdTree
    {
      public:
        KdTree()
        {
        }
        void build(shared_ptr<Mesh<V> >& from, shared_ptr<Mesh<V> >& to)
        {
          KdContext<V> context;
          context._vertices = &from->_vertices[0];
          context._index = ~0;
          context._mesh = to;

          for (uint32_t i = 0; i < from->_vertices.size(); ++i)
          {
            context._index = i;
            _root.addPoint(context);
          }
        }

      private:
        KdNode<V> _root;
    };

  template <typename V>
    shared_ptr<Mesh<V> > meshWithMeta(shared_ptr<Mesh<V> > rhs)
    {
      shared_ptr<Mesh<V> > lhs(new Mesh<V>());
      lhs->_geometryGroups.assign(rhs->beginGeometryGroups(), rhs->endGeometryGroups());
      lhs->_materialGroups.assign(rhs->beginMaterialGroups(), rhs->endMaterialGroups());
      lhs->_materials = rhs->materials();
      return lhs;
    }

  template<typename V>
    shared_ptr<Mesh<V> > indexedMeshFromMesh(shared_ptr<Mesh<V> > flatMesh)
    {
      shared_ptr<Mesh<V> > IM = meshWithMeta(flatMesh);
      assert(flatMesh->_indices.empty());
      KdTree<V> tree;
      tree.build(flatMesh, IM); 
      return IM;
    }

  template<typename V>
    shared_ptr<Mesh<V> > meshFromIndexedMesh(shared_ptr<Mesh<V> > indexedMesh)
    {
      assert(!indexedMesh->_indices.empty());
      shared_ptr<Mesh<V> > flat = meshWithMeta(indexedMesh);
      std::transform(indexedMesh->indices().begin(),
          indexedMesh->indices().end(),
          back_inserter(flat->_vertices), 
          bind(&Mesh<V>::vertexAtIndex, indexedMesh.get(), _1));
      return flat;
    }
}

#endif

