#pragma once

#include <random>


struct Guid {
	unsigned long data1;
	unsigned short data2;
	unsigned short data3;
	unsigned char data4[8];

	bool operator==(const Guid& other) const;
	bool operator<(const Guid& other) const;

	static Guid Generate();
};

    template<>
	struct std::hash<Guid>
	{
		static size_t operator()(const Guid& guid)
		{
			
			return 0;
		}
    };
	