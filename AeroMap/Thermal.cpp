// Thermal.cpp
// Port of odm thermal.py
//

#include "Thermal.h"

//import cv2
//import os
//from opendm import log
//from opendm.thermal_tools import dji_unpack
//from opendm.exiftool import extract_raw_thermal_image_data
//from opendm.thermal_tools.thermal_utils import sensor_vals_to_temp

cv::Mat Thermal::resize_to_match(cv::Mat image, Photo match_photo)
{
	// Resize images to match the dimension of another photo
	// 
	// Inputs:
	//    image			= image data to resize
	//    match_photo	= photo whose dimensions should be used for resize
	// Outputs:
	//    return		= resized image
	//
	
	int w = image.cols;
	int h = image.rows;
		
	if ((w != match_photo.GetWidth()) || (h != match_photo.GetHeight()))
	{
		double fx = (double)match_photo.GetWidth() / (double)w;
		double fy = (double)match_photo.GetHeight() / (double)h;

		cv::Mat mat;
		cv::Size dsize(0, 0);	// None in python
		cv::resize(image, mat, dsize, fx, fy, cv::INTER_LANCZOS4);
		
		//    image = cv2.resize(image, None,
		//            fx=match_photo.width/w,
		//            fy=match_photo.height/h,
		//            interpolation=cv2.INTER_LANCZOS4)

		return mat;
	}

	return image;
}

cv::Mat Thermal::dn_to_temperature(Photo photo, cv::Mat image, XString images_path)
{
	// Convert Digital Number values to temperature (C) values
	// 
	// Inputs:
	//    photo			= ODM_Photo
	//    image			= numpy array containing image data
	//    images_path	= path to original source image to read data using PIL for DJI thermal photos
	// Outputs:
	//    return = numpy array with temperature (C) image values
	//

	// Handle thermal bands
	if (photo.is_thermal())
	{
		// Every camera stores thermal information differently
		// The following will work for MicaSense Altum cameras
		// but not necessarily for others
		
		if (photo.GetMake().CompareNoCase("MicaSense"))
		{
			if (photo.GetModel().CompareNoCase("Altum"))
			{
				//    image = image.astype("float32")
				//    image -= (273.15 * 100.0) # Convert Kelvin to Celsius
				//    image *= 0.01
				//    return image
			}
		}
		else if (photo.GetMake().CompareNoCase("DJI"))
		{
			if (photo.GetModel().CompareNoCase("ZH20T"))
			{
				//    filename, file_extension = os.path.splitext(photo.filename)
				//    # DJI H20T high gain mode supports measurement of -40~150 celsius degrees
				//    if file_extension.lower() in [".tif", ".tiff"] and image.min() >= 23315: # Calibrated grayscale tif
				//        image = image.astype("float32")
				//        image -= (273.15 * 100.0) # Convert Kelvin to Celsius
				//        image *= 0.01
				//        return image
				//    else:
				//        return image
			}
			else if (photo.GetModel().CompareNoCase("MAVIC2-ENTERPRISE-ADVANCED"))
			{
				//    image = dji_unpack.extract_temperatures_dji(photo, image, images_path)
				//    image = image.astype("float32")
				//    return image
			}
		}
		else
		{
			//    try:
			//        params, image = extract_raw_thermal_image_data(os.path.join(images_path, photo.filename))
			//        image = sensor_vals_to_temp(image, **params)
			//    except Exception as e:
			//        log.ODM_WARNING("Cannot radiometrically calibrate %s: %s" % (photo.filename, str(e)))
		}

		//    image = image.astype("float32")
		//    return image
	}
	else
	{
		//        image = image.astype("float32")
		//        log.ODM_WARNING("Tried to radiometrically calibrate a non-thermal image with temperature values (%s)" % photo.filename)
		//        return image
	}

	return image;
}
