#pragma once
#ifndef laLeapC_h
#define laLeapC_h

#include <LeapC.h>
#include <math.h>

#include <raylib.h>

float vec3Dist(LEAP_VECTOR v1, LEAP_VECTOR v2);

LEAP_VECTOR vec3Add(LEAP_VECTOR v1, LEAP_VECTOR v2);
LEAP_VECTOR vec3Sub(LEAP_VECTOR v1, LEAP_VECTOR v2);

LEAP_VECTOR vec3ScalMul(LEAP_VECTOR v1, float val);
LEAP_VECTOR vec3ScalDiv(LEAP_VECTOR v1, float val);
LEAP_VECTOR vec3Norm(LEAP_VECTOR v);
void        vec3NormIP(LEAP_VECTOR *v);


float Vec3Dist(Vector3 v1, Vector3 v2);

Vector3 Vec3Add(Vector3 v1, Vector3 v2);
Vector3 Vec3Sub(Vector3 v1, Vector3 v2);

Vector3 Vec3ScalMul(Vector3 v1, float val);
Vector3 Vec3ScalDiv(Vector3 v1, float val);
Vector3 Vec3Norm(Vector3 v);
void    Vec3NormIP(Vector3 *v);

#endif // laLeapC_h
