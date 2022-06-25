#include "common.h"

#include <sys/stat.h>

using namespace std;

bool isFile(const string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
        return S_ISREG(st.st_mode);
    return false;
}