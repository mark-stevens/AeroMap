// Mesh.cpp
// Port of odm mesh.py
//

#include "AeroLib.h"
#include "DEM.h"
#include "Mesh.h"

XString Mesh::screened_poisson_reconstruction(XString inPointCloud, XString outMesh, int depth, double samples_per_node,
	int maxVertexCount, double point_weight, int threads)
{
	// inPointCloud -> PoissonRecon -> outMeshDirty -> ReconMesh -> outMesh
	//

	XString mesh_path = outMesh.GetPathName();
	XString mesh_filename = outMesh.GetFileName();
	// mesh_path = path/to
	// mesh_filename = odm_mesh.ply

	XString basename;
	XString ext;
	int pos = mesh_filename.ReverseFind('.');
	if (pos > -1)
	{
		basename = mesh_filename.Left(pos);
		ext = mesh_filename.Mid(pos);
		// basename = odm_mesh
		// ext = .ply
	}

	XString outMeshDirty = XString::CombinePath(mesh_path, XString::Format("%s.dirty%s", basename.c_str(), ext.c_str()));
	if (QFile::exists(outMeshDirty.c_str()))
		QFile::remove(outMeshDirty.c_str());

	// Since PoissonRecon has some kind of a race condition on ppc64el, and this helps...
	//if platform.machine() == 'ppc64le':
	//    log.ODM_WARNING("ppc64le platform detected, forcing single-threaded operation for PoissonRecon")
	//    threads = 1

	while (true)
	{
		// Run PoissonRecon

		QStringList args;
		args.push_back("--in");
		args.push_back(inPointCloud.c_str());
		args.push_back("--out");
		args.push_back(outMeshDirty.c_str());
		args.push_back("--depth");
		args.push_back(XString::Format("%d", depth).c_str());
		args.push_back("--pointWeight");
		args.push_back(XString::Format("%0.1f", point_weight).c_str());
		args.push_back("--samplesPerNode");
		args.push_back(XString::Format("%0.1f", samples_per_node).c_str());
		args.push_back("--threads");
		args.push_back(XString::Format("%d", threads).c_str());
		args.push_back("--bType");									// boundary type: 1=free, 2=Dirichlet, 3=Neumann
		args.push_back("2");
		args.push_back("--linearFit");
		AeroLib::RunProgramEnv(tree.prog_poisson.c_str(), args);
		// cmd: PoissonRecon 
		//			--in "d:/test_odm/odm_filterpoints/point_cloud.ply" 
		//			--out "d:/test_odm/odm_meshing/odm_mesh.dirty.ply" 
		//			--depth 11 --pointWeight 4.0 --samplesPerNode 1.0 --threads 15 --bType 2 --linearFit 

		if (QFile::exists(outMeshDirty.c_str()))
		{
			break;	// Done!
		}
		else
		{
			// PoissonRecon will sometimes fail due to race conditions
			// on certain machines, especially on Windows
			threads /= 2;

			if (threads < 1)
			{
				Logger::Write(__FUNCTION__, "PoissonRecon failed, exiting.");
				break;
			}
			else
			{
				Logger::Write(__FUNCTION__, "PoissonRecon failed with %d threads, retrying with %d...", threads * 2, threads);
			}
		}
	}

	// Cleanup and reduce vertex count if necessary

	int max_faces = maxVertexCount * 2;

	QStringList args;
	args.push_back("-i");						// input file containing camera poses and image list
	args.push_back(outMeshDirty.c_str());
	args.push_back("-o");						// output mesh file
	args.push_back(outMesh.c_str());
	args.push_back("--archive-type");
	args.push_back("3");
	args.push_back("--remove-spikes");			// flag controlling the removal of spike faces
	args.push_back("0");
	args.push_back("--remove-spurious");		// spurious factor for removing faces with too long edges or isolated components
	args.push_back("20");
	args.push_back("--smooth");					// number of iterations to smooth the reconstructed surface
	args.push_back("0");
	args.push_back("--target-face-num");
	args.push_back(XString::Format("%d", max_faces).c_str());
	args.push_back("-v");
	args.push_back("0");
	AeroLib::RunProgramEnv(tree.prog_recon_mesh.c_str(), args);
	// cmd: ReconstructMesh 
	//			-i "d:/test_odm/odm_meshing/odm_mesh.dirty.ply" 
	//			-o "d:/test_odm/odm_meshing/odm_mesh.ply" 
	//			--archive-type 3 --remove-spikes 0 --remove-spurious 20 --smooth 0 --target-face-num 400000 -v 0

	// Delete intermediate results
	//os.remove(outMeshDirty)

	return outMesh;
}

XString Mesh::create_25dmesh(XString inPointCloud, XString outMesh, std::vector<double>radius_steps,
	double dsm_resolution, int depth, double samples_per_node, int maxVertexCount, int available_cores,
	XString method, bool smooth_dsm, int max_tiles)
{
	// Create DSM from point cloud
	//

	// Create temporary directory
	XString mesh_directory = outMesh.GetPathName();
	XString tmp_directory = XString::CombinePath(mesh_directory, "tmp");
	//if os.path.exists(tmp_directory):
	//    shutil.rmtree(tmp_directory)
	AeroLib::CreateFolder(tmp_directory);

	//log.ODM_INFO('Creating DSM for 2.5D mesh')

	DEM::create_dem(
		inPointCloud,			// input point cloud
		"mesh_dsm",				// dem type
		radius_steps,
		"max",					// output type
		true,					// gap fill 
		tmp_directory,			// out dir
		dsm_resolution,
		available_cores,
		4096,					// max tile size
		0,						// decimation
		false,					// keep unfilled copy
		smooth_dsm,				// apply smoothing
		max_tiles);

	XString mesh;
	if (method == "gridded")
	{
		mesh = dem_to_mesh_gridded(XString::CombinePath(tmp_directory, "mesh_dsm.tif"), outMesh, maxVertexCount, std::max(1, available_cores));
	}
	else if (method == "poisson")
	{
		XString dsm_points = dem_to_points(XString::CombinePath(tmp_directory, "mesh_dsm.tif"), XString::CombinePath(tmp_directory, "dsm_points.ply"));
		mesh = screened_poisson_reconstruction(
			dsm_points,
			outMesh,
			arg.mesh_oct_tree_depth,
			samples_per_node,
			maxVertexCount,
			4.0,
			std::max(1, available_cores - 1));	// poissonrecon can get stuck on some machines if --threads == all cores
	}
	else
	{
		Logger::Write(__FUNCTION__, "Invalid method: '%s'", method.c_str());
		assert(false);
	}

	// Cleanup tmp
	//if os.path.exists(tmp_directory):
	//    shutil.rmtree(tmp_directory)

	return mesh;
}

XString Mesh::dem_to_points(XString inGeotiff, XString outPointCloud)
{
	//log.ODM_INFO('Sampling points from DSM: %s' % inGeotiff)

	//kwargs = {
	//    'bin': context.dem2points_path,
	//    'outfile': outPointCloud,
	//    'infile': inGeotiff
	//}

	QStringList args;
	args.push_back("-inputFile");
	args.push_back(inGeotiff.c_str());
	args.push_back("-outputFile");
	args.push_back(outPointCloud.c_str());
	args.push_back("-skirtHeightThreshold");
	args.push_back("1.5");			// literal
	args.push_back("-skirtIncrements");
	args.push_back("0.2");
	args.push_back("-skirtHeightCap");
	args.push_back("100");
	args.push_back("-verbose");
	AeroLib::RunProgramEnv(tree.prog_dem2points.c_str(), args);

	//system.run('"{bin}" -inputFile "{infile}" '
	//        '-outputFile "{outfile}" '
	//        '-skirtHeightThreshold 1.5 '
	//        '-skirtIncrements 0.2 '
	//        '-skirtHeightCap 100 '
	//        '-verbose '.format(**kwargs))

	return outPointCloud;
}

XString Mesh::dem_to_mesh_gridded(XString inGeotiff, XString outMesh, int maxVertexCount, int maxConcurrency)
{
	// inGeoTiff -> dem2mesh -> outMeshDirty -> ReconstructMesh -> outMesh
	// 

	//log.ODM_INFO('Creating mesh from DSM: %s' % inGeotiff)

	XString mesh_path = outMesh.GetPathName();
	XString mesh_filename = outMesh.GetFileName();
	// mesh_path = path/to
	// mesh_filename = odm_mesh.ply

	XString basename;
	XString ext;
	int pos = mesh_filename.ReverseFind('.');
	if (pos > -1)
	{
		basename = mesh_filename.Left(pos);
		ext = mesh_filename.Mid(pos);
		// basename = odm_mesh
		// ext = .ply
	}

	XString outMeshDirty = XString::CombinePath(mesh_path, XString::Format("%s.dirty%s", basename.c_str(), ext.c_str()));

	// This should work without issues most of the time,
	// but just in case we lower maxConcurrency if it fails.
	while (true)
	{
		//try:
		//    kwargs = {
		//        'bin': context.dem2mesh_path,
		//        'outfile': outMeshDirty,
		//        'infile': inGeotiff,
		//        'maxVertexCount': maxVertexCount,
		//        'maxConcurrency': maxConcurrency
		//    }
		QStringList args;
		args.push_back("-inputFile");
		args.push_back(inGeotiff.c_str());
		args.push_back("-outputFile");
		args.push_back(outMeshDirty.c_str());
		args.push_back("-maxTileLength");
		args.push_back("2000");				// literal
		args.push_back("-maxVertexCount");
		args.push_back(XString::Format("%d", maxVertexCount).c_str());
		args.push_back("-maxConcurrency");
		args.push_back(XString::Format("%d", maxConcurrency).c_str());
		args.push_back("-edgeSwapThreshold");
		args.push_back("0.15");				// literal
		args.push_back("-verbose");
		AeroLib::RunProgramEnv(tree.prog_dem2mesh, args);
		// system.run('"{bin}" -inputFile "{infile}" '
		//        '-outputFile "{outfile}" '
		//        '-maxTileLength 2000 '
		//        '-maxVertexCount {maxVertexCount} '
		//        '-maxConcurrency {maxConcurrency} '
		//        '-edgeSwapThreshold 0.15 '
		//        '-verbose '.format(**kwargs))
		if (QFile::exists(outMeshDirty.c_str()))
		{
			break;		// done
		}
		else
		{
			maxConcurrency /= 2;
			if (maxConcurrency >= 1)
			{
				Logger::Write(__FUNCTION__, "dem2mesh failed, retrying with lower concurrency (%d) in case this is a memory issue", maxConcurrency);
			}
			else
			{
				Logger::Write(__FUNCTION__, "dem2mesh failed");
				assert(false);
			}
		}
	}

	// Cleanup and reduce vertex count if necessary
	// (as dem2mesh cannot guarantee that we'll have the target vertex count)
	//cleanupArgs = {
	//    'reconstructmesh': context.omvs_reconstructmesh_path,
	//    'outfile': outMesh,
	//    'infile': outMeshDirty,
	//    'max_faces': maxVertexCount * 2
	//}

	int max_faces = maxVertexCount * 2;

	QStringList args;
	args.push_back("-i");
	args.push_back(outMeshDirty.c_str());
	args.push_back("-o");
	args.push_back(outMesh.c_str());
	args.push_back("--archive-type");
	args.push_back("3");
	args.push_back("--remove-spikes");
	args.push_back("0");
	args.push_back("--remove-spurious");
	args.push_back("0");
	args.push_back("--smooth");
	args.push_back("0");
	args.push_back("--target-face-num");
	args.push_back(XString::Format("%d", max_faces).c_str());
	args.push_back("-v");
	args.push_back("0");
	AeroLib::RunProgramEnv(tree.prog_recon_mesh, args);
	//system.run('"{reconstructmesh}" -i "{infile}" '
	//        '-o "{outfile}" '
	//        '--archive-type 3 '
	//        '--remove-spikes 0 --remove-spurious 0 --smooth 0 '
	//        '--target-face-num {max_faces} -v 0'.format(**cleanupArgs))

	// Delete intermediate results
	//os.remove(outMeshDirty)

	return outMesh;
}
