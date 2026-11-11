//BaseLib SQuaternion, worked out on the 65C64 (Main.cpp) and natively (TestCPP65GeneratorBaseLib), which must
//agree bit for bit.  QUATERNION and INTEGER keep a result.  The quaternions are passed and returned by value.
{
	SQuaternion	a(1.0f, 2.0f, 3.0f, 4.0f);
	SQuaternion	b(0.5f, -1.5f, 2.25f, -0.125f);
	SQuaternion	c;
	float		af[4] = { 0.1f, 0.2f, 0.3f, 0.4f };
	SQuaternion	d(af);

	QUATERNION(a + b);
	QUATERNION(a - b);
	QUATERNION(a * 0.3f);
	QUATERNION(0.3f * a);
	QUATERNION(b / 3.0f);
	QUATERNION(-b);
	QUATERNION(+a);
	QUATERNION((a + b) * 2.0f - (a - b) / 4.0f + d);
	c = a;
	c += b;
	c -= d;
	c *= 1.5f;
	c /= 7.0f;
	QUATERNION(c);
	INTEGER(a == a);
	INTEGER(a == b);
	INTEGER(a != b);
	INTEGER((a + b) == (b + a));
	INTEGER((a + b - b) == a);
	INTEGER(sizeof(SQuaternion));
}
