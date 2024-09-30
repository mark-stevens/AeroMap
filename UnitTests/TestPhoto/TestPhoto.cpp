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
		CHECK(model == "Canon DIGITAL IXUS 120 IS");

		// actually 3000 x 4000, but think both exif libs "interpret" based on Orientation
		LONGS_EQUAL(4000, photo.GetWidth());
		LONGS_EQUAL(3000, photo.GetHeight());

		double lat = photo.GetLatitude();
		double lon = photo.GetLongitude();
		double alt = photo.GetAltitude();
		DOUBLES_EQUAL(46.5531566000, lat, 1E-9);
		DOUBLES_EQUAL(  6.6841704000, lon, 1E-9);
		DOUBLES_EQUAL(980.2969924812, alt, 1E-9);

		DOUBLES_EQUAL(0.002, photo.GetExposureTime(), 1E-4);
		LONGS_EQUAL(125, photo.GetIsoSpeed());
		//LONGS_EQUAL(24, photo.GetBitsPerSample());		//TODO:

		XString date = photo.GetDateTime();
		CHECK(date == "2011:03:04 16:42:16");

		__int64 unix_epoch = photo.GetEpoch();
		LONGS_EQUAL(1299256936, unix_epoch);

		double focal_ratio = photo.GetFocalRatio();
		DOUBLES_EQUAL(0.80676390860978453, focal_ratio, 1E-15);

		double focal_length = photo.GetFocalLength();
		DOUBLES_EQUAL(5.0, focal_length, 0.0);

		CHECK(photo.has_geo() == true);
		CHECK(photo.has_ypr() == false);
		CHECK(photo.has_speed() == false);

		XString camera_str_osfm = photo.GetCameraStrOSFM();
		XString camera_str_odm = photo.GetCameraStrODM();

		CHECK(camera_str_osfm == "v2 canon canon digital ixus 120 is 4000 3000 brown 0.8068");
		CHECK(camera_str_odm == "canon canon digital ixus 120 is 4000 3000 brown 0.8068");

	}
}
