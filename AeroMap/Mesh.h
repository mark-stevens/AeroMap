#ifndef MESH_H
#define MESH_H

#include "AeroMap.h"		// application header

class Mesh
{
public:
	static XString create_25dmesh(XString inPointCloud, XString outMesh, std::vector<double>radius_steps,
		double dsm_resolution = 0.05, int depth = 8, double samples_per_node = 1.0, int maxVertexCount = 100000, int available_cores = 0,
		XString method = "gridded", bool smooth_dsm = true, int max_tiles = 0);

	static XString screened_poisson_reconstruction(XString inPointCloud, XString outMesh, int depth = 8, double samples_per_node = 1.0,
		int maxVertexCount = 100000, double point_weight = 4.0, int threads = 0 /*context.num_cores*/);

	static XString dem_to_points(XString inGeotiff, XString outPointCloud);
	static XString dem_to_mesh_gridded(XString inGeotiff, XString outMesh, int maxVertexCount, int maxConcurrency);
};

#endif // #ifndef MESH_H
