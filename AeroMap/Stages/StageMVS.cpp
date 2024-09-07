// StageMVS.cpp
// Multi view stereo.
//

#include "StageMVS.h"

int StageMVS::Run()
{
	// Inputs:
	// Outputs:
    //      + opensfm/undistorted
    //          +openmvs
    //              scene.mvs                       <- exported from opensfm
    //              scene_dense.mvs                 <- DensifyPointCloud, pass 1
    //              scene_dense.ply
    //              Densify.ini
    //              scene_dense_dense_filtered.mvs  <- DensifyPointCloud, pass 2
    //              scene_dense_dense_filtered.ply
    //

	int status = 0;

	BenchmarkStart();
	GetApp()->LogWrite("OpenMVS...");

    // check if reconstruction was done before
    if ((QFile::exists(tree.openmvs_model.c_str()) == false) || Rerun())
    {
        //if self.rerun():
            //if io.dir_exists(tree.openmvs):
              //  shutil.rmtree(tree.openmvs)

        // export reconstruction from opensfm
        XString openmvs_scene_file = XString::CombinePath(tree.openmvs, "scene.mvs");
        if ((QFile::exists(openmvs_scene_file.c_str()) == false) || Rerun())
        {
            GetApp()->LogWrite("OpenMVS: Importing scene from OpenSFM...");
            QStringList args;
            args.push_back("export_openmvs");
            args.push_back(tree.opensfm.c_str());
            AeroLib::RunProgramEnv(tree.prog_opensfm.c_str(), args);
            // cmd: opensfm export_openmvs "d:/test_odm/opensfm"
        }
        else
        {
            Logger::Write(__FUNCTION__, "Found existing '%s'", openmvs_scene_file.c_str());
        }

        XString depthmaps_dir = XString::CombinePath(tree.openmvs, "depthmaps");
        //if io.dir_exists(depthmaps_dir) and self.rerun():
            //shutil.rmtree(depthmaps_dir)
        AeroLib::CreateFolder(depthmaps_dir);

        // Depthmap resolution set to: 500px
        
        XString densify_ini_file = XString::CombinePath(tree.openmvs, "Densify.ini");

        GetApp()->LogWrite("Densify pass 1/2...");
        QStringList args;
        args.push_back(openmvs_scene_file.c_str());
        args.push_back("-w");                       // working directory
        args.push_back(depthmaps_dir.c_str());
        args.push_back("--dense-config-file");
        args.push_back(densify_ini_file.c_str());
        args.push_back("--resolution-level");       // how many times to scale down the images before point cloud computation
        args.push_back("3");
        args.push_back("--max-resolution");
        args.push_back(XString::Format("%d", GetProject().get_undistorted_image_max_size()).c_str());
        args.push_back("--max-threads");
        args.push_back(XString::Format("%d", arg.max_concurrency).c_str());
        args.push_back("--number-views-fuse");      //  minimum number of images that agrees with an estimate during fusion in order to consider it inlier (<2=only merge depth maps)
        args.push_back("2");
        args.push_back("--sub-resolution-levels");  //  number of patch-match sub-resolution iterations (0=disabled)
        args.push_back("2");
        args.push_back("--archive-type");           // project archive type: -1=interface, 0=text, 1=binary, 2=compressed binary
        args.push_back("3");                        // 3 = ???
        args.push_back("-v");
        args.push_back("0");
        args.push_back("--cuda-device");
        args.push_back("-1");
        AeroLib::RunProgramEnv(tree.prog_densify.c_str(), args);
        // cmd: DensifyPointCloud 
        //          "d:/test_odm/opensfm/undistorted/openmvs/scene.mvs"  
        //          -w "d:/test_odm/opensfm/undistorted/openmvs/depthmaps" 
        //          --dense-config-file "d:/test_odm/opensfm/undistorted/openmvs/Densify.ini" 
        //          --resolution-level 3
        //          --max-resolution 4000 --max-threads 16 --number-views-fuse 2 --sub-resolution-levels 2 
        //          --archive-type 3 
        //          -v 0 --cuda-device -1

        XString scene_dense = XString::CombinePath(tree.openmvs, "scene_dense.mvs");

        GetApp()->LogWrite("Densify pass 2/2...");
        args.clear();
        args.push_back("--filter-point-cloud");
        args.push_back("-20");
        args.push_back("-i");       // input filename containing camera poses and image list
        args.push_back(scene_dense.c_str());
        args.push_back("-v");
        args.push_back("0");
        args.push_back("--cuda-device");
        args.push_back("-1");
        AeroLib::RunProgramEnv(tree.prog_densify.c_str(), args);
        // cmd: DensifyPointCloud 
        //          --filter-point-cloud -20
        //          -i "d:/test_odm/opensfm/undistorted/openmvs/scene_dense.mvs" 
        //          -v 0 --cuda-device -1
    }
    else
    {
        Logger::Write(__FUNCTION__, "Found OpenMVS reconstruction file: '%s'", tree.openmvs_model.c_str());
    }

	BenchmarkStop("OpenMVS");

	return status;
}