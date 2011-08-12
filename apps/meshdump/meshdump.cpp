#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <lap/lap.h>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

using namespace lap;
using namespace std;
using namespace std::tr1;

  template <typename V>
void doMesh(shared_ptr<V> mesh)
{ 
  cout << "mesh: #verts " << mesh->vertices().size() << 
    ", numIndices " << mesh->indices().size() << "\n";
  cout << *mesh << endl;
/*
  GroupConstIter iter = find_if(mesh->beginGeometryGroups(), mesh->endGeometryGroups(),
    boost::lambda::bind(&Group::name, boost::cref(_1)) == 
    "a_8750_center_support a_8750_default_scene__from_scene_file0");
  if (iter != mesh->endGeometryGroups())
  {
    shared_ptr<V> diced = mesh->slice(*iter);
    cout << "diced:\n" << *diced << endl;
  }
*/
  obj::ModelPtr objOut = objFromMesh(mesh);
//  cout << "Obj\n" << *objOut << endl;
  
  obj::ObjTranslator ot;
  ot.exportFile(objOut, string("out.obj"));

  if (0)
  {
    for (std::vector<Group>::const_iterator iter = mesh->beginGeometryGroups(); 
        iter != mesh->endGeometryGroups(); ++iter)
    {
      shared_ptr<V> subMesh = mesh->slice(*iter);
//      cout << iter->name() << ":\n" << *subMesh << endl;
      if (1)
      {
        shared_ptr<V> idxMesh = indexedMeshFromMesh(subMesh);
        cout << "idxMesh: #verts " << idxMesh->vertices().size() << 
          ", #indices " << idxMesh->indices().size() << "\n";
        cout << *idxMesh << endl;
      }
    }
  }

  if (0)
  {
    shared_ptr<V> flat = mesh->flatten();
    cout << "flattened:\n" << *flat << endl;

    if (1)
    {
      shared_ptr<V> idxMesh = indexedMeshFromMesh(flat);
      cout << "idxMesh: #verts " << idxMesh->vertices().size() << 
        ", #indices " << idxMesh->indices().size() << "\n";
      cout << *idxMesh << endl;

      shared_ptr<V> flatAgain = meshFromIndexedMesh(idxMesh);
      cout << "flatAgain: #verts " << flatAgain->vertices().size() << 
        ", #indices " << flatAgain->indices().size() << "\n";
      cout << *flatAgain << endl;
    }
  }
}


int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << "Usage: meshdump <objfile>\n";
    return 1;
  }
  const string modelFile = argv[1];

  obj::ModelPtr model = obj::ObjTranslator().importFile(modelFile);
  if (!model)
  {
    cerr << "Error importing " << modelFile << endl;
    return 1;
  }
  cout << "vertexFormat: " << model->vertexFormat() << endl;
//  processModel(model, doMesh);
  switch (model->vertexFormat())
  {
    case obj::kPosition: doMesh(meshFromObj<VertexP>(model)); break;
    case obj::kPositionUV: doMesh(meshFromObj<VertexPT>(model)); break;
    case obj::kPositionNormal: doMesh(meshFromObj<VertexPN>(model)); break;
    case obj::kPositionUVNormal: doMesh(meshFromObj<VertexPTN>(model)); break;
    default: cerr << "Invalid vertex format" << endl; break;
  }
  return 0;
}
