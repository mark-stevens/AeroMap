// TestPhoto.cpp
// CppUnitLite test harness for Photo class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "Photo.h"						// interface to class under test

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	SetDataPath(argv[0]);

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(Photo, test1)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "IMG_0428.JPG");
		Photo photo(file_name.c_str());

		XString make = photo.GetMake();
		XString model = photo.GetModel();

		CHECK(make == "Canon");
	}
}
