#include <cstring>
#include <iostream>

using namespace std;

int char2int(const char *c);

float char2float(const char *c);

void char2vertex(char *c, float &x, float &y, float &z);

void char2face(char *face, int &a, int &b, int &c);

void char2UnitData(char *c, int &index, float &x, float &y, float&z);

void char2TexCoord(char *c, float &x, float &y);