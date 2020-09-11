#pragma once

#include "misc.h"

namespace Converter
{
	class Test
	{
	public:

		Test(SF2::Interface* inInterface, Misc* inOutput);
		~Test();

		void TestAll();
		void TestSimpleAndSave();

	private:

		SF2::Interface* m_SF2;
		Converter::Misc* m_Output;
	};
}