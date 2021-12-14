#include <math3D.h>

vertex& vertex::operator+=(const vertex& b) { x += b.x;    y += b.y;   z+= b.z;     return *this; }
vertex& vertex::operator+=(const float& b)  { x += b;      y += b;     z+= b;       return *this; }
vertex& vertex::operator-=(const vertex& b) { x -= b.x;    y -= b.y;   z-= b.z;     return *this; }
vertex& vertex::operator-=(const float& b)  { x -= b;      y -= b;     z-= b;       return *this; }
vertex& vertex::operator*=(const float& b)  { x *= b;      y *= b;     z*= b;       return *this; }
vertex& vertex::operator*=(const vertex& b) { x *= b.x;      y *= b.y;     z*= b.z; return *this; }
vertex  vertex::operator- ()                { return vertex {-x,-y,-z}; }

vertex operator+ (vertex a, vertex const& b){return a += b;}
vertex operator+ (vertex a, const float b)  {return a += b;}
vertex operator- (vertex a, vertex const& b){return a -= b;}
vertex operator- (vertex a, const float b)  {return a -= b;}
vertex operator* (vertex a, const float b)  {return a *= b;}
vertex operator* (vertex a, const vertex b) {return a *= b;}
