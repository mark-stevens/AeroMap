#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "AeroMap.h"

class PointCloud
{
public:
	static void filter(XString input_point_cloud, XString output_point_cloud, XString output_stats,
		double standard_deviation = 2.5, double sample_radius = 0, /*boundary=None,*/ int max_concurrency = 1);

	static double get_spacing(XString stats_file, double resolution_fallback = 5.0);
	static RectD  get_extent(XString input_point_cloud);
};

#endif // #ifndef POINTCLOUD_H