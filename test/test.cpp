#include "types/types.hpp"

int main() {

	//Test floating point conversions
	//Expected output:

	//0 = 0 != f16 0 (0, 0)
	//80000000 = -0 != f16 8000 (-0, 80000000)
	//3f800000 = 1 != f16 3c00 (1, 3f800000)
	//bf800000 = -1 != f16 bc00 (-1, bf800000)
	//3f000000 = 0.5 != f16 3800 (0.5, 3f000000)
	//3e800000 = 0.25 != f16 3400 (0.25, 3e800000)
	//3e000000 = 0.125 != f16 3000 (0.125, 3e000000)
	//3eaaaaab = 0.333333 != f16 3555 (0.333252, 3eaaa000)
	//38002000 = 3.05474e-05 != f16 1 (3.05474e-05, 38002000)
	//47000000 = 32768 != f16 7800 (32768, 47000000)
	//477fe000 = 65504 != f16 7bff (65504, 477fe000)
	//10001999 = 2.52633e-29 != f16 0 (0, 0)
	//2000 = 1.14794e-41 != f16 0 (0, 0)
	//48000000 = 131072 != f16 7c00 (inf, 7f800000)
	//477ff000 = 65520 != f16 7c00 (inf, 7f800000)
	//40490fdb = 3.14159 != f16 4248 (3.14063, 40490000)
	//402d70a4 = 2.71 != f16 416b (2.70898, 402d6000)
	//7f800000 = inf != f16 7c00 (inf, 7f800000)
	//ff800000 = -inf != f16 fc00 (-inf, ff800000)
	//ff800001 = -nan != f16 ffff (-nan, ffffffff)
	//7f800001 = nan != f16 7fff (nan, 7fffffff)
	//40a9999a = 5.3 != f16 454c (5.29688, 40a98000)

	u32 testValues[] = {
		0x00000000,		//0
		0x80000000,		//-0
		0x3f800000,		//1
		0xbf800000,		//-1
		0x3f000000,		//0.5
		0x3e800000,		//0.25
		0x3e000000,		//0.125
		0x3eaaaaab,		//0.33333
		0x38002000,		//minimum half value (3.05473804474e-05)
		0x47000000,		//Maximum exponent (32768)
		0x477fe000,		//Maximum half value (65504)
		0x10001999,		//Minimum exponent, but mantissa can't be represented (0)
		0x00002000,		//Minimum mantissa, but exponent can't be represented (0)
		0x48000000,		//Higher than maximum exponent (inf)
		0x477ff000,		//Higher than maximum value
		0x40490fdb,		//~pi
		0x402d70a4,		//~e
		0x7f800000,		//inf
		0xff800000,		//-inf
		0xff800001,		//-nan
		0x7f800001,		//nan
		0x40a9999a		//5.3
	};

	static constexpr usz j = sizeof(testValues) / sizeof(testValues[0]);

	for (usz i = 0; i < j; ++i) {

		auto &tv = testValues[i];
		auto &tvf = (f32&)tv;
		f32 representedValue = f32(f16(tvf));

		oic::System::log()->debug(
			std::hex, tv, " = ",
			std::dec, tvf, " != f16 ",
			std::hex, f16(tvf).value, " (",
			std::dec, representedValue, ", ",
			std::hex, *(const u32*) &representedValue, ")"
		);
	}

	return 0;
}