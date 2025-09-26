#pragma once

#include <cstdlib>
#include <ctime>

namespace SecurityService
{
	static constexpr int P = 23;
	static constexpr int G = 5;

	inline void Init()
	{
		std::srand((unsigned)std::time(nullptr));
	}

	inline int ModularExponential(int base, int exp, int mod)
	{
		int res = 1;
		base %= mod;

		while (exp > 0)
		{
			if (exp & 1)
			{
				res = (int)((int64_t)res * base % mod);
			}

			base = (int)((int64_t)base * base % mod);
			exp >>= 1;
		}


		return res;
	}

	inline int GenPrivate()
	{
		return 1 + (std::rand() % (P - 2));
	}

	inline int ComputePublic(int priv)
	{
		return ModularExponential(G, priv, P);
	}

	inline int ComputeShared(int receivedPublic, int priv)
	{
		return ModularExponential(receivedPublic, priv, P);
	}
}