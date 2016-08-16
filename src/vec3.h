#ifndef VEC3_H
#define VEC3_H
#include <stdbool.h>

struct vec3_t { float x, y, z; };
typedef struct vec3_t vec3;

vec3  vec3_make  (float x, float y, float z);
vec3  vec3_add   (vec3 u, vec3 v);
vec3  vec3_sub   (vec3 u, vec3 v);
vec3  vec3_mul   (vec3 u, vec3 v);
vec3  vec3_div   (vec3 u, vec3 v);
vec3  vec3_divs  (vec3 u, float s);

bool  vec3_gt    (vec3 u, vec3 v);
vec3  vec3_max   (vec3 u, vec3 v);
vec3  vec3_min   (vec3 u, vec3 v);
float vec3_hmax  (vec3 u);
float vec3_length(vec3 u);
vec3  vec3_abs   (vec3 u);

/* -------------------------------------------------------------------------- */

struct ivec3_t { int x, y, z; };
typedef struct ivec3_t ivec3;

ivec3 ivec3_make(int x, int y, int z);

#endif /* VEC3_H */
