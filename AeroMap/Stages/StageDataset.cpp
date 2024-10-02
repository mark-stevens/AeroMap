// StageDataset.cpp
// Load dataset.
//

#include "StageDataset.h"

int StageDataset::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_georeferencing
	//			coords.txt
	//			odm_georeferencing_model_geo.txt		<- deprecated, likely not needed at all
	//			proj.txt
	//		benchmark.txt
	//		images.json
	//		img_list.txt
	//

	int status = 0;

	BenchmarkStart();
	GetApp()->LogWrite("Load dataset...");

	WriteImageListText();
	WriteImageListJson();

	InitGeoref();

	BenchmarkStop("Load dataset", true);

	return status;
}

int StageDataset::InitGeoref()
{
	// Create & populate georeferencing folder.
	//

	int status = 0;

	XString georef_path = tree.odm_georef_path;
	AeroLib::CreateFolder(georef_path);

	// write 'coords.txt'

	// Create a coordinate file containing the GPS positions of all cameras
	// to be used later in the ODM toolchain for automatic georeferecing

	int utm_zone = 0;
	char hemi = ' ';
	std::vector<VEC3> coords;
	
	for (Photo image : GetProject().GetImageList())
	{
		//	if photo.latitude is None or photo.longitude is None:
		//		log.ODM_WARNING("GPS position not available for %s" % photo.filename)
		//		continue

		if (utm_zone == 0)
		{
			utm_zone = GIS::GetUTMZone(image.GetLongitude());
			hemi = image.GetLatitude() >= 0.0 ? 'N' : 'S';
		}

		double x, y;
		GIS::LatLonToXY_UTM(image.GetLatitude(), image.GetLongitude(), x, y, GIS::Ellipsoid::WGS_84);

		coords.push_back(VEC3(x, y, image.GetAltitude()));
	}

	//if utm_zone is None:
	//	raise Exception("No images seem to have GPS information")

	// Calculate average
	double dx = 0.0;
	double dy = 0.0;
	double num = (double)coords.size();
	for (const VEC3& coord : coords)
	{
		dx += coord.x / num;
		dy += coord.y / num;
	}

	dx = floor(dx);
	dy = floor(dy);

	FILE* pFile = fopen(GetProject().GetCoordsFileName().c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "WGS84 UTM %d%c\n", utm_zone, hemi);
		fprintf(pFile, "%d %d\n", (int)dx, (int)dy);
		for (const VEC3& coord : coords)
			fprintf(pFile, "%0.15f %0.15f %0.15f\n", coord.x - dx, coord.y - dy, coord.z);

		fclose(pFile);
	}

	// write 'odm_georeferencing_model_geo.txt'

	XString file_name = XString::CombinePath(georef_path, "odm_georeferencing_model_geo.txt");
	pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "WGS84 UTM %d%c\n", utm_zone, hemi);
		fprintf(pFile, "%d %d\n", (int)dx, (int)dy);

		fclose(pFile);
	}

	// write 'proj.txt'

	file_name = XString::CombinePath(georef_path, "proj.txt");
	pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "+proj=utm +zone=%d +datum=WGS84 +units=m +no_defs +type=crs\n", utm_zone);

		fclose(pFile);
	}

	return status;
}

int StageDataset::WriteImageListText()
{
	// Write 'img_list.txt'
	//

	int status = 0;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "img_list.txt");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		for (Photo image : GetProject().GetImageList())
		{
			// root name only
			fprintf(pFile, "%s\n", image.GetFileName().GetFileName().c_str());
		}

		fclose(pFile);
	}

	return status;
}

int StageDataset::WriteImageListJson()
{
	// Write 'images.json'
	//

	int status = 0;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "images.json");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "[\n");

		int ctr = 0;
		for (Photo image : GetProject().GetImageList())
		{
			fprintf(pFile, "	{\n");
			fprintf(pFile, "		\"filename\": \"%s\",\n", image.GetFileName().c_str());
			fprintf(pFile, "		\"mask\": null,\n");
			fprintf(pFile, "		\"width\": %d,\n", image.GetWidth());
			fprintf(pFile, "		\"height\": %d,\n", image.GetHeight());
			fprintf(pFile, "		\"camera_make\": \"%s\",\n", image.GetMake().c_str());
			fprintf(pFile, "		\"camera_model\": \"%s\",\n", image.GetModel().c_str());
			fprintf(pFile, "		\"orientation\": %d,\n", image.GetOrientation());
			fprintf(pFile, "		\"latitude\": %0.15f,\n", image.GetLatitude());		//46.553156600003355
			fprintf(pFile, "		\"longitude\": %0.15f,\n", image.GetLongitude());
			fprintf(pFile, "		\"altitude\": %0.12f,\n", image.GetAltitude());		//980.296992481203
			fprintf(pFile, "		\"band_name\": \"RGB\",\n");		// in/derived from exif?
			fprintf(pFile, "		\"band_index\": 0,\n");
			fprintf(pFile, "		\"capture_uuid\": null,\n");
			fprintf(pFile, "		\"fnumber\": %0.1f,\n", image.GetFNumber());
			fprintf(pFile, "		\"radiometric_calibration\": null,\n");
			fprintf(pFile, "		\"black_level\": null,\n");
			fprintf(pFile, "		\"gain\": null,\n");
			fprintf(pFile, "		\"gain_adjustment\": null,\n");
			fprintf(pFile, "		\"exposure_time\": %0.3f,\n", image.GetExposureTime());
			fprintf(pFile, "		\"iso_speed\": %d,\n", image.GetIsoSpeed());
			fprintf(pFile, "		\"bits_per_sample\": null,\n");
			fprintf(pFile, "		\"vignetting_center\": null,\n");
			fprintf(pFile, "		\"vignetting_polynomial\": null,\n");
			fprintf(pFile, "		\"spectral_irradiance\": null,\n");
			fprintf(pFile, "		\"horizontal_irradiance\": null,\n");
			fprintf(pFile, "		\"irradiance_scale_to_si\": null,\n");
			fprintf(pFile, "		\"utc_time\": %I64u000.0,\n", image.GetEpoch());		//1299256936000.0
			fprintf(pFile, "		\"yaw\": null,\n");
			fprintf(pFile, "		\"pitch\": null,\n");
			fprintf(pFile, "		\"roll\": null,\n");
			fprintf(pFile, "		\"omega\": null,\n");
			fprintf(pFile, "		\"phi\": null,\n");
			fprintf(pFile, "		\"kappa\": null,\n");
			fprintf(pFile, "		\"sun_sensor\": null,\n");
			fprintf(pFile, "		\"dls_yaw\": null,\n");
			fprintf(pFile, "		\"dls_pitch\": null,\n");
			fprintf(pFile, "		\"dls_roll\": null,\n");
			fprintf(pFile, "		\"speed_x\": null,\n");
			fprintf(pFile, "		\"speed_y\": null,\n");
			fprintf(pFile, "		\"speed_z\": null,\n");
			fprintf(pFile, "		\"exif_width\": %d,\n", image.GetExifWidth());
			fprintf(pFile, "		\"exif_height\": %d,\n", image.GetExifHeight());
			fprintf(pFile, "		\"gps_xy_stddev\": null,\n");
			fprintf(pFile, "		\"gps_z_stddev\": null,\n");
			fprintf(pFile, "		\"camera_projection\": \"brown\",\n");				// in exif?
			fprintf(pFile, "		\"focal_ratio\": %0.16f\n", image.GetFocalRatio());		//0.8067639086097845
			fprintf(pFile, "	}%s\n", ++ctr < GetProject().GetImageCount() ? "," :  "");
		}

		fprintf(pFile, "]\n");

		fclose(pFile);
	}

	return status;
}
