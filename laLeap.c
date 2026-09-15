#include "laLeap.h"

/*
gcc -c laLeap.c -Wall -Wextra -I./raylib-5.5/include -o test_bin/laLeap.o -lm
gcc -c laLeap.c -O3 -march=native -flto -I./raylib-5.5/include -o test_bin/laLeap.o -lm
*/

float vec3Dist(LEAP_VECTOR v1, LEAP_VECTOR v2)
{
   return sqrtf(powf(v1.x - v2.x, 2) + powf(v1.y - v2.y, 2) + powf(v1.z - v2.z, 2));
}

LEAP_VECTOR vec3Add(LEAP_VECTOR v1, LEAP_VECTOR v2)
{
   return (LEAP_VECTOR) {
      .x = v1.x + v2.x,
      .y = v1.y + v2.y,
      .z = v1.z + v2.z,
   };
}

LEAP_VECTOR vec3Sub(LEAP_VECTOR v1, LEAP_VECTOR v2)
{
   return (LEAP_VECTOR) {
      .x = v1.x - v2.x,
      .y = v1.y - v2.y,
      .z = v1.z - v2.z,
   };
}

LEAP_VECTOR vec3ScalDiv(LEAP_VECTOR v1, float val)
{
   return (LEAP_VECTOR) {
      .x = v1.x / val,
      .y = v1.y / val,
      .z = v1.z / val,
   };
}

LEAP_VECTOR vec3ScalMul(LEAP_VECTOR v1, float val)
{
   return (LEAP_VECTOR) {
      .x = v1.x * val,
      .y = v1.y * val,
      .z = v1.z * val,
   };
}

LEAP_VECTOR vec3Norm(LEAP_VECTOR v)
{
   float length = sqrtf(powf(v.x, 2) + powf(v.y, 2) + powf(v.z, 2));
   return (LEAP_VECTOR) {
      .x = v.x / length, 
      .y = v.y / length, 
      .z = v.z / length, 
   };
}

void vec3NormIP(LEAP_VECTOR *v)
{
   float length = sqrtf(powf(v->x, 2) + powf(v->y, 2) + powf(v->z, 2));
   v->x = v->x / length;
   v->y = v->y / length;
   v->z = v->z / length;
}

// ----------------

float Vec3Dist(Vector3 v1, Vector3 v2)
{
   return sqrtf(powf(v1.x - v2.x, 2) + powf(v1.y - v2.y, 2) + powf(v1.z - v2.z, 2));
}

Vector3 Vec3Add(Vector3 v1, Vector3 v2)
{
   return (Vector3) {
      .x = v1.x + v2.x,
      .y = v1.y + v2.y,
      .z = v1.z + v2.z,
   };
}

Vector3 Vec3Sub(Vector3 v1, Vector3 v2)
{
   return (Vector3) {
      .x = v1.x - v2.x,
      .y = v1.y - v2.y,
      .z = v1.z - v2.z,
   };
}

Vector3 Vec3ScalDiv(Vector3 v1, float val)
{
   return (Vector3) {
      .x = v1.x / val,
      .y = v1.y / val,
      .z = v1.z / val,
   };
}

Vector3 Vec3ScalMul(Vector3 v1, float val)
{
   return (Vector3) {
      .x = v1.x * val,
      .y = v1.y * val,
      .z = v1.z * val,
   };
}

Vector3 Vec3Norm(Vector3 v)
{
   float length = sqrtf(powf(v.x, 2) + powf(v.y, 2) + powf(v.z, 2));
   return (Vector3) {
      .x = v.x / length, 
      .y = v.y / length, 
      .z = v.z / length, 
   };
}

void Vec3NormIP(Vector3 *v)
{
   float length = sqrtf(powf(v->x, 2) + powf(v->y, 2) + powf(v->z, 2));
   v->x = v->x / length;
   v->y = v->y / length;
   v->z = v->z / length;
}
