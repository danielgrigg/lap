#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <lap/lap.h>
#include <boost/function.hpp>

using namespace lap;
using namespace std;
using namespace std::tr1;

  template <typename V>
void extractGroups(shared_ptr<V> mesh)
{ 

  for (GroupConstIter iter = mesh->beginGeometryGroups(); 
      iter != mesh->endGeometryGroups(); ++iter)
  {
    shared_ptr<V> sliced = mesh->slice(*iter)->flatten();
    cout << iter->name() << " Sliced.. ";

    shared_ptr<V> welded = meshFromIndexedMesh(indexedMeshFromMesh(sliced));
    cout << "welded.. ";
    const std::string outName = iter->name() + ".obj";
    obj::ObjTranslator().exportFile(objFromMesh(welded), outName);
    cout << "written to " << outName << endl; 
  }
}

int main(int argc, char **argv)
{
  // dude where's my options
  if (argc < 2)
  {
    cerr << "Usage: lapinfo <command>\n  xg : extract all geometry-groups\n";
    return 1;
  }
  const string modelFile = argv[1];

  obj::ModelPtr model = obj::ObjTranslator().importFile(modelFile);
  if (!model)
  {
    cerr << "Error importing " << modelFile << endl;
    return 1;
  }
  cout << "ModelFile: " << modelFile << endl;
  cout << "vertexFormat: " << model->vertexFormat() << endl;

  switch (model->vertexFormat())
  {
    case obj::kPosition: extractGroups(meshFromObj<VertexP>(model)); break;
    case obj::kPositionUV: extractGroups(meshFromObj<VertexPT>(model)); break;
    case obj::kPositionNormal: extractGroups(meshFromObj<VertexPN>(model)); break;
    case obj::kPositionUVNormal: extractGroups(meshFromObj<VertexPTN>(model)); break;
    default: cerr << "Invalid vertex format" << endl; break;
  }
  return 0;
}
