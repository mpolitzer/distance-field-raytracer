#ifndef DF_H
#define DF_H
#include <stdint.h>
#include "vec3.h"

// This is a distance-field block in CPU memory.
typedef struct df_t df;

// This is the implicit surface generator function.
typedef float (*is_gen)(float x, float y, float z, void *arg);

struct df_t {

	float *p; // dense block of distance function and its dimensions
	ivec3  n; // p dimensions

	vec3 center, half; // AABB
};

void df_build(df *o,
              ivec3 n, float *p,      // canvas / output buffer
              vec3 center, vec3 half, // AABB
              is_gen fn, void *arg);  // implicit surface generator

float df_value_at     (const df *i, float x, float y, float z);

void df_build_sphere  (df *o, float x, float y, float z, float r);
void df_build_box     (df *o, float x, float y, float z, float w, float h, float d);
void df_unite         (df *o, const df *a, const df *b);
void df_unite_rounded (df *o, const df *a, const df *b);
void df_unite_chamfer (df *o, const df *a, const df *b);
void df_intersect     (df *o, const df *a, const df *b);
void df_subtract      (df *o, const df *a, const df *b);

#endif /* DF_H */
