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
TEST(Photo, rgb)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "IMG_0428.JPG");
		Photo photo(file_name.c_str());

		XString make = photo.GetMake();
		XString model = photo.GetModel();

		CHECK(make == "Canon");
		CHECK(model == "Canon DIGITAL IXUS 120 IS");

		CHECK(photo.is_rgb() == true);
		CHECK(photo.is_thermal() == false);

		XString band_name = photo.GetBandName();
		CHECK(band_name == "RGB");
		int band_index = photo.GetBandIndex();
		LONGS_EQUAL(0, band_index);

		// actually 3000 x 4000, but think both exif libs "interpret" based on Orientation
		LONGS_EQUAL(4000, photo.GetWidth());
		LONGS_EQUAL(3000, photo.GetHeight());
		LONGS_EQUAL(6, photo.GetOrientation());

		double lat = photo.GetLatitude();
		double lon = photo.GetLongitude();
		double alt = photo.GetAltitude();
		DOUBLES_EQUAL(46.5531566000, lat, 1E-9);
		DOUBLES_EQUAL(6.6841704000, lon, 1E-9);
		DOUBLES_EQUAL(980.2969924812, alt, 1E-9);

		DOUBLES_EQUAL(0.002, photo.GetExposureTime(), 1E-4);
		LONGS_EQUAL(125, photo.GetIsoSpeed());
		double fnumber = photo.GetFNumber();
		DOUBLES_EQUAL(2.8, fnumber, 0.01);

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

//----------------------------------------------------------------------------
TEST(Photo, thermal)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "DJI_0058.JPG");
		Photo photo(file_name.c_str());

		XString make = photo.GetMake();
		XString model = photo.GetModel();

		CHECK(make == "DJI");
		CHECK(model == "FLIR");

		CHECK(photo.is_thermal() == true);
		CHECK(photo.is_rgb() == false);

		XString band_name = photo.GetBandName();
		CHECK(band_name == "LWIR");
		int band_index = photo.GetBandIndex();
		LONGS_EQUAL(0, band_index);

		int w = photo.GetWidth();
		int h = photo.GetHeight();
		LONGS_EQUAL(640, photo.GetWidth());
		LONGS_EQUAL(512, photo.GetHeight());
		int orient = photo.GetOrientation();

		double lat = photo.GetLatitude();
		double lon = photo.GetLongitude();
		double alt = photo.GetAltitude();
		DOUBLES_EQUAL( 46.5175318333, lat, 1E-9);
		DOUBLES_EQUAL(  6.5630358333, lon, 1E-9);
		DOUBLES_EQUAL(318.3588260, alt, 1E-6);

		int bit_depth = photo.GetBitsPerSample();		//TODO:
		//LONGS_EQUAL(, photo.GetBitsPerSample());		//TODO:

		// believe simply doesn't have
		DOUBLES_EQUAL(0.0, photo.GetExposureTime(), 0.0);
		LONGS_EQUAL(0, photo.GetIsoSpeed());

		double fnumber = photo.GetFNumber();
		DOUBLES_EQUAL(1.4, fnumber, 0.01);

		XString date = photo.GetDateTime();
		CHECK(date == "2019:02:05 13:49:04");
		
		__int64 unix_epoch = photo.GetEpoch();
		LONGS_EQUAL(1549374544, unix_epoch);
		              
		double focal_ratio = photo.GetFocalRatio();
		DOUBLES_EQUAL(0.84999, focal_ratio, 1E-3);

		double focal_length = photo.GetFocalLength();
		DOUBLES_EQUAL(9.0, focal_length, 0.0);

		CHECK(photo.has_geo() == true);
		CHECK(photo.has_ypr() == true);
		CHECK(photo.has_speed() == false);

		XString camera_str_osfm = photo.GetCameraStrOSFM();
		XString camera_str_odm = photo.GetCameraStrODM();
		CHECK(camera_str_osfm == "v2 dji flir 640 512 brown 0.8500");
		CHECK(camera_str_odm == "dji flir 640 512 brown 0.8500");
	}
}
