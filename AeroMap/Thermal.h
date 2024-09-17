#ifndef THERMAL_H
#define THERMAL_H

#include <opencv2/opencv.hpp>	// OpenCV

#include "AeroMap.h"

class Thermal
{
public:

	static cv::Mat resize_to_match(cv::Mat image, Project::ImageType match_photo);
	static cv::Mat dn_to_temperature(Project::ImageType photo, cv::Mat image, XString images_path);
};

#endif // #ifndef THERMAL_H
