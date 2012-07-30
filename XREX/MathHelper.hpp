/*
*	Code partially come from KlayGE, with some simplification and improvement.
*/
#pragma once


namespace MathHelper
{

	template <typename T, uint32 N>
	struct TransformHelper
	{
		static void Do(T out[N], T const m[16], T const v[N], T lastComponent);
	};

	template <typename T>
	struct TransformHelper<T, 4>
	{
		static void Do(T out[4], T const m[16], T const v[4], T lastComponent)
		{
			out[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * v[3];
			out[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * v[3];
			out[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
			out[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
		}
	};
	template <typename T>
	struct TransformHelper<T, 3>
	{
		static void Do(T out[3], T const m[16], T const v[3], T lastComponent)
		{
			out[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * lastComponent;
			out[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * lastComponent;
			out[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * lastComponent;
		}
	};
	template <typename T>
	struct TransformHelper<T, 2>
	{
		static void Do(T out[2], T const m[16], T const v[2], T lastComponent)
		{
			out[0] = m[0] * v[0] + m[4] * v[1] + m[12] * lastComponent;
			out[1] = m[1] * v[0] + m[5] * v[1] + m[13] * lastComponent;
		}
	};

	template <typename T, uint32 N>
	struct VectorHelper
	{
		template <typename U>
		static void DoCopy(T out[N], U const rhs[N])
		{
			out[0] = static_cast<T>(rhs[0]);
			VectorHelper<T, N - 1>::DoCopy(out + 1, rhs + 1);
		}

		static void DoAssign(T out[N], T const & rhs)
		{
			out[0] = rhs;
			VectorHelper<T, N - 1>::DoAssign(out + 1, rhs);
		}

		static void DoAdd(T out[N], T const lhs[N], T const rhs[N])
		{
			out[0] = lhs[0] + rhs[0];
			VectorHelper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs + 1);
		}

		static void DoAdd(T out[N], T const lhs[N], T const & rhs)
		{
			out[0] = lhs[0] + rhs;
			VectorHelper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs);
		}

		static void DoSubtract(T out[N], T const lhs[N], T const rhs[N])
		{
			out[0] = lhs[0] - rhs[0];
			VectorHelper<T, N - 1>::DoSubtract(out + 1, lhs + 1, rhs + 1);
		}

		static void DoSubtract(T out[N], T const lhs[N], T const & rhs)
		{
			out[0] = lhs[0] - rhs;
			VectorHelper<T, N - 1>::DoSubtract(out + 1, lhs + 1, rhs);
		}

		static void DoMultiply(T out[N], T const lhs[N], T const rhs[N])
		{
			out[0] = lhs[0] * rhs[0];
			VectorHelper<T, N - 1>::DoMultiply(out + 1, lhs + 1, rhs + 1);
		}

		static void DoScale(T out[N], T const lhs[N], T const & rhs)
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

		static void DoAssign(T out[1], T const & rhs)
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

		static void DoSubtract(T out[1], T const lhs[1], T const & rhs)
		{
			out[0] = lhs[0] - rhs;
		}

		static void DoMultiply(T out[1], T const lhs[1], T const rhs[1])
		{
			out[0] = lhs[0] * rhs[0];
		}

		static void DoScale(T out[1], T const lhs[1], T const & rhs)
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
			// subscript: row, column
			T l11 = lhs[0], l21 = lhs[1], l31 = lhs[2], l41 = lhs[3],
				l12 = lhs[4], l22 = lhs[5], l32 = lhs[6], l42 = lhs[7],
				l13 = lhs[8], l23 = lhs[9], l33 = lhs[10], l43 = lhs[11],
				l14 = lhs[12], l24 = lhs[13], l34 = lhs[14], l44 = lhs[15];
			// subscript: row, column
			T r11 = rhs[0], r21 = rhs[1], r31 = rhs[2], r41 = rhs[3],
				r12 = rhs[4], r22 = rhs[5], r32 = rhs[6], r42 = rhs[7],
				r13 = rhs[8], r23 = rhs[9], r33 = rhs[10], r43 = rhs[11],
				r14 = rhs[12], r24 = rhs[13], r34 = rhs[14], r44 = rhs[15];

			out[0] = r11 * l11 + r21 * l12 + r31 * l13 + r41 * l14;
			out[1] = r11 * l21 + r21 * l22 + r31 * l23 + r41 * l24;
			out[2] = r11 * l31 + r21 * l32 + r31 * l33 + r41 * l34;
			out[3] = r11 * l41 + r21 * l42 + r31 * l43 + r41 * l44;
			out[4] = r12 * l11 + r22 * l12 + r32 * l13 + r42 * l14;
			out[5] = r12 * l21 + r22 * l22 + r32 * l23 + r42 * l24;
			out[6] = r12 * l31 + r22 * l32 + r32 * l33 + r42 * l34;
			out[7] = r12 * l41 + r22 * l42 + r32 * l43 + r42 * l44;
			out[8] = r13 * l11 + r23 * l12 + r33 * l13 + r43 * l14;
			out[9] = r13 * l21 + r23 * l22 + r33 * l23 + r43 * l24;
			out[10] = r13 * l31 + r23 * l32 + r33 * l33 + r43 * l34;
			out[11] = r13 * l41 + r23 * l42 + r33 * l43 + r43 * l44;
			out[12] = r14 * l11 + r24 * l12 + r34 * l13 + r44 * l14;
			out[13] = r14 * l21 + r24 * l22 + r34 * l23 + r44 * l24;
			out[14] = r14 * l31 + r24 * l32 + r34 * l33 + r44 * l34;
			out[15] = r14 * l41 + r24 * l42 + r34 * l43 + r44 * l44;
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

			// non-invertable
			if (std::numeric_limits<T>::epsilon() >= determinant) {
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
