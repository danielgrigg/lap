#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <lap/lap.h>

using namespace lap;
using namespace std;

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    cerr << "Usage: objdump <objfile> [outobjfile]\n";
    return 1;
  }
  const string modelFile = argv[1];
  const string outModelFile = argv[2];

  obj::ObjTranslator ot;
  obj::ModelPtr model = ot.importFile(modelFile);
  if (!model)
  {
    cerr << "Error importing '" << modelFile << "'\n";
    return 1;
  }

//  cout << model << endl;

  cout << "HasVertices: " << !model->positions().empty() << endl;
  cout << "HasUVS: " << !model->uvs().empty() << endl;
  cout << "HasNormals: " << !model->normals().empty() << endl;
  cout << model->positions().size() << " vertices\n";
  cout << model->numTriangles() << " triangles.\n";

  ot.exportFile(model, outModelFile);
  return 0;
}
