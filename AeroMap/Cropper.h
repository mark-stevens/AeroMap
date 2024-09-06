#ifndef CROPPER_H
#define CROPPER_H

#include "AeroMap.h"

class Cropper
{
public:
	Cropper(XString storage_dir, XString files_prefix = "crop");
	
	XString path(XString suffix);

	XString create_bounds_geojson(XString pointcloud_path, double buffer_distance = 0, int decimation_step = 40);
	XString create_bounds_gpkg(XString pointcloud_path, double buffer_distance = 0, int decimation_step = 40);

	static XString crop(XString gpkg_path, XString geotiff_path, /*gdal_options,*/ bool keep_original = true/*, warp_options=[]*/);
	static void merge_bounds(XString input_bound_files, XString output_bounds, double buffer_distance = 0);

private:
	XString m_storage_dir;
	XString m_files_prefix;
};

#endif // #ifndef CROPPER_H
