#include <string>
using std::string;

#ifdef _WIN32
#include <Windows.h>
#endif

namespace CGL {

string resolve_path( const char* filename ) {
#ifdef _WIN32
  char resolved[512];
  GetFullPathName(filename, 512, resolved, NULL);
  return string(resolved);
#else
  return string(realpath(filename, NULL));
#endif
}

}
