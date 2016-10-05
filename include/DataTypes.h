/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#ifndef __DataTypes_h__
#define __DataTypes_h__

// Define integer types for Visual Studio 2008 and earlier
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#ifdef _WIN64
  // warning C4244: 'argument' : conversion from '__int64' to 'int', possible loss of data
  #pragma warning(push)
  #pragma warning(disable: 4244)
#endif

#if !_WIN32
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include <Eigen/Dense>

#if !_WIN32
  #pragma GCC diagnostic pop
#endif

//Standard Library
#include <vector>
#include <Eigen/StdVector>
#ifdef _WIN64
  #pragma warning(pop)
#endif

typedef float MATH_TYPE;

// matrices
typedef Eigen::Matrix<MATH_TYPE, 3, 3> Matrix3x3;

// vectors
typedef Eigen::Matrix<MATH_TYPE, 2, 1> Vector2;
typedef Eigen::Matrix<MATH_TYPE, 3, 1> Vector3;
typedef Eigen::Matrix<MATH_TYPE, 4, 1> Vector4;

#endif
