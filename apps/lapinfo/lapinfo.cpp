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
void getInfo(shared_ptr<V> mesh)
{ 
  cout << "vertices " << mesh->vertices().size() << endl;
  {
    shared_ptr<V> idxMesh = indexedMeshFromMesh(mesh->flatten());
    cout << "indexed-vertices " << idxMesh->vertices().size() << endl;
    cout << "triangles " << idxMesh->indices().size() << endl;
  }

  cout << "groups\n";

  for (GroupConstIter iter = mesh->beginGeometryGroups(); 
      iter != mesh->endGeometryGroups(); ++iter)
  {
    shared_ptr<V> subMesh = mesh->slice(*iter)->flatten();
    cout << "  " << iter->name() << endl;
    cout << "    vertices " << subMesh->vertices().size() << endl;

    shared_ptr<V> indexedSubMesh = indexedMeshFromMesh(subMesh);
    cout << "    indexed-vertices " << indexedSubMesh->vertices().size() << endl;
    cout << "    triangles " << indexedSubMesh->indices().size() << endl;
    cout << "    materials ";

    for_each(subMesh->beginMaterialGroups(), subMesh->endMaterialGroups(), 
        cout << bind(&Group::name, _1) << ' ');
    cout << endl;
  }
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << "Usage: lapinfo <obj-file>\n";
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
    case obj::kPosition: getInfo(meshPFromObj(model)); break;
    case obj::kPositionUV: getInfo(meshPTFromObj(model)); break;
    case obj::kPositionNormal: getInfo(meshPNFromObj(model)); break;
    case obj::kPositionUVNormal: getInfo(meshPTNFromObj(model)); break;
    default: cerr << "Invalid vertex format" << endl; break;
  }
  return 0;
}
