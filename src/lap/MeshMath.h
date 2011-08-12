#ifndef MESH_MATH_H
#define MESH_MATH_H

#include <iostream>
#include <algorithm>
#include <cmath>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <cfloat>
#include <tr1/unordered_map>
#include <vector>

namespace lap 
{
  template <class T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
      std::tr1::hash<T> hasher;
      seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

  const float epsilon = 5E-4f;
  const float infinity = FLT_MAX;

  template<typename T>
    inline bool floatEquals(T a, T b)
    {
      return (fabs(a - b) < epsilon);
    }

  template<typename T, int N>
    class vec
    {
      public:
        vec()
        {
          std::fill(v, v+N, T());
        }

        bool operator==(const vec<T, N>& rhs)const
        {
          return std::equal(v, v+N, rhs.v);
        }

        bool operator<(const vec<T, N>& rhs)const
        {
          return std::lexicographical_compare(v, v+N,
              rhs.v, rhs.v + N);
        }

        vec<T,N>& operator=(const vec<T, N>& rhs)
        {
          std::copy(rhs.v, rhs.v + N, v);
          return *this;
        }

        T operator[](int i)const { return v[i]; }
        T& operator[](int i) { return v[i]; }

        bool equals(const vec<T, N>& rhs)const
        {
          return std::equal(v, v+N, rhs.v, floatEquals<T>);  
        }

      private:
        T v[N];
    };

  template <typename T, int N>
    vec<T, N> vecMin(const vec<T, N>& a, const vec<T, N>& b)
    {
      vec<T, N> c;
      for (int i = 0; i < N; ++i) c[i] = std::min(a[i], b[i]);
      return c;
    }

  template <typename T, int N>
    vec<T, N> vecMax(const vec<T, N>& a, const vec<T, N>& b)
    {
      vec<T, N> c;
      for (int i = 0; i < N; ++i) c[i] = std::max(a[i], b[i]);
      return c;
    }

  template <typename T, int N>
    std::ostream& operator<<(std::ostream& os, const vec<T, N>& rhs)
    {
      os << '[';
      for (int i = 0; i < N-1; ++i) os << rhs[i] << ' ';
      os << rhs[N-1];
      os << ']';
      return os;
    }

  template <typename V, int N>
    void writeVec(std::ostream& os, const vec<V,N>& rhs, 
        const char* prefix = NULL,
        const char* suffix = NULL)
    {
        if (prefix) os << prefix;
        for (int i = 0; i < N-1; ++i) os << rhs[i] << ' ';
        os << rhs[N-1];
        if (suffix) os << suffix;
    }

  template <typename T>
    class Range
    {
      public:
        Range(T* a, size_t count):
          _begin(a),
          _count(count)
      {}
        size_t count()const { return _count; }
        T* begin(){ return _begin; }
        const T* begin()const{ return _begin; }
        T* end(){ return _begin + _count; }
        const T* end()const{ return _begin + _count; }

        const T& operator[](int idx)const { return _begin[idx]; }
        T& operator[](int idx) { return _begin[idx]; }
      private:
        T* _begin;
        size_t _count;
    };

  template <typename T, typename U>
    Range<T> make_range(const U* p, size_t elements)
    {
      return Range<T>((T*)p, elements / (sizeof(T) / sizeof(U)));
    }

  template <typename I>
    Range<I> allocateRange(std::vector<uint32_t>& dest, size_t length)
    {
      const uint32_t components = sizeof(I) / sizeof(uint32_t);
      dest.resize(components * length);
      return make_range<I>(&dest[0], dest.size());
    }

  typedef vec<uint32_t,3> uint3;
  typedef vec<uint32_t,2> uint2;
  typedef vec<uint32_t,1> uint1;
  typedef vec<float,3> float3;
  typedef vec<float,2> float2;
  typedef vec<float,1> float1;

  template<int N>
    vec<float,N> parseVec(char* context)
    {
      vec<float, N> v;
      for (int i = 0; i < N; ++i) 
        v[i] = strtof(strtok_r(NULL, " ", &context), NULL);
      return v;
    }

  template <typename P>
    class BoundingBox
    {
      public:
        BoundingBox()
        {
          _min[0] = infinity; _min[1] = infinity; _min[2] = infinity; 
          _max[0] = -infinity; _max[1] = -infinity; _max[2] = -infinity; 
        }

        BoundingBox(const P *points, int numPoints)
        {
          _min[0] = infinity; _min[1] = infinity; _min[2] = infinity; 
          _max[0] = -infinity; _max[1] = -infinity; _max[2] = -infinity; 
          std::for_each(points, points + numPoints, 
              boost::lambda::bind(&BoundingBox::unionPoint, this, boost::lambda::_1));
        }

        const P & min()const { return _min; }
        const P & max()const { return _max; }
        float size(int i)const { return _max[i] - _min[i]; }
        P centre()const
        {
          P p;
          p[0] = _min[0] + size(0) / 2.0f;
          p[1] = _min[1] + size(1) / 2.0f;
          p[2] = _min[2] + size(2) / 2.0f;
          return p;
        }
        void unionPoint(const P &p)
        {
          _min = vecMin(_min, p);
          _max = vecMax(_max, p);
        }
        int largestAxis()const
        {
          return std::max(make_pair(size(0), 0), 
              std::max(make_pair(size(1), 1), make_pair(size(2), 2))).second;
        }
      private:
        P _min;
        P _max;
    };

  template <typename P>
    std::ostream & operator<<(std::ostream &os, const BoundingBox<P> &rhs)
    {
      os << '[' << rhs.min() << ", " << rhs.max() << ']';  
      return os;
    }

  class Group
  {
    public:
      Group(const std::string& name, uint32_t start, uint32_t count = 0):
        _start(start),
        _count(count),
        _name(name)
    {}
      uint32_t begin()const { return _start; }
      uint32_t end()const { return _start + _count; }
      uint32_t count()const { return  _count; }
      void setCount(uint32_t c) { _count = c; }
      const std::string& name()const { return _name; }

      bool operator<(const Group& b)const
      {
        return begin() < b.begin() || (begin() == b.begin() && b.end() < end());
      }

      bool contains(const Group& other)const
      {
        return other.begin() >= begin() && other.end() <= end();
      }

      bool intersects(const Group& other)const
      {
        return other.begin() < end() && other.end() > begin();
      }

    private:
      uint32_t _start;
      uint32_t _count;
      std::string _name;
  };

  inline Group intersection(const Group& a, const Group& b)
  {
    uint32_t s = std::max(a.begin(), b.begin());
    uint32_t e = std::min(a.end(), b.end());
    return Group(a.name(), s, e - s);
  }

  std::ostream& operator<<(std::ostream& os, const Group& rhs);
}

  namespace std { 
    namespace tr1 {
      template <typename T, int N>
        struct hash<lap::vec<T,N> > : public unary_function<lap::vec<T,N>, size_t>
        {
          size_t operator()(const lap::vec<T, N>& v) const
          {
            std::size_t seed = 0;
            for (int i = 0; i < N; ++i) lap::hash_combine(seed, v[i]);
            return seed;

          }
        };
    }}


#endif
