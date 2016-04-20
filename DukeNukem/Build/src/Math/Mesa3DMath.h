/*
* Mesa 3-D graphics library
*
* Copyright (C) 1999-2005  Brian Paul   All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

// jmarshall: I want to deprecate this as soon as possible, but for testing I want to construct
// the view and projection matrixs as exact as possible to the original so I don't introduce any new bugs.
// Turth is I'm just being lazy.

#include <math.h>
#include <memory.h>
#pragma once

namespace Math
{
	#define M_PI 3.14159265358979323846
	#define A(row,col)  a[(col<<2)+row]
	#define B(row,col)  b[(col<<2)+row]
	#define P(row,col)  product[(col<<2)+row]

	/**
	* Identity matrix.
	*/
	static const float Identity[16] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	static void glhFrustumf2(float *matrix, float left, float right, float bottom, float top, float znear, float zfar)
	{
		float temp, temp2, temp3, temp4;
		temp = 2.0 * znear;
		temp2 = right - left;
		temp3 = top - bottom;
		temp4 = zfar - znear;
		matrix[0] = temp / temp2;
		matrix[1] = 0.0;
		matrix[2] = 0.0;
		matrix[3] = 0.0;
		matrix[4] = 0.0;
		matrix[5] = temp / temp3;
		matrix[6] = 0.0;
		matrix[7] = 0.0;
		matrix[8] = (right + left) / temp2;
		matrix[9] = (top + bottom) / temp3;
		matrix[10] = (-zfar - znear) / temp4;
		matrix[11] = -1.0;
		matrix[12] = 0.0;
		matrix[13] = 0.0;
		matrix[14] = (-temp * zfar) / temp4;
		matrix[15] = 0.0;
	}

	static void glhPerspectivef2(float *matrix, float fovyInDegrees, float aspectRatio, float znear, float zfar)
	{
		float ymax, xmax;
		float temp, temp2, temp3, temp4;
		ymax = znear * tanf(fovyInDegrees * M_PI / 360.0);
		//ymin = -ymax;
		//xmin = -ymax * aspectRatio;
		xmax = ymax * aspectRatio;
		glhFrustumf2(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
	}
	

	static void matmul4(float *product, const float *a, const float *b)
	{
		int i;
		for (i = 0; i < 4; i++) {
			const float ai0 = A(i, 0), ai1 = A(i, 1), ai2 = A(i, 2), ai3 = A(i, 3);
			P(i, 0) = ai0 * B(0, 0) + ai1 * B(1, 0) + ai2 * B(2, 0) + ai3 * B(3, 0);
			P(i, 1) = ai0 * B(0, 1) + ai1 * B(1, 1) + ai2 * B(2, 1) + ai3 * B(3, 1);
			P(i, 2) = ai0 * B(0, 2) + ai1 * B(1, 2) + ai2 * B(2, 2) + ai3 * B(3, 2);
			P(i, 3) = ai0 * B(0, 3) + ai1 * B(1, 3) + ai2 * B(2, 3) + ai3 * B(3, 3);
		}
	}

	static void _math_matrix_rotate(XMMATRIX &matrix, float angle, float x, float y, float z)
	{
		float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
		float m[16];
		bool optimized;

		s = sinf(angle * M_PI / 180.0);
		c = cosf(angle * M_PI / 180.0);

		memcpy(m, Identity, sizeof(Identity));
		optimized = false;

#define M(row,col)  m[col*4+row]

		if (x == 0.0F) {
			if (y == 0.0F) {
				if (z != 0.0F) {
					optimized = true;
					/* rotate only around z-axis */
					M(0, 0) = c;
					M(1, 1) = c;
					if (z < 0.0F) {
						M(0, 1) = s;
						M(1, 0) = -s;
					}
					else {
						M(0, 1) = -s;
						M(1, 0) = s;
					}
				}
			}
			else if (z == 0.0F) {
				optimized = true;
				/* rotate only around y-axis */
				M(0, 0) = c;
				M(2, 2) = c;
				if (y < 0.0F) {
					M(0, 2) = -s;
					M(2, 0) = s;
				}
				else {
					M(0, 2) = s;
					M(2, 0) = -s;
				}
			}
		}
		else if (y == 0.0F) {
			if (z == 0.0F) {
				optimized = true;
				/* rotate only around x-axis */
				M(1, 1) = c;
				M(2, 2) = c;
				if (x < 0.0F) {
					M(1, 2) = s;
					M(2, 1) = -s;
				}
				else {
					M(1, 2) = -s;
					M(2, 1) = s;
				}
			}
		}

		if (!optimized) {
			const float mag = sqrtf(x * x + y * y + z * z);

			if (mag <= 1.0e-4F) {
				/* no rotation, leave mat as-is */
				return;
			}

			x /= mag;
			y /= mag;
			z /= mag;


			/*
			*     Arbitrary axis rotation matrix.
			*
			*  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
			*  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
			*  (which is about the X-axis), and the two composite transforms
			*  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
			*  from the arbitrary axis to the X-axis then back.  They are
			*  all elementary rotations.
			*
			*  Rz' is a rotation about the Z-axis, to bring the axis vector
			*  into the x-z plane.  Then Ry' is applied, rotating about the
			*  Y-axis to bring the axis vector parallel with the X-axis.  The
			*  rotation about the X-axis is then performed.  Ry and Rz are
			*  simply the respective inverse transforms to bring the arbitrary
			*  axis back to its original orientation.  The first transforms
			*  Rz' and Ry' are considered inverses, since the data from the
			*  arbitrary axis gives you info on how to get to it, not how
			*  to get away from it, and an inverse must be applied.
			*
			*  The basic calculation used is to recognize that the arbitrary
			*  axis vector (x, y, z), since it is of unit length, actually
			*  represents the sines and cosines of the angles to rotate the
			*  X-axis to the same orientation, with theta being the angle about
			*  Z and phi the angle about Y (in the order described above)
			*  as follows:
			*
			*  cos ( theta ) = x / sqrt ( 1 - z^2 )
			*  sin ( theta ) = y / sqrt ( 1 - z^2 )
			*
			*  cos ( phi ) = sqrt ( 1 - z^2 )
			*  sin ( phi ) = z
			*
			*  Note that cos ( phi ) can further be inserted to the above
			*  formulas:
			*
			*  cos ( theta ) = x / cos ( phi )
			*  sin ( theta ) = y / sin ( phi )
			*
			*  ...etc.  Because of those relations and the standard trigonometric
			*  relations, it is pssible to reduce the transforms down to what
			*  is used below.  It may be that any primary axis chosen will give the
			*  same results (modulo a sign convention) using thie method.
			*
			*  Particularly nice is to notice that all divisions that might
			*  have caused trouble when parallel to certain planes or
			*  axis go away with care paid to reducing the expressions.
			*  After checking, it does perform correctly under all cases, since
			*  in all the cases of division where the denominator would have
			*  been zero, the numerator would have been zero as well, giving
			*  the expected result.
			*/

			xx = x * x;
			yy = y * y;
			zz = z * z;
			xy = x * y;
			yz = y * z;
			zx = z * x;
			xs = x * s;
			ys = y * s;
			zs = z * s;
			one_c = 1.0F - c;

			/* We already hold the identity-matrix so we can skip some statements */
			M(0, 0) = (one_c * xx) + c;
			M(0, 1) = (one_c * xy) - zs;
			M(0, 2) = (one_c * zx) + ys;
			/*    M(0,3) = 0.0F; */

			M(1, 0) = (one_c * xy) + zs;
			M(1, 1) = (one_c * yy) + c;
			M(1, 2) = (one_c * yz) - xs;
			/*    M(1,3) = 0.0F; */

			M(2, 0) = (one_c * zx) - ys;
			M(2, 1) = (one_c * yz) + xs;
			M(2, 2) = (one_c * zz) + c;
			/*    M(2,3) = 0.0F; */

			/*
			M(3,0) = 0.0F;
			M(3,1) = 0.0F;
			M(3,2) = 0.0F;
			M(3,3) = 1.0F;
			*/
		}
#undef M
		XMMATRIX _internalMatrix(&m[0]);
		//_internalMatrix.r[0] = XMVectorSet(m[0], m[1], m[2], m[3]);
		//_internalMatrix.r[1] = XMVectorSet(m[4], m[5], m[6], m[7]);
		//_internalMatrix.r[2] = XMVectorSet(m[8], m[9], m[10], m[11]);
		//_internalMatrix.r[3] = XMVectorSet(m[12], m[13], m[14], m[15]);
		matrix = XMMatrixMultiply(_internalMatrix, matrix);

		//matmul4(mat, mat, m);
		//matrix_multf(mat, m, MAT_FLAG_ROTATION);
	}
}