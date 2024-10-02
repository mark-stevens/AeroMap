// Gsd.cpp
// Port of odm gsd.py
//

#include <fstream>
#include <nlohmann/json.h>
using json = nlohmann::json;

#include "AeroLib.h"
#include "Shots.h"
#include "Gsd.h"

double Gsd::cap_resolution(double resolution, XString reconstruction_json, double gsd_error_estimate, double gsd_scaling,
	bool ignore_gsd, bool ignore_resolution, bool has_gcp)
{
    // Inputs:
    //      resolution = resolution in cm / pixel
    //      reconstruction_json = path to OpenSfM's reconstruction.json
    //      gsd_error_estimate = percentage of estimated error in the GSD calculation to set an upper bound on resolution.
    //      gsd_scaling = scaling of estimated GSD.
    //      ignore_gsd = when set to True, forces the function to just return resolution.
    //      ignore_resolution = when set to True, forces the function to return a value based on GSD.
    // Outputs:
    //      return = max value between resolution and the GSD computed from the reconstruction.
    //               If a GSD cannot be computed, or ignore_gsd is set to True, it just returns resolution. Units are in cm / pixel.
    //

    if (ignore_gsd)
        return resolution;

    double gsd = opensfm_reconstruction_average_gsd(reconstruction_json, has_gcp || ignore_resolution);

    if (gsd > 0.0)
    {
        gsd = gsd * (1.0 - gsd_error_estimate) * gsd_scaling;
        if (gsd > resolution || ignore_resolution)
        {
            Logger::Write(__FUNCTION__, 
                "Maximum resolution set to %0.2f * (GSD - %0.2f%) "
                "(%0.2f cm/pixel, requested resolution was %0.2f cm/pixel)",
                gsd_scaling, gsd_error_estimate * 100, gsd, resolution);

            return gsd;
        }
        else
        {
            return resolution;
        }
    }
    else
    {
        Logger::Write(__FUNCTION__, "Unable to calculate GSD, using requested resolution of %0.2f", resolution);
        return resolution;
    }
}

double Gsd::opensfm_reconstruction_average_gsd(XString reconstruction_json, bool use_all_shots)
{
    // Computes the average Ground Sampling Distance of an OpenSfM reconstruction.
    // 
    // Inputs:
    //    reconstruction_json = path to OpenSfM's reconstruction.json
    // Outputs:
    //    return = Ground Sampling Distance value (cm / pixel) or -1.0 if
    //             a GSD estimate cannot be computed
    //

    if (QFile::exists(reconstruction_json.c_str()) == false)
    {
         Logger::Write(__FUNCTION__, reconstruction_json + " does not exist.");
         assert(false);
    }

    std::ifstream f(reconstruction_json.c_str());
    json data = json::parse(f);

    // Calculate median height from sparse reconstruction
    json reconstruction = data[0];

    std::vector<double> point_heights;
    for (json point : reconstruction["points"])
    {
        json coord = point["coordinates"];
        double z = coord[2];
        point_heights.push_back(z);
    }

    double ground_height = AeroLib::Median(point_heights);
    
    std::vector<double> gsds;
    json shots = reconstruction["shots"];
    for (json shot : shots)
    {
        if (use_all_shots || shot.contains("gps_dop"))
        {
            json shot_rot = shot["rotation"];
            json shot_trx = shot["translation"];

            // convert to double, nlohmann recommended way
            double d0 = shot_rot[0].template get<double>();
            double d1 = shot_rot[1].template get<double>();
            double d2 = shot_rot[2].template get<double>();
            cv::Vec3d vec_rot(d0, d1, d2);
            
            d0 = shot_trx[0].template get<double>();
            d1 = shot_trx[1].template get<double>();
            d2 = shot_trx[2].template get<double>();
            cv::Vec3d vec_trx(d0, d1, d2);

            cv::Vec3d shot_origin = Shots::get_origin(vec_rot, vec_trx);
            double shot_height = shot_origin[2];
            // another instance where single camera is assumed
            Photo image = GetProject().GetImageList()[0];
            double focal_ratio = image.GetFocalRatio();
            //focal_ratio = camera.get('focal', camera.get('focal_x'))
            //if not focal_ratio:
            //    log.ODM_WARNING("Cannot parse focal values from %s. This is likely an unsupported camera model." % reconstruction_json)
            //    return None

            double gsd = calculate_gsd_from_focal_ratio(focal_ratio, shot_height - ground_height, image.GetWidth());
            gsds.push_back(gsd);
        }
    }

    if (gsds.size() > 0)
    {
        double mean = AeroLib::Mean(gsds);
        if (mean < 0)
            Logger::Write(__FUNCTION__, "Negative GSD estimated, this might indicate a flipped Z-axis.");
        return abs(mean);
    }

    return -1.0;
}

double Gsd::calculate_gsd_from_focal_ratio(double focal_ratio, double flight_height, int image_width)
{
    // Inputs:
    //      focal_ratio focal length (mm) / sensor_width (mm)
    //      flight_height in meters
    //      image_width in pixels
    // Outputs:
    //      return = Ground Sampling Distance
    //

    if (focal_ratio == 0.0 || image_width == 0)
        return -1.0;

    return ((flight_height * 100.0) / image_width) / focal_ratio;
}

int Gsd::image_max_size(const std::vector<Photo>& photos, double target_resolution, XString reconstruction_json, double gsd_error_estimate,
    bool ignore_gsd, bool has_gcp)
{
    // Inputs:
    //      photos              = images database
    //      target_resolution   = resolution the user wants have in cm / pixel
    //      reconstruction_json = path to OpenSfM's reconstruction.json
    //      gsd_error_estimate  = percentage of estimated error in the GSD calculation to set an upper bound on resolution.
    //      ignore_gsd          = if set to True, simply return the largest side of the largest image in the images database.
    // Outputs:
    //      return  = dimension in pixels calculated by taking the image_scale_factor and applying it to the size of the largest image.
    //                Returned value is never higher than the size of the largest side of the largest image.

    int max_size = 0;

    int max_width = 0;
    int max_height = 0;

    double isf = 1.0;
    if (ignore_gsd)
        isf = 1.0;
    else
        isf = image_scale_factor(target_resolution, reconstruction_json, gsd_error_estimate, has_gcp);
    
    for (Photo image : photos)
    {
        max_width = std::max((int)image.GetWidth(), max_width);
        max_height = std::max((int)image.GetHeight(), max_height);
    }
    
    max_size = ceil(std::max(max_width, max_height)*isf);

    return max_size;
}

double Gsd::image_scale_factor(double target_resolution, XString reconstruction_json, double gsd_error_estimate, bool has_gcp)
{
    // Inputs:
    //      target_resolution   = resolution the user wants have in cm / pixel
    //      reconstruction_json path to OpenSfM's reconstruction.json
    //      gsd_error_estimate percentage of estimated error in the GSD calculation to set an upper bound on resolution.
    // Outputs:
    //      return  = down-scale (<= 1) value to apply to images to achieve the target resolution by comparing the current GSD of the reconstruction.
    //                If a GSD cannot be computed, it just returns 1. 
    //                Returned scale values are never higher than 1 and are always obtained by dividing by 2 (e.g. 0.5, 0.25, etc.)
    //

    double gsd = opensfm_reconstruction_average_gsd(reconstruction_json, has_gcp);
    
    if ((gsd > 0.0) && (target_resolution > 0.0))
    {
        gsd = gsd * (1.0 + gsd_error_estimate);
        double isf = std::min(1.0, abs(gsd) / target_resolution);
        double ret = 0.5;
        while (ret >= isf)
            ret /= 2.0;
        return ret * 2.0;
    }
    else
    {
        return 1.0;
    }
}

//def calculate_gsd(sensor_width, flight_height, focal_length, image_width):
//    """
//    :param sensor_width in millimeters
//    :param flight_height in meters
//    :param focal_length in millimeters
//    :param image_width in pixels
//    :return Ground Sampling Distance
//
//    >>> round(calculate_gsd(13.2, 100, 8.8, 5472), 2)
//    2.74
//    >>> calculate_gsd(13.2, 100, 0, 2000)
//    >>> calculate_gsd(13.2, 100, 8.8, 0)
//    """
//    if sensor_width != 0:
//        return calculate_gsd_from_focal_ratio(focal_length / sensor_width,
//                                                flight_height,
//                                                image_width)
//    else:
//        return None
//
//
//def rounded_gsd(reconstruction_json, default_value=None, ndigits=0, ignore_gsd=False):
//    """
//    :param reconstruction_json path to OpenSfM's reconstruction.json
//    :return GSD value rounded. If GSD cannot be computed, or ignore_gsd is set, it returns a default value.
//    """
//    if ignore_gsd:
//        return default_value
//
//    gsd = opensfm_reconstruction_average_gsd(reconstruction_json)
//
//    if gsd is not None:
//        return round(gsd, ndigits)
//    else:
//        return default_value
//
