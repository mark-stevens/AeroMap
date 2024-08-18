// StageDEM.cpp
// Create DTM/DSM.
//

#include "DEM.h"
#include "Gsd.h"
#include "LidarModel.h"
#include "StageDEM.h"

int StageDEM::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_dem
	//			dtm.tif
	//			dsm.tif
	//

	int status = 0;

	BenchmarkStart();
	GetApp()->LogWrite("Create DTM/DSM...");

	// define paths and create working directories
	AeroLib::CreateFolder(tree.odm_dem);

	XString dem_input = tree.odm_georeferencing_model_laz;
	bool pc_model_found = QFile::exists(dem_input.c_str());
	bool ignore_resolution = false;
	bool pseudo_georeference = false;

	AeroLib::Georef georef = AeroLib::ReadGeoref();

	if (georef.is_valid == false)
	{
		Logger::Write(__FUNCTION__, "Not georeferenced, using ungeoreferenced point cloud...");
		ignore_resolution = true;
		pseudo_georeference = true;
	}

	double resolution = Gsd::cap_resolution(
		arg.dem_resolution, 
		tree.opensfm_reconstruction,
		0.1,									// error estimate
		1.0,									// scaling
		arg.ignore_gsd,							// ignore gsd
		ignore_resolution && arg.ignore_gsd,	// ignore scaling
		false);									// TODO: has_gcp=reconstruction.has_gcp()

	if (arg.pc_classify && pc_model_found)
	{
		XString pc_classify_marker = XString::CombinePath(tree.odm_dem, "pc_classify_done.txt");

		if ((QFile::exists(pc_classify_marker.c_str()) == false) || Rerun())
		{
			Logger::Write(__FUNCTION__, "Classifying '%s' using Simple Morphological Filter (1/2)", dem_input.c_str());
			DEM::classify(dem_input,
				arg.smrf_scalar,
				arg.smrf_slope,
				arg.smrf_threshold,
				arg.smrf_window);

			//TODO:
			//if the previous step classified the point cloud, what does this do?
			Logger::Write(__FUNCTION__, "Classifying {} using OpenPointClass (2/2)", dem_input.c_str());
		    //classify(dem_input, arg.max_concurrency)

			FILE* pFile = fopen(pc_classify_marker.c_str(), "wt");
			if (pFile)
			{
				fprintf(pFile, "Classify: smrf\n");
				fprintf(pFile, "Scalar: %0.2f\n", arg.smrf_scalar);
				fprintf(pFile, "Slope: %0.2f\n", arg.smrf_slope);
				fprintf(pFile, "Threshold: %0.2f\n", arg.smrf_threshold);
				fprintf(pFile, "Window: %0.2f\n", arg.smrf_window);

				fclose(pFile);
			}
		}
	}

	// Do we need to process anything here?
	if ((arg.dsm || arg.dtm) && pc_model_found)
	{
		if ((arg.dtm && QFile::exists(tree.odm_dem_dtm.c_str()) == false) ||
			(arg.dsm && QFile::exists(tree.odm_dem_dsm.c_str()) == false) ||
			Rerun())
		{
			std::vector<XString> products;

			if (arg.dsm || (arg.dtm && arg.dem_euclidean_map))
				products.push_back("dsm");
			if (arg.dtm)
				products.push_back("dtm");

			std::vector<double> radius_steps = DEM::get_dem_radius_steps(tree.filtered_point_cloud_stats, arg.dem_gapfill_steps, resolution);

			for (XString product : products)
			{
				XString output_type = "max";
				if (product == "dtm")
					output_type = "idw";

				int max_tiles = GetProject().GetImageCount() / 2;

				DEM::create_dem(
					dem_input,						// input point cloud
					product,						// dem type
					radius_steps,
					output_type,
					arg.dem_gapfill_steps > 0,		// gap fill flag
					tree.odm_dem,					// output dir
					resolution / 100.0,				// why /100?
					arg.max_concurrency,
					4096,							// max tiles
					arg.dem_decimation,
					arg.dem_euclidean_map,			// keep unfilled copy
					true,							// apply smoothing
					max_tiles);

				XString dem_geotiff_path = XString::CombinePath(tree.odm_dem, XString::Format("%s.tif", product.c_str()));
				XString bounds_file_path = XString::CombinePath(tree.odm_georef_path, "odm_georeferenced_model.bounds.gpkg");

				//if arg.crop > 0 or arg.boundary:
				{
					//    # Crop DEM
					//    Cropper.crop(bounds_file_path, dem_geotiff_path, utils.get_dem_vars(args), keep_original=not arg.optimize_disk_space)
				}

				if (arg.dem_euclidean_map)
				{
					//unfilled_dem_path = io.related_file_path(dem_geotiff_path, postfix=".unfilled")

					//if arg.crop > 0 or arg.boundary:
					//    # Crop unfilled DEM
					//    Cropper.crop(bounds_file_path, unfilled_dem_path, utils.get_dem_vars(args), keep_original=not arg.optimize_disk_space)

					//commands.compute_euclidean_map(unfilled_dem_path,
					//                    io.related_file_path(dem_geotiff_path, postfix=".euclideand"),
					//                    overwrite=True)
				}

				//if pseudo_georeference:
				//    pseudogeo.add_pseudo_georeferencing(dem_geotiff_path)

				//if arg.tiles:
				//    generate_dem_tiles(dem_geotiff_path, tree.path("%s_tiles" % product), arg.max_concurrency)

				//if arg.cog:
				//    convert_to_cogeo(dem_geotiff_path, max_workers=arg.max_concurrency)
			}
		}
		else
		{
			Logger::Write(__FUNCTION__, "Found existing outputs in: '%s'", tree.odm_dem.c_str());
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "DEM will not be generated");
	}

	BenchmarkStop("Create DTM/DSM");

	return status;
}
