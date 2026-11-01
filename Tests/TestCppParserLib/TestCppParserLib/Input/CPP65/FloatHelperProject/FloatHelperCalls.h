//BaseLib FloatHelper calls, worked out on the 65C64 (Main.cpp) and natively (TestCPP65GeneratorBaseLib), which
//must agree bit for bit.  FLOAT32, FLOAT64 and INTEGER keep a result.  No long double (it is 80 bits natively).
{
	float32		f;
	float64		d;
	float32		fCos;
	float64		dCos;
	float64		dSin;
	int			i;

	FLOAT32(minf(1.5f, -2.0f));
	FLOAT32(maxf(1.5f, -2.0f));
	INTEGER(FloatEqual(0.1f + 0.2f, 0.3f));
	INTEGER(FloatEqual(1.0f, 1.0001f, 0.001f));
	INTEGER(FloatGreaterThanOrEqual(1.0f, 1.0f));
	INTEGER(FloatLessThan(-0.0f, 0.0f));
	INTEGER(FloatEqual(0.1 + 0.2, 0.3));
	INTEGER(DoubleGreaterThan(2.0, 1.9999999999));
	INTEGER(DoubleLessThanOrEqual(1e-300, 0.0));
	for (i = -4; i <= 4; i++)
	{
		FLOAT32(TruncateFloat(3.14159265f, i));
		FLOAT32(RoundFloat(-2.71828f, i));
		FLOAT64(TruncateFloat(3.14159265358979, (int64)i));
		FLOAT64(RoundFloat(-2.718281828459045, (int64)i));
	}
	FLOAT32(NormaliseFloat(8.0f, 3.0f));
	FLOAT32(NormaliseFloat(10.0f, 2.0f, 7.5f));
	FLOAT32(InterpolateFloat(1.0f, 5.0f, 0.375f));
	InterpolateFloat(&f, -2.0f, 2.0f, 0.1f);
	FLOAT32(f);
	FLOAT64(NormaliseDouble(8.0, 3.0));
	FLOAT64(NormaliseDouble(10.0, 2.0, 7.5));
	FLOAT64(InterpolateDouble(1.0, 5.0, 0.375));
	InterpolateDouble(&d, -2.0, 2.0, 0.1);
	FLOAT64(d);
	INTEGER((int)FloatWholeNumbers(12345.6f));
	INTEGER((int)FloatWholeNumbers(-0.5f));
	INTEGER((int)FloatWholeNumbers(987654.321));
	f = 1.25f;
	fCos = -3.5f;
	SwapFloat(&f, &fCos);
	FLOAT32(f);
	FLOAT32(fCos);
	FLOAT32(Float32NotANumber());
	FLOAT64(Float64NotANumber());
	f = Float32NotANumber();
	INTEGER(IsNotANumber(&f));
	FLOAT32(det2x2(1.5f, 2.0f, -3.0f, 4.25f));
	FLOAT32(det3x3(1.0f, 2.0f, 3.0f, 0.5f, -1.0f, 4.0f, 2.0f, 2.0f, -2.0f));
	FLOAT64(det2x2(1.5, 2.0, -3.0, 4.25));
	FLOAT64(det3x3(1.0, 2.0, 3.0, 0.5, -1.0, 4.0, 2.0, 2.0, -2.0));
	FLOAT32(SquareRoot(2.0f));
	FLOAT64(SquareRoot(2.0));
	FLOAT32(Square(1.1f));
	FLOAT64(Square(1.1));
	CosSinDegrees(270.0, &dCos, &dSin);
	FLOAT64(dCos);
	FLOAT64(dSin);
	CosSinDegrees(-180.0, &dCos, &dSin);
	FLOAT64(dCos);
	FLOAT64(dSin);
}
