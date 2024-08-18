// TestShots.cpp
// CppUnitLite test harness for Shots class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "Shots.h"						// interface to class under test

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(Shots, get_origin)
{
	{
		//np_rot= [-1.05886251  2.80394908 -0.01291668]
		//np_trx= [ 135.50834354  -67.86450303 1003.34887028]
		//ret= [187.76392301 199.06594819 977.13587332]

		cv::Vec3d vec_rot(-1.05886251, 2.80394908, -0.01291668);
		cv::Vec3d vec_trx(135.50834354, -67.86450303, 1003.34887028);

		cv::Vec3d vec_origin = Shots::get_origin(vec_rot, vec_trx);

		double d0 = vec_origin[0];
		double d1 = vec_origin[1];
		double d2 = vec_origin[2];

		DOUBLES_EQUAL(187.76392301, d0, 1E-6);
		DOUBLES_EQUAL(199.06594819, d1, 1E-6);
		DOUBLES_EQUAL(977.13587332, d2, 1E-6);
	}

	{
		//np_rot= [-2.99510791 -0.85414858 -0.23955145]
		//np_trx= [-49.64434616 -95.88867117 977.65226709]
		//ret= [-56.75821567 -79.68333299 978.71968668]

		cv::Vec3d vec_rot(-2.99510791, -0.85414858, -0.23955145);
		cv::Vec3d vec_trx(-49.64434616, -95.88867117, 977.65226709);

		cv::Vec3d vec_origin = Shots::get_origin(vec_rot, vec_trx);

		double d0 = vec_origin[0];
		double d1 = vec_origin[1];
		double d2 = vec_origin[2];

		DOUBLES_EQUAL(-56.75821567, d0, 1E-6);
		DOUBLES_EQUAL(-79.68333299, d1, 1E-5);
		DOUBLES_EQUAL(978.71968668, d2, 1E-6);
	}

	{
		//np_rot= [-2.97107019 -0.9337492  -0.17855985]
		//np_trx= [ 175.16610894 -133.40440433  996.42797556]
		//ret= [-181.97985551 -222.69347201  979.10140486]

		cv::Vec3d vec_rot(-2.97107019, -0.9337492, -0.17855985);
		cv::Vec3d vec_trx(175.16610894, -133.40440433, 996.42797556);

		cv::Vec3d vec_origin = Shots::get_origin(vec_rot, vec_trx);

		double d0 = vec_origin[0];
		double d1 = vec_origin[1];
		double d2 = vec_origin[2];

		DOUBLES_EQUAL(-181.97985551, d0, 1E-6);    
		DOUBLES_EQUAL(-222.69347201, d1, 1E-5);
		DOUBLES_EQUAL(979.10140486, d2, 1E-6);
	}
}

//----------------------------------------------------------------------------
TEST(Shots, get_rotation_matrix)
{
	{
		// rotation = [-1.05886251  2.80394908 - 0.01291668]
		// cv2.Rodrigues(rotation)[0] = 
		//		[[-0.74128687 -0.65693278  0.13759758]
		//		 [-0.65817259  0.75164974  0.0427962]
		//		 [-0.13153941 -0.0588387  -0.98956323]]

		cv::Vec3d vec_rot(-1.05886251, 2.80394908, -0.01291668);
		cv::Mat mat = Shots::get_rotation_matrix(vec_rot);

		double d00 = mat.at<double>(0, 0);
		double d01 = mat.at<double>(0, 1);
		double d02 = mat.at<double>(0, 2);

		double d10 = mat.at<double>(1, 0);
		double d11 = mat.at<double>(1, 1);
		double d12 = mat.at<double>(1, 2);

		double d20 = mat.at<double>(2, 0);
		double d21 = mat.at<double>(2, 1);
		double d22 = mat.at<double>(2, 2);

		DOUBLES_EQUAL(-0.74128687, d00, 1E-8);
		DOUBLES_EQUAL(-0.65693278, d01, 1E-8);
		DOUBLES_EQUAL(0.13759758, d02, 1E-8);

		DOUBLES_EQUAL(-0.65817259, d10, 1E-8);
		DOUBLES_EQUAL(0.75164974, d11, 1E-8);
		DOUBLES_EQUAL(0.0427962, d12, 1E-8);

		DOUBLES_EQUAL(-0.13153941, d20, 1E-8);
		DOUBLES_EQUAL(-0.0588387, d21, 1E-8);
		DOUBLES_EQUAL(-0.98956323, d22, 1E-8);
	}
	{
		// rotation = [-2.99510791 -0.85414858 -0.23955145]
		// cv2.Rodrigues(rotation)[0] = 
		//		[[ 0.83871244  0.52569129  0.14216226]
		//		 [0.52295019 - 0.85031385  0.05907161]
		//		 [0.15193597  0.02479969 - 0.98807917]]

		cv::Vec3d vec_rot(-2.99510791, -0.85414858, -0.23955145);
		cv::Mat mat = Shots::get_rotation_matrix(vec_rot);

		double d00 = mat.at<double>(0, 0);
		double d01 = mat.at<double>(0, 1);
		double d02 = mat.at<double>(0, 2);

		double d10 = mat.at<double>(1, 0);
		double d11 = mat.at<double>(1, 1);
		double d12 = mat.at<double>(1, 2);

		double d20 = mat.at<double>(2, 0);
		double d21 = mat.at<double>(2, 1);
		double d22 = mat.at<double>(2, 2);

		DOUBLES_EQUAL(0.83871244, d00, 1E-8);
		DOUBLES_EQUAL(0.52569129, d01, 1E-8);
		DOUBLES_EQUAL(0.14216226, d02, 1E-8);

		DOUBLES_EQUAL(0.52295019, d10, 1E-8);
		DOUBLES_EQUAL(-0.85031385, d11, 1E-8);
		DOUBLES_EQUAL(0.05907161, d12, 1E-8);

		DOUBLES_EQUAL(0.15193597, d20, 1E-8);
		DOUBLES_EQUAL(0.02479969, d21, 1E-8);
		DOUBLES_EQUAL(-0.98807917, d22, 1E-8);
	}
	{
		// rotation = [-2.97107019 -0.9337492 -0.17855985]
		// cv2.Rodrigues(rotation)[0] = 
		//		[[0.81427237  0.57138011  0.10239765]
		//		 [0.56884652 -0.82057962  0.05534185]
		//		 [0.11564666  0.0131852  -0.9932029]]

		cv::Vec3d vec_rot(-2.97107019, -0.9337492, -0.17855985);
		cv::Mat mat = Shots::get_rotation_matrix(vec_rot);

		double d00 = mat.at<double>(0, 0);
		double d01 = mat.at<double>(0, 1);
		double d02 = mat.at<double>(0, 2);

		double d10 = mat.at<double>(1, 0);
		double d11 = mat.at<double>(1, 1);
		double d12 = mat.at<double>(1, 2);

		double d20 = mat.at<double>(2, 0);
		double d21 = mat.at<double>(2, 1);
		double d22 = mat.at<double>(2, 2);

		DOUBLES_EQUAL(0.81427237, d00, 1E-8);
		DOUBLES_EQUAL(0.57138011, d01, 1E-8);
		DOUBLES_EQUAL(0.10239765, d02, 1E-8);

		DOUBLES_EQUAL(0.56884652, d10, 1E-8);
		DOUBLES_EQUAL(-0.82057962, d11, 1E-8);
		DOUBLES_EQUAL(0.05534185, d12, 1E-8);

		DOUBLES_EQUAL(0.11564666, d20, 1E-8);
		DOUBLES_EQUAL(0.0131852, d21, 1E-8);
		DOUBLES_EQUAL(-0.9932029, d22, 1E-8);
	}
}
