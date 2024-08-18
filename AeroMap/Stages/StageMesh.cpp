// StageMesh.cpp
// Create 3D & 2.5D meshes.
//

#include "DEM.h"
#include "Mesh.h"
#include "StageMesh.h"

int StageMesh::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_meshing
	//			odm_mesh.ply
	//			odm_mesh.mvs
	//			odm_25dmesh.ply
	//			odm_25dmesh.mvs
	//

	int status = 0;

	BenchmarkStart();
	GetApp()->LogWrite("Create meshes...");

	AeroLib::CreateFolder(tree.odm_meshing);

	CreateMesh25D();
	CreateMesh3D();

	BenchmarkStop("Create meshes");

	return status;
}

int StageMesh::CreateMesh25D()
{
	int status = 0;

	// Always generate a 2.5D mesh
	// unless --use-3dmesh is set.
	if (arg.use_3dmesh)
		return 0;

	if ((QFile::exists(tree.odm_25dmesh.c_str()) == false) || Rerun())
	{
		//log.ODM_INFO('Writing ODM 2.5D Mesh file in: %s' % tree.odm_25dmesh)

		double multiplier = PI / 2.0;
		std::vector<double> radius_steps = DEM::get_dem_radius_steps(tree.filtered_point_cloud_stats, 3, arg.orthophoto_resolution, multiplier);
		double dsm_resolution = radius_steps[0] / multiplier;

		//log.ODM_INFO('ODM 2.5D DSM resolution: %s' % dsm_resolution)

		// emulate parameters
		int max_vertex = 200000;		// self.params.get(max_vertex)

		XString method = "gridded";
		if (arg.fast_orthophoto)
		{
			dsm_resolution *= 8.0;
			method = "poisson";
		}

		int max_tiles = GetProject().GetImageCount() / 2;

		Mesh::create_25dmesh(
			tree.filtered_point_cloud, 
			tree.odm_25dmesh, 
			radius_steps, 
			dsm_resolution, 
			arg.mesh_oct_tree_depth, 
			true, 
			max_vertex, 
			arg.max_concurrency, 
			method, 
			true, 
			max_tiles);
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing 2.5D mesh: '%s'", tree.odm_25dmesh.c_str());
	}

	return status;
}

int StageMesh::CreateMesh3D()
{
	int status = 0;

	// Create full 3D model unless --skip-3dmodel is set
	if (arg.skip_3dmodel)
		return 0;
	
	if ((QFile::exists(tree.odm_mesh.c_str()) == false) || Rerun())
	{
		//log.ODM_INFO('Writing ODM Mesh file in: %s' % tree.odm_mesh)

		// emulate parameters
		double samples = 1.0;			// self.params.get('samples')
		int max_vertex = 200000;		// self.params.get(max_vertex)
		double point_weight = 4.0;		// self.params.get(point_weight)

		Mesh::screened_poisson_reconstruction(
			tree.filtered_point_cloud,				// input point cloud
			tree.odm_mesh,							// output mesh
			arg.mesh_oct_tree_depth, 
			samples,								// samples per node
			max_vertex, 
			point_weight,
			std::max(1, arg.max_concurrency - 1));	// poissonrecon can get stuck on some machines if --threads == all cores
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing mesh: '%s'", tree.odm_mesh.c_str());
	}

	return status;
}
