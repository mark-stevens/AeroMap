// StageOrtho.cpp
// Generate orthophoto.
//

#include "GSd.h"
#include "StageOrtho.h"

int StageOrtho::Run()
{
	// Inputs:
    //      + odm_texturing
    //          odm_textured_model_geo.obj
    //      + odm_texturing_25d
    //          odm_textured_model_geo.obj
    // Outputs:
    //      + odm_orthophoto
    //          odm_orthophoto.tif
    //          odm_orthophoto_corners.txt
    //          odm_orthophoto_log.txt
    //

	int status = 0;

	BenchmarkStart();
	GetApp()->LogWrite("Generate orthophoto..");

    // define paths and create working directories
    AeroLib::CreateFolder(tree.odm_orthophoto);

    if (arg.skip_orthophoto)
    {
        Logger::Write(__FUNCTION__, "--skip-orthophoto is set, no orthophoto will be generated");
        return 0;
    }

    if ((QFile::exists(tree.odm_orthophoto_tif.c_str()) == false) || Rerun())
    {
        AeroLib::Georef georef = AeroLib::ReadGeoref();

        double res_cap = Gsd::cap_resolution(arg.orthophoto_resolution, tree.opensfm_reconstruction,
            arg.ignore_gsd,
            (georef.is_valid == false) && arg.ignore_gsd,       // ignore resolution
            GetProject().GetGcpCount() > 0);                    // has gcp

        // why 1/100 here?
        double resolution = 1.0 / (res_cap / 100.0);

        std::vector<XString> models;

        XString base_dir;
        if (arg.use_3dmesh)
            base_dir = tree.odm_texturing;
        else
            base_dir = tree.odm_25dtexturing;

        XString model_file = tree.odm_textured_model_obj;

        double in_paint = -1.0;
        //if reconstruction.multi_camera:
        {
            //    for band in reconstruction.multi_camera:
            //        primary = band['name'] == get_primary_band_name(reconstruction.multi_camera, arg.primary_band)
            //        subdir = ""
            //        if not primary:
            //            subdir = band['name'].lower()
            //        models.append(os.path.join(base_dir, subdir, model_file))
            //    kwargs['bands'] = '-bands %s' % (','.join([double_quote(b['name']) for b in reconstruction.multi_camera]))

            //    # If a RGB band is present,
            //    # use bit depth of the first non-RGB band
            //    depth_idx = None
            //    all_bands = [b['name'].lower() for b in reconstruction.multi_camera]
            //    for b in ['rgb', 'redgreenblue']:
            //        if b in all_bands:
            //            for idx in range(len(all_bands)):
            //                if all_bands[idx] != b:
            //                    depth_idx = idx
            //                    break
            //            break

            //    if depth_idx is not None:
        //        kwargs['depth_idx'] = '-outputDepthIdx %s' % depth_idx
        }
        //else:
        {
            models.push_back(XString::CombinePath(base_dir, model_file));

            // Perform edge inpainting on georeferenced RGB datasets
            if (georef.is_valid)
                in_paint = 1.0;

            //    # Thermal dataset with single band
            //    if reconstruction.photos[0].band_name.upper() == "LWIR":
            //        kwargs['bands'] = '-bands lwir'
        }

        // run odm_orthophoto
        QStringList args;
        args.push_back("-inputFiles");
        for (XString model : models)
            args.push_back(model.c_str());
        args.push_back("-logFile");
        args.push_back(tree.odm_orthophoto_log.c_str());
        args.push_back("-outputFile");
        if (georef.is_valid)
        {
            args.push_back(tree.odm_orthophoto_tif.c_str());  // Render directly to final file

            args.push_back("-utm_north_offset");
            args.push_back(XString::Format("%0.1f", georef.y).c_str());
            args.push_back("-utm_east_offset");
            args.push_back(XString::Format("%0.1f", georef.x).c_str());
            args.push_back("-a_srs");
            args.push_back(XString::Format("+proj=utm +zone=%d +datum=WGS84 +units=m +no_defs +type=crs", georef.utm_zone).c_str());
        }
        else
        {
            args.push_back(tree.odm_orthophoto_render.c_str());
        }
        args.push_back("-outputCornerFile");
        args.push_back(tree.odm_orthophoto_corners.c_str());
        args.push_back("-resolution");
        args.push_back(XString::Format("%0.1f", resolution).c_str());
        if (in_paint > 0.0)
        {
            args.push_back("-inpaintThreshold");
            args.push_back(XString::Format("%0.1f", in_paint).c_str());
        }
        args.push_back("-co");
        args.push_back("TILED=YES");
        args.push_back("-co");
        args.push_back("COMPRESS=DEFLATE");
        args.push_back("-co");
        args.push_back("PREDICTOR=2");
        args.push_back("-co");
        args.push_back("BIGTIFF=IF_SAFER");
        args.push_back("-co");
        args.push_back("BLOCKXSIZE=512");
        args.push_back("-co");
        args.push_back("BLOCKYSIZE=512");
        args.push_back("-co");
        args.push_back(XString::Format("NUM_THREADS=%d", arg.max_concurrency).c_str());
        args.push_back("--config");
        args.push_back("GDAL_CACHEMAX");
        args.push_back("10411063296.0");        //TODO: (get_max_memory_mb() * 1024 * 1024)
        args.push_back("-verbose");
        AeroLib::RunProgramEnv(tree.prog_orthophoto, args);
        // cmd: odm_orthophoto 
        //          -inputFiles "d:/test_odm/odm_texturing_25d/odm_textured_model_geo.obj" 
        //          -logFile "d:/test_odm/odm_orthophoto/odm_orthophoto_log.txt" 
        //          -outputFile "d:/test_odm/odm_orthophoto/odm_orthophoto.tif" 
        //          -outputCornerFile "d:/test_odm/odm_orthophoto/odm_orthophoto_corners.txt"   
        //          -resolution 20.0
        //          -inpaintThreshold 1.0 
        //          -utm_north_offset 5157982.0 -utm_east_offset 322263.0 
        //          -a_srs "+proj=utm +zone=32 +datum=WGS84 +units=m +no_defs +type=crs"
        //          -co TILED=YES -co COMPRESS=DEFLATE -co PREDICTOR=2 -co BIGTIFF=IF_SAFER -co BLOCKXSIZE=512 -co BLOCKYSIZE=512 -co NUM_THREADS=16
        //          --config GDAL_CACHEMAX 10411063296.0 
        //          -verbose

        // Create georeferenced GeoTiff
        if (georef.is_valid)
        {
            XString bounds_file_path = XString::CombinePath(tree.odm_georef_path, "odm_georeferenced_model.bounds.gpkg");

            // Cutline computation, before cropping
            // We want to use the full orthophoto, not the cropped one.
            //if arg.orthophoto_cutline:
            {
                //    cutline_file = os.path.join(tree.odm_orthophoto, "cutline.gpkg")

            //    compute_cutline(tree.odm_orthophoto_tif,
            //                    bounds_file_path,
            //                    cutline_file,
            //                    arg.max_concurrency,
            //                    scale=0.25)

            //    orthophoto.compute_mask_raster(tree.odm_orthophoto_tif, cutline_file,
            //                            os.path.join(tree.odm_orthophoto, "odm_orthophoto_cut.tif"),
            //                            blend_distance=20, only_max_coords_feature=True)
            }

            //orthophoto.post_orthophoto_steps(args, bounds_file_path, tree.odm_orthophoto_tif, tree.orthophoto_tiles)

            //# Generate feathered orthophoto also
            //if arg.orthophoto_cutline:
            //    orthophoto.feather_raster(tree.odm_orthophoto_tif,
            //            os.path.join(tree.odm_orthophoto, "odm_orthophoto_feathered.tif"),
            //            blend_distance=20)
        }
        else
        {
            //if io.file_exists(tree.odm_orthophoto_render):
            //    pseudogeo.add_pseudo_georeferencing(tree.odm_orthophoto_render)
            //    log.ODM_INFO("Renaming %s --> %s" % (tree.odm_orthophoto_render, tree.odm_orthophoto_tif))
            //    os.replace(tree.odm_orthophoto_render, tree.odm_orthophoto_tif)
            //else:
            //    log.ODM_WARNING("Could not generate an orthophoto (it did not render)")
        }
    }
    else
    {
        Logger::Write(__FUNCTION__, "Found existing orthophoto: '%s'", tree.odm_orthophoto_tif.c_str());
    }

    //if io.file_exists(tree.odm_orthophoto_render):
    //    os.remove(tree.odm_orthophoto_render)

	BenchmarkStop("Generate orthophoto..");

	return status;
}
