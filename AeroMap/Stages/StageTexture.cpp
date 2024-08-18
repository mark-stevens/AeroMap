// StageTexture.cpp
// Apply textures.
//

#include "StageTexture.h"

int StageTexture::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_texturing
	//			odm_textured_model_geo.conf
	//			odm_textured_model_geo.mtl
	//			odm_textured_model_geo.obj
	//		+ odm_texturing_25d
	//			odm_textured_model_geo.conf
	//			odm_textured_model_geo.mtl
	//			odm_textured_model_geo.obj
	//

	int status = 0;

	BenchmarkStart();
	GetApp()->LogWrite("Apply textures...");

	//max_dim = find_largest_photo_dim(reconstruction.photos)
	int	max_texture_size = 8 * 1024;	// default

	//if max_dim > 8000:
		//log.ODM_INFO("Large input images (%s pixels), increasing maximum texture size." % max_dim)
		//max_texture_size *= 3
		
	// if input mesh file exists
	if (QFile::exists(tree.odm_mesh.c_str()))
	{
		AeroLib::CreateFolder(tree.odm_texturing);

		XString odm_textured_model_obj = XString::CombinePath(tree.odm_texturing, tree.odm_textured_model_obj);
		if ((QFile::exists(odm_textured_model_obj.c_str()) == false) || Rerun())
		{
			// Writing MVS Textured file in: d:/test_odm/odm_texturing/odm_textured_model_geo.obj

			XString out_prefix = odm_textured_model_obj.Left(odm_textured_model_obj.GetLength() - 4);

			QStringList args;
			args.push_back(tree.opensfm_reconstruction_nvm.c_str());	// IN_SCENE
			args.push_back(tree.odm_mesh.c_str());						// IN_MESH
			args.push_back(out_prefix.c_str());							// OUT_PREFIX
			args.push_back("-d");						// Data term: {area, gmi}
			args.push_back("gmi");
			args.push_back("-o");						// Photometric outlier (pedestrians etc.) removal method: {none, gauss_damping, gauss_clamping}
			args.push_back("gauss_clamping");
			args.push_back("-t");						// Tone mapping method: {none, gamma}
			args.push_back("none");
			args.push_back("--no_intermediate_results");
			args.push_back(XString::Format("--max_texture_size=%d", max_texture_size).c_str());
			AeroLib::RunProgramEnv(tree.prog_recon_tex, args);
			// cmd: texrecon 
			//			"d:/test_odm/opensfm/undistorted/reconstruction.nvm"	
			//			"d:/test_odm/odm_meshing/odm_mesh.ply" 
			//			"d:/test_odm/odm_texturing/odm_textured_model_geo" 
			//			-d gmi -o gauss_clamping -t none --no_intermediate_results --max_texture_size=8192
		}
	}

	// if input mesh file exists
	if (QFile::exists(tree.odm_25dmesh.c_str()))
	{
		AeroLib::CreateFolder(tree.odm_25dtexturing);

		XString odm_textured_model_obj = XString::CombinePath(tree.odm_25dtexturing, tree.odm_textured_model_obj);
		if ((QFile::exists(odm_textured_model_obj.c_str()) == false) || Rerun())
		{
			// Writing MVS Textured file in: d:/test_odm/odm_texturing_25d/odm_textured_model_geo.obj

			XString out_prefix = odm_textured_model_obj.Left(odm_textured_model_obj.GetLength() - 4);

			QStringList args;
			args.push_back(tree.opensfm_reconstruction_nvm.c_str());	// IN_SCENE
			args.push_back(tree.odm_25dmesh.c_str());					// IN_MESH
			args.push_back(out_prefix.c_str());							// OUT_PREFIX
			args.push_back("-d");
			args.push_back("gmi");
			args.push_back("-o");
			args.push_back("gauss_clamping");
			args.push_back("-t");
			args.push_back("none");
			args.push_back("--nadir_mode");
			args.push_back("--no_intermediate_results");
			args.push_back(XString::Format("--max_texture_size=%d", max_texture_size).c_str());
			AeroLib::RunProgramEnv(tree.prog_recon_tex, args);
			// cmd: texrecon 
			//			"d:/test_odm/opensfm/undistorted/reconstruction.nvm" 
			//			"d:/test_odm/odm_meshing/odm_25dmesh.ply" 
			//			"d:/test_odm/odm_texturing_25d/odm_textured_model_geo" 
			//			-d gmi -o gauss_clamping -t none --no_intermediate_results --nadir_mode --max_texture_size=8192
		}
	}

	BenchmarkStop("Apply textures");

	return status;
}
