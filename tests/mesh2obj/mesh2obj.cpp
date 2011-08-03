#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <lap/ObjModel.h>
#include <lap/MeshAsset.h>

using namespace lap;
using namespace std;
using namespace std::tr1;

  template <typename V>
void doMesh(shared_ptr<V> mesh, const std::string& outFile)
{ 
//  cout << "mesh: #verts " << mesh->vertices().size() << "\n" << *mesh << endl;
  shared_ptr<V> flat =
    meshFromIndexedMesh(
      indexedMeshFromMesh(
        mesh->flatten()));

  obj::ModelPtr model = objFromMesh(flat);
//  cout << "Model:\n" << *model << endl;
//
  obj::ObjTranslator ot;
  ot.exportFile(model, outFile);
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    cerr << "Usage: meshdump <objfile> <outfile>\n";
    return 1;
  }
  const string modelFile = argv[1];
  const string outFile = argv[2];

  obj::ModelPtr model = obj::ObjTranslator().importFile(modelFile);
  if (!model)
  {
    cerr << "Error importing " << modelFile << endl;
    return 1;
  }
  cout << "vertexFormat: " << model->vertexFormat() << endl;
  switch (model->vertexFormat())
  {
    case obj::kPosition: doMesh(meshPFromObj(model), outFile); break;
    case obj::kPositionUV: doMesh(meshPTFromObj(model), outFile); break;
    case obj::kPositionNormal: doMesh(meshPNFromObj(model), outFile); break;
    case obj::kPositionUVNormal: doMesh(meshPTNFromObj(model), outFile); break;
    default: cerr << "Invalid vertex format" << endl; break;
  }
  return 0;
}
