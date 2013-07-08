#include <XREXAll.hpp>

#include "TestFile.hpp"

#include <iostream>



using namespace XREX;
using namespace std;

void TestFile::SQRTSpeedTest()
{
	uint32 N = 1000000;
	vector<float> results(N, 0);
	vector<float> todo(N);
	for (uint32 i = 0; i < N; ++i)
	{
		todo[i] = float(i);
	}

	Timer t;
	cout << ReciprocalSqrt(0.0f) << endl;

	for (uint32 i = 0; i < N; ++i)
	{
		results[i] = 1 / sqrt(todo[i]);
	}
	for (uint32 i = 0; i < N; ++i)
	{
		results[i] = ReciprocalSqrt(todo[i]);
	}

	t.Restart();
	float start1 = static_cast<float>(t.Elapsed());
	for (uint32 i = 0; i < N; ++i)
	{
		results[i] = 1 / sqrt(todo[i]);
		//results[i] = sqrt(todo[i]);
	}
	float end1 = static_cast<float>(t.Elapsed());
	float result1 = end1 - start1;
	cout << result1 << endl;
	t.Restart();
	float start2 = static_cast<float>(t.Elapsed());
	for (uint32 i = 0; i < N; ++i)
	{
		results[i] = ReciprocalSqrt(todo[i]);
		//results[i] = 1 / ReciprocalSqrt(todo[i]);
	}
	float end2 = static_cast<float>(t.Elapsed());
	float result2 = end2 - start2;
	cout << result2 << endl;
	float delta = result1 - result2;
	cout << delta << endl;
}

void TestFile::TestMath()
{
	floatV4 f40(1, 2, 3, 4);
	floatV4	f41(4, 3, 2, 1);
	VectorT<uint8, 4> ui40(1, 2, 3, 4);
	floatV4 f42(1, 2, 3, 4);
	auto resultA = static_cast<floatV4>(ui40) + f40;
	auto resultM = static_cast<floatV4>(ui40) * f40;
	auto resultS = static_cast<floatV4>(ui40) - f40;
	auto resultD = static_cast<floatV4>(ui40) / f40;
	float length = f41.Length();
	auto normalized = f41.Normalize();
	floatV4 newV0 = normalized * length;
	floatV4 newV1 = length * normalized;
	floatV4 newV2 = newV0 / length;
	if (f40 != f41)
	{
		f41 = -f41;
	}
	if (f42 == f40)
	{
		f42 = f40;
	}
	newV0 = newV1;

	floatM44 m0;
	floatM44 m1(0.5, 0.5, 0, 0,
		-0.5, 0.5, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	m0 = floatM44::Identity;
	floatM44 m2 = m1.Transpose();
	floatM44 m3 = m1.Inverse();
	floatM44 m4 = m1 * m1;
	if (m2 == m3)
	{
		m0 = -m2;
	}
	m0 = 2 * m0 * 2;
	m0 = m0 / 4;

	floatV4 start(1, 0, 0, 1);
	floatV4 resultTrans = Transform(m1, start);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);
	resultTrans = Transform(m1, resultTrans);

	floatV3 normal(1, 0, 0);
	floatV3 resultNormal = TransformDirection(m1, normal);
	resultNormal = TransformDirection(m1, resultNormal);
	resultNormal = TransformDirection(m1, resultNormal);

	floatM44 trans = TranslationMatrix(floatV3(1, 2, 3));
	floatM44 rot = RotationMatrix(PI / 4, floatV3(0, 1, 0));
	rot = RotationMatrixX(PI / 4);
	rot = RotationMatrixY(PI / 4);
	rot = RotationMatrixZ(PI / 4);
	floatM44 sc = ScalingMatrix(floatV3(1, 2, 1));
	floatM44 rotft = RotationMatrixFromTo(floatV3(0, 1, 0), floatV3(1, 0, 1));

	floatM44 frustum = FrustumProjectionMatrix(PI / 4, 8.0f / 6.0f, 1.0f, 10.0f);

	floatQ quat0(sin(PI / 2 / 2) * floatV3(1, 1, 0).Normalize(), cos(PI / 2 / 2));
	floatQ quat1 = quat0.Normalize();
	floatQ quat2 = quat0.Conjugate();
	float angle = PI * 0.15f;
	floatQ quat3 = RotationQuaternion(angle, floatV3(-1, -1, 1));
	floatM44 mat3 = RotationMatrix(angle, floatV3(-1, -1, 1));
	floatV3 axisQ = quat0.V();
	if (quat3.V() != quat0.V())
	{
		axisQ = quat3.V();
	}
	floatV3 vecToRotate = floatV3(1, 1, 0);
	floatM44 tempMat = MatrixFromQuaternion(quat3);
	floatQ resQ = QuaternionFromMatrix(tempMat);
	floatV3 rotRes0 = RotateByQuaternion(quat3, vecToRotate);
	floatM44 fromQuat3 = MatrixFromQuaternion(quat3);
	floatV3 rotRes1 = Transform(fromQuat3, vecToRotate);
	floatV3 rotRes2 = Transform(mat3, vecToRotate);
	floatQ quatFromMat3 = QuaternionFromMatrix(mat3);
	floatV3 rotRes3 = RotateByQuaternion(quatFromMat3, vecToRotate);


	floatV3 rfrom = floatV3(1, 0, 0);
	floatV3 rto = floatV3(0, 1, 0);
	floatV3 toRotate0 = floatV3(0, 0, -1);
	floatM44 rotateftm = RotationMatrixFromTo(rfrom, rto);
	floatQ rotateftq = RotationQuaternionFromTo(rfrom, rto);
	floatV3 rftmr0 = Transform(rotateftm, toRotate0);
	floatV3 rftqr0 = RotateByQuaternion(rotateftq, toRotate0);
	floatV3 rftmr1 = Transform(rotateftm, rfrom);
	floatV3 rftqr1 = RotateByQuaternion(rotateftq, rfrom);

	floatV3 front = floatV3(1, 0, 0);
	floatV3 up = floatV3(0, 1, 0);
	floatV3 to0 = floatV3(0, 0, 1);
	floatV3 to1 = floatV3(-1, -1, -1);
	floatQ ftq0 = FaceToQuaternion(to0, up, front, up);
	floatM44 ftm0 = FaceToMatrix(to0, up, front, up);
	floatQ ftq1 = FaceToQuaternion(to1, up, front, up);
	floatM44 ftm1 = FaceToMatrix(to1, up, front, up);
	floatV3 vecToRotateFaceTo0 = floatV3(1, 0, 0);
	floatV3 vecToRotateFaceTo1 = floatV3(0, 1, 0);

	floatV3 ftrrq00 = RotateByQuaternion(ftq0, vecToRotateFaceTo0);
	floatV3 ftrrm00 = Transform(ftm0, vecToRotateFaceTo0);

	floatV3 ftrrq01 = RotateByQuaternion(ftq0, vecToRotateFaceTo1);
	floatV3 ftrrm01 = Transform(ftm0, vecToRotateFaceTo1);

	floatV3 ftrrq10 = RotateByQuaternion(ftq1, vecToRotateFaceTo0);
	floatV3 ftrrm10 = Transform(ftm1, vecToRotateFaceTo0);

	floatV3 ftrrq11 = RotateByQuaternion(ftq1, vecToRotateFaceTo1);
	floatV3 ftrrm11 = Transform(ftm1, vecToRotateFaceTo1);
}

void TestFile::TestTransformation()
{
	TransformationSP parent0 = MakeSP<Transformation>();
	TransformationSP parent1 = MakeSP<Transformation>();
	TransformationSP child = MakeSP<Transformation>();
	parent0->Rotate(RadianFromDegree(90), 0, 1, 0);
	parent0->Translate(1, 0, 0);
	floatM44 p0m = parent0->GetWorldMatrix();
	parent1->SetParent(parent0);
	floatM44 p1m = parent1->GetWorldMatrix();
	parent1->Rotate(RadianFromDegree(90), 0, -1, 0);
	parent1->Translate(0, 0, -1);
	floatM44 p1m1 = parent1->GetWorldMatrix();

	child->Translate(1, 0, 0);
	child->SetParent(parent1);
	floatM44 cm = child->GetWorldMatrix();

	parent0->Translate(-1, 0, 0);
	floatM44 cm1 = child->GetWorldMatrix();
}

template <uint32 N>
struct MyStruct
{
	template <typename T>
	MyStruct(T& a);
	operator int() const
	{
		return 1;
	}
};

template <uint32 N>
template <typename T>
MyStruct<N>::MyStruct(T& a)
{

}

template <>
template <typename T>
MyStruct<3>::MyStruct(T& a)
{

}

template <>
template <>
MyStruct<3>::MyStruct(int& a)
{

}

template MyStruct<1>;
template MyStruct<2>;

void Temp()
{
	int i = 3;
	MyStruct<3> ms0(i);
	floatV3 fv3;
	MyStruct<4> ms1(fv3);
	double da[] = {3.0};
	sort(da, da + extent<decltype(da)>::value, [] (double, double) {return true;});
	double d;
	MyStruct<4> ms4(d);
	//int msi = static_cast<int>(ms4);
	if (ms4)
	{
		cout << "true" << endl;
	}
}
