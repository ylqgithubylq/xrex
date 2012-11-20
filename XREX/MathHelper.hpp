/*
*	Code partially come from KlayGE, with some simplification and improvement.
*/
#pragma once

namespace XREX
{
	namespace MathHelper
	{
		template <typename T, uint32 N> // is a class due to template function cannot be partially specialized.
		struct TransformHelper
		{
			static void DoTransform(T out[N], T const m[16], T const v[N], T lastComponent);
			static void DoTransformDirection(T out[N], T const m[16], T const v[N]);
		};

		template <typename T>
		struct TransformHelper<T, 4>
		{
			static void DoTransform(T out[4], T const m[16], T const v[4], T lastComponent)
			{
				out[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * v[3];
				out[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * v[3];
				out[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
				out[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
			}
			static void DoTransformDirection(T out[4], T const m[16], T const v[4])
			{
				out[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2];
				out[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2];
				out[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2];
				out[3] = T(0);
			}
		};
		template <typename T>
		struct TransformHelper<T, 3>
		{
			static void DoTransform(T out[3], T const m[16], T const v[3], T lastComponent)
			{
				T inverseW = T(1) / (m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * lastComponent);
				out[0] = (m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * lastComponent) * inverseW;
				out[1] = (m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * lastComponent) * inverseW;
				out[2] = (m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * lastComponent) * inverseW;
			}
			static void DoTransformDirection(T out[3], T const m[16], T const v[3])
			{
				out[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2];
				out[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2];
				out[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2];
			}
		};


		template <typename T, uint32 N>
		struct VectorHelper
		{
			static_assert(N <= 4, "Vector larger than 4 are not support.");
			template <typename U>
			static void DoCopy(T out[N], U const rhs[N])
			{
				out[0] = static_cast<T>(rhs[0]);
				VectorHelper<T, N - 1>::DoCopy(out + 1, rhs + 1);
			}

			static void DoAssign(T out[N], T const& rhs)
			{
				out[0] = rhs;
				VectorHelper<T, N - 1>::DoAssign(out + 1, rhs);
			}

			static void DoAdd(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = lhs[0] + rhs[0];
				VectorHelper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs + 1);
			}

			static void DoAdd(T out[N], T const lhs[N], T const& rhs)
			{
				out[0] = lhs[0] + rhs;
				VectorHelper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs);
			}

			static void DoSubtract(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = lhs[0] - rhs[0];
				VectorHelper<T, N - 1>::DoSubtract(out + 1, lhs + 1, rhs + 1);
			}

			static void DoSubtract(T out[N], T const lhs[N], T const& rhs)
			{
				out[0] = lhs[0] - rhs;
				VectorHelper<T, N - 1>::DoSubtract(out + 1, lhs + 1, rhs);
			}

			static void DoMultiply(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = lhs[0] * rhs[0];
				VectorHelper<T, N - 1>::DoMultiply(out + 1, lhs + 1, rhs + 1);
			}

			static void DoScale(T out[N], T const lhs[N], T const& rhs)
			{
				out[0] = lhs[0] * rhs;
				VectorHelper<T, N - 1>::DoScale(out + 1, lhs + 1, rhs);
			}

			static void DoDivide(T out[N], T const lhs[N], T const rhs[N])
			{
				out[0] = lhs[0] / rhs[0];
				VectorHelper<T, N - 1>::DoDivide(out + 1, lhs + 1, rhs + 1);
			}

			static void DoNegate(T out[N], T const rhs[N])
			{
				out[0] = -rhs[0];
				VectorHelper<T, N - 1>::DoNegate(out + 1, rhs + 1);
			}

			static bool DoEqual(T const lhs[N], T const rhs[N])
			{
				return VectorHelper<T, 1>::DoEqual(lhs, rhs) && VectorHelper<T, N - 1>::DoEqual(lhs + 1, rhs + 1);
			}

			static void DoSwap(T lhs[N], T rhs[N])
			{
				std::swap(lhs[0], rhs[0]);
				VectorHelper<T, N - 1>::DoSwap(lhs + 1, rhs + 1);
			}


			static T DoDot(T const lhs[1], T const rhs[1])
			{
				return lhs[0] * rhs[0] + VectorHelper<T, N - 1>::DoDot(lhs + 1, rhs + 1);
			}
		};

		template <typename T>
		struct VectorHelper<T, 1>
		{
			template <typename U>
			static void DoCopy(T out[1], U const rhs[1])
			{
				out[0] = static_cast<T>(rhs[0]);
			}

			static void DoAssign(T out[1], T const& rhs)
			{
				out[0] = rhs;
			}

			static void DoAdd(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = lhs[0] + rhs[0];
			}

			static void DoAdd(T out[1], T const lhs[1], T const rhs)
			{
				out[0] = lhs[0] + rhs;
			}

			static void DoSubtract(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = lhs[0] - rhs[0];
			}

			static void DoSubtract(T out[1], T const lhs[1], T const& rhs)
			{
				out[0] = lhs[0] - rhs;
			}

			static void DoMultiply(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = lhs[0] * rhs[0];
			}

			static void DoScale(T out[1], T const lhs[1], T const& rhs)
			{
				out[0] = lhs[0] * rhs;
			}

			static void DoDivide(T out[1], T const lhs[1], T const rhs[1])
			{
				out[0] = lhs[0] / rhs[0];
			}

			static void DoNegate(T out[1], T const rhs[1])
			{
				out[0] = -rhs[0];
			}

			static bool DoEqual(T const lhs[1], T const rhs[1])
			{
				return lhs[0] == rhs[0];
			}

			static T DoDot(T const lhs[1], T const rhs[1])
			{
				return lhs[0] * rhs[0];
			}
		};

		template <typename T>
		struct MatrixHepler
		{
			static void DoMultiply(T out[16], T const lhs[16], T const rhs[16])
			{
				out[0] = rhs[0] * lhs[0] + rhs[1] * lhs[4] + rhs[2] * lhs[8] + rhs[3] * lhs[12];
				out[1] = rhs[0] * lhs[1] + rhs[1] * lhs[5] + rhs[2] * lhs[9] + rhs[3] * lhs[13];
				out[2] = rhs[0] * lhs[2] + rhs[1] * lhs[6] + rhs[2] * lhs[10] + rhs[3] * lhs[14];
				out[3] = rhs[0] * lhs[3] + rhs[1] * lhs[7] + rhs[2] * lhs[11] + rhs[3] * lhs[15];
				out[4] = rhs[4] * lhs[0] + rhs[5] * lhs[4] + rhs[6] * lhs[8] + rhs[7] * lhs[12];
				out[5] = rhs[4] * lhs[1] + rhs[5] * lhs[5] + rhs[6] * lhs[9] + rhs[7] * lhs[13];
				out[6] = rhs[4] * lhs[2] + rhs[5] * lhs[6] + rhs[6] * lhs[10] + rhs[7] * lhs[14];
				out[7] = rhs[4] * lhs[3] + rhs[5] * lhs[7] + rhs[6] * lhs[11] + rhs[7] * lhs[15];
				out[8] = rhs[8] * lhs[0] + rhs[9] * lhs[4] + rhs[10] * lhs[8] + rhs[11] * lhs[12];
				out[9] = rhs[8] * lhs[1] + rhs[9] * lhs[5] + rhs[10] * lhs[9] + rhs[11] * lhs[13];
				out[10] = rhs[8] * lhs[2] + rhs[9] * lhs[6] + rhs[10] * lhs[10] + rhs[11] * lhs[14];
				out[11] = rhs[8] * lhs[3] + rhs[9] * lhs[7] + rhs[10] * lhs[11] + rhs[11] * lhs[15];
				out[12] = rhs[12] * lhs[0] + rhs[13] * lhs[4] + rhs[14] * lhs[8] + rhs[15] * lhs[12];
				out[13] = rhs[12] * lhs[1] + rhs[13] * lhs[5] + rhs[14] * lhs[9] + rhs[15] * lhs[13];
				out[14] = rhs[12] * lhs[2] + rhs[13] * lhs[6] + rhs[14] * lhs[10] + rhs[15] * lhs[14];
				out[15] = rhs[12] * lhs[3] + rhs[13] * lhs[7] + rhs[14] * lhs[11] + rhs[15] * lhs[15];
			}

			static void CalculateDeterminant(T vectors_[16]) 
			{
				// subscript: row, column
				T m11 = in[0], m21 = in[1], m31 = in[2], m41 = in[3],
					m12 = in[4], m22 = in[5], m32 = in[6], m42 = in[7],
					m13 = in[8], m23 = in[9], m33 = in[10], m43 = in[11],
					m14 = in[12], m24 = in[13], m34 = in[14], m44 = in[15];

				T _1122_2112 = m11 * m22 - m21 * m12,
					_1132_3112 = m11 * m32 - m31 * m12,
					_1142_4112 = m11 * m42 - m41 * m12,
					_2132_3122 = m21 * m32 - m31 * m22,
					_2142_4122 = m21 * m42 - m41 * m22,
					_3142_4132 = m31 * m42 - m41 * m32,
					_1324_2314 = m13 * m24 - m23 * m14,
					_1334_3314 = m13 * m34 - m33 * m14,
					_1344_4314 = m13 * m44 - m43 * m14,
					_2334_3324 = m23 * m34 - m33 * m24,
					_2344_4324 = m23 * m44 - m43 * m24,
					_3344_4334 = m33 * m44 - m43 * m34;

				T determinant = (_1122_2112 * _3344_4334 - _1132_3112 * _2344_4324 + _1142_4112 * _2334_3324 + _2132_3122 * _1344_4314 - _2142_4122 * _1334_3314 + _3142_4132 * _1324_2314);
				return determinant;
			}

			bool static DoInverse(T out[16], T const in[16])
			{
				// subscript: row, column
				T m11 = in[0], m21 = in[1], m31 = in[2], m41 = in[3],
					m12 = in[4], m22 = in[5], m32 = in[6], m42 = in[7],
					m13 = in[8], m23 = in[9], m33 = in[10], m43 = in[11],
					m14 = in[12], m24 = in[13], m34 = in[14], m44 = in[15];

				T _1122_2112 = m11 * m22 - m21 * m12,
					_1132_3112 = m11 * m32 - m31 * m12,
					_1142_4112 = m11 * m42 - m41 * m12,
					_2132_3122 = m21 * m32 - m31 * m22,
					_2142_4122 = m21 * m42 - m41 * m22,
					_3142_4132 = m31 * m42 - m41 * m32,
					_1324_2314 = m13 * m24 - m23 * m14,
					_1334_3314 = m13 * m34 - m33 * m14,
					_1344_4314 = m13 * m44 - m43 * m14,
					_2334_3324 = m23 * m34 - m33 * m24,
					_2344_4324 = m23 * m44 - m43 * m24,
					_3344_4334 = m33 * m44 - m43 * m34;

				T determinant = (_1122_2112 * _3344_4334 - _1132_3112 * _2344_4324 + _1142_4112 * _2334_3324 + _2132_3122 * _1344_4314 - _2142_4122 * _1334_3314 + _3142_4132 * _1324_2314);

				// non-invertible
				if (Equal<T>(determinant, 0)) {
					return false;
				}

				T inverseDeterminant;
				inverseDeterminant = T(1) / determinant;

				out[0] = (m22 * _3344_4334 - m32 * _2344_4324 + m42 * _2334_3324) * inverseDeterminant;
				out[1] = (-m21 * _3344_4334 + m31 * _2344_4324 - m41 * _2334_3324) * inverseDeterminant;
				out[2] = (m24 * _3142_4132 - m34 * _2142_4122 + m44 * _2132_3122) * inverseDeterminant;
				out[3] = (-m23 * _3142_4132 + m33 * _2142_4122 - m43 * _2132_3122) * inverseDeterminant;
				out[4] = (-m12 * _3344_4334 + m32 * _1344_4314 - m42 * _1334_3314) * inverseDeterminant;
				out[5] = (m11 * _3344_4334 - m31 * _1344_4314 + m41 * _1334_3314) * inverseDeterminant;
				out[6] = (-m14 * _3142_4132 + m34 * _1142_4112 - m44 * _1132_3112) * inverseDeterminant;
				out[7] = (m13 * _3142_4132 - m33 * _1142_4112 + m43 * _1132_3112) * inverseDeterminant;
				out[8] = (m12 * _2344_4324 - m22 * _1344_4314 + m42 * _1324_2314) * inverseDeterminant;
				out[9] = (-m11 * _2344_4324 + m21 * _1344_4314 - m41 * _1324_2314) * inverseDeterminant;
				out[10] = (m14 * _2142_4122 - m24 * _1142_4112 + m44 * _1122_2112) * inverseDeterminant;
				out[11] = (-m13 * _2142_4122 + m23 * _1142_4112 - m43 * _1122_2112) * inverseDeterminant;
				out[12] = (-m12 * _2334_3324 + m22 * _1334_3314 - m32 * _1324_2314) * inverseDeterminant;
				out[13] = (m11 * _2334_3324 - m21 * _1334_3314 + m31 * _1324_2314) * inverseDeterminant;
				out[14] = (-m14 * _2132_3122 + m24 * _1132_3112 - m34 * _1122_2112) * inverseDeterminant;
				out[15] = (m13 * _2132_3122 - m23 * _1132_3112 + m33 * _1122_2112) * inverseDeterminant;

				return true;
			}

		};

	}
}
