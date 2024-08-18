// Project.cpp
// Manager for AeroMap project files.
//
// Sample File (tab-indented):
//
//		project
//			name = Sample Project
//			srs = UTM_10
//		drone
//			input = location of drone photogrammetry inputs
//			output = root folder that will receive outputs
//		lidar
//			src = D:/Geodata/lidar/test-data/points-1.4_6.las
//			src = D:/Geodata/lidar/test-data/points-1.4_7.las
//

#include <stdio.h>
#include <assert.h>

#include "AeroMap.h"
#include "AeroLib.h"
#include "TextFile.h"		// manage small text files
#include "Logger.h"
#include "RasterFile.h"
#include "Project.h"

#define DEFAULT_EXT "aero"

ArgType arg;
TreeType tree;

enum class Section
{
	None = 0,
	Project,		// main project section
	Drone,			// drone photogrammetry
	Lidar			// lidar files
};

Project::Project()
	: mp_SRS(nullptr)
	, mb_IsDirty(false)
{
	mv_Lidar.reserve(32);
}

Project::~Project()
{
	FreeResources();
}

int Project::Load(const char* fileName)
{
	// Load a project file.
	//
	// Inputs:
	//		fileName = full path/file name
	// Outputs:
	//		return = # of errors encountered
	//				 if nonzero, can query errors individually
	//

	// Project file are hierachical, tab-delimited text files.
	
	FreeResources();

	m_ErrorCtr = 0;
	Section sect = Section::None;

	FILE* pFile = fopen(fileName, "rt");
	if (pFile != nullptr)
	{
		TextFile textFile(fileName);
		for (int i = 0; i < (int)textFile.GetLineCount(); ++i)
		{
			XString strLine = textFile.GetLine(i).c_str();
			int indent = GetIndentLevel(strLine);

			strLine.Trim("\n\r\t");

			if (indent == 0 && strLine.CompareNoCase("project"))
			{
				sect = Section::Project;
			}
			else if (indent == 0 && strLine.CompareNoCase("drone"))
			{
				sect = Section::Drone;
			}
			else if (indent == 0 && strLine.CompareNoCase("lidar"))
			{
				sect = Section::Lidar;
			}
			else if (sect != Section::None)
			{
				int tokenCount = strLine.Tokenize("=");
				switch (sect)
				{
				case Section::Project:
					if (strLine.BeginsWith("name"))
					{
						if (tokenCount > 1)
							ms_ProjectName = strLine.GetToken(1);
						ms_ProjectName.Trim();
					}
					break;
				case Section::Drone:
					if ((indent == 1) && (tokenCount == 2))
					{
						if (strLine.BeginsWith("input"))
							SetDroneInputPath(strLine.GetToken(1));
						else if (strLine.BeginsWith("output"))
							SetDroneOutputPath(strLine.GetToken(1));
					}
					break;
				case Section::Lidar:
					if ((indent == 1) && (tokenCount == 2))
					{
						if (strLine.BeginsWith("src"))
							AddLidarFileToList(strLine.GetToken(1).c_str());
					}
					break;
				}
			}
		}
	}
	else
	{
		// add "file not found" to error list
		++m_ErrorCtr;
	}

	LoadData();

	InitArg();
	InitTree();

	ms_FileName = fileName;
	mb_IsDirty = false;

	emit projectChanged_sig();

	return m_ErrorCtr;
}

void Project::LoadData()
{
	// Load project data.
	//

	GDALAllRegister();

	for (int i = 0; i < (int)mv_Lidar.size(); ++i)
	{
		if (mv_Lidar[i].exists)
			mv_Lidar[i].pFile = new LasFile(mv_Lidar[i].src.c_str());
	}

	emit projectChanged_sig();
}

int Project::Save(const char* fileName)
{
	// Save a project file.
	//
	// Inputs:
	//		fileName = full path/file name
	// Outputs:
	//		return = # of errors encountered
	//

	m_ErrorCtr = 0;

	XString strFileName;
	if (fileName == nullptr)
		strFileName = ms_FileName;
	else
		strFileName = fileName;

	FILE* pFile = fopen(strFileName.c_str(), "wt");
	if (pFile != nullptr)
	{
		// write project section
		fprintf(pFile, "project\n");
		fprintf(pFile, "\tname=%s\n", ms_ProjectName.c_str());
		fprintf(pFile, "\tsrs=%s\n", "---");

		// drone section
		fprintf(pFile, "drone\n");
		fprintf(pFile, "\tinput = %s\n", ms_DroneInputPath.c_str());
		fprintf(pFile, "\toutput = %s\n", ms_DroneOutputPath.c_str());

		// lidar section
		fprintf(pFile, "lidar\n");
		for (int i = 0; i < mv_Lidar.size(); ++i)
		{
			fprintf(pFile, "\tsrc = %s\n", mv_Lidar[i].src.c_str());
		}
		
		fclose(pFile);
	}
	else
	{
		// add "unable to create file" to error list
		++m_ErrorCtr;
	}

	ms_FileName = strFileName;
	mb_IsDirty = false;

	return m_ErrorCtr;
}

void Project::Clear()
{
	// Reset project.
	//

	FreeResources();

	ms_FileName.Clear();
	ms_ProjectName.Clear();

	mb_IsDirty = false;

	emit projectChanged_sig();
}

bool Project::IsDirty()
{
	return mb_IsDirty;
}

int Project::LoadImageList()
{
	QDir dir(GetProject().GetDroneInputPath().c_str());
	dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i)
	{
		QFileInfo fileInfo = list.at(i);

		XString file_name = fileInfo.fileName().toLatin1().constData();
		XString file_path = fileInfo.absoluteFilePath().toLatin1().constData();

		if (fileInfo.isFile())
		{
			if ((file_name.EndsWithNoCase(".jpg") == false) && (file_name.EndsWithNoCase(".jpeg") == false))
				continue;

			ImageType entry;
			entry.file_name = file_path;

			// Read the JPEG file into a buffer
			FILE* fp = fopen(file_path.c_str(), "rb");
			if (!fp)
			{
				Logger::Write(__FUNCTION__, "Unable to open image file: '%s'", file_path.c_str());
				return -1;
			}
			fseek(fp, 0, SEEK_END);
			unsigned long fsize = ftell(fp);
			rewind(fp);
			unsigned char* buf = new unsigned char[fsize];
			if (fread(buf, 1, fsize, fp) != fsize)
			{
				Logger::Write(__FUNCTION__, "Unable to open read file: '%s'", file_path.c_str());
				delete[] buf;
				return -2;
			}
			fclose(fp);

			// Parse EXIF
			int code = entry.exif.parseFrom(buf, fsize);
			delete[] buf;
			if (code)
			{
				Logger::Write(__FUNCTION__, "Error parsing EXIF: '%s' (code %d)", file_path.c_str(), code);
				return -3;
			}

			// pre-computed values

			entry.focal_ratio = AeroLib::CalcFocalRatio(entry.exif);
			entry.epoch = AeroLib::CalcUnixEpoch(entry.exif.DateTime.c_str());
			entry.camera_str_osfm = AeroLib::GetCameraString(entry.exif, true);
			entry.camera_str_odm = AeroLib::GetCameraString(entry.exif, false);

			m_ImageList.push_back(entry);
		}
	}

	return GetImageCount();
}

int Project::GetImageCount()
{
	return (int)m_ImageList.size();
}

const std::vector<Project::ImageType>& Project::GetImageList()
{
	// Return a const reference to the image list.
	//

	return m_ImageList;
}

XString Project::GetFileName()
{
	return ms_FileName;
}

XString Project::Project::GetName()
{
	return ms_ProjectName;
}

const char* Project::GetDefaultExt()
{
	// Return default extension for AeroMap projects.

	return DEFAULT_EXT;
}

OGRSpatialReference* Project::GetSRS()
{
	return mp_SRS;
}

int Project::GetIndentLevel(XString& str)
{
	// Return # of leading tabs in str.

	int level = 0;
	for (int i = 0; i < str.GetLength(); ++i)
	{
		if (str[i] == '\t')
			++level;
		else
			break;
	}
	return level;
}

void Project::FreeResources()
{
	// Free project resources.
	//

	if (mp_SRS)
	{
//TODO:
//like this?
		//DestroySpatialReference
		OGRFree(mp_SRS);
		//delete mp_SRS;
		mp_SRS = nullptr;
	}

	// free lidar resources

	for (auto lidar : mv_Lidar)
	{
		if (lidar.pFile)
		{
			delete lidar.pFile;
			lidar.pFile = nullptr;
		}
	}
	mv_Lidar.clear();
}

int Project::GetLidarFileCount()
{
	return static_cast<int>(mv_Lidar.size());
}

void Project::AddLidarFile(const char* pathName)
{
	if (pathName == nullptr)
		return;

	AddLidarFileToList(pathName);

	mb_IsDirty = true;

	emit projectChanged_sig();
}

void Project::AddLidarFileToList(const char* pathName)
{
	if (pathName == nullptr)
		return;

	LidarType lidar;
	lidar.src = pathName;
	lidar.src.Trim();
	lidar.src.NormalizePath();
	lidar.exists = QFile::exists(pathName);
	if (lidar.exists)
	{
		lidar.pFile = new LasFile(pathName);
		lidar.type = lidar.pFile->GetFileType();
	}
	mv_Lidar.push_back(lidar);
}

void Project::RemoveLidarFile(int index)
{
	// Remove lidar file from project.
	//
	// Inputs:
	//		index = index of file to remove
	//

	if (index >= (int)mv_Lidar.size())
		return;

	mv_Lidar.erase(mv_Lidar.cbegin() + index);

	mb_IsDirty = true;

	emit projectChanged_sig();
}

XString Project::GetLidarFileName(int index)
{
	if (index >= 0 && index < GetLidarFileCount())
		return mv_Lidar[index].src;

	return "";
}

LasFile* Project::GetLasFile(int index)
{
	if (index >= 0 && index < GetLidarFileCount())
		return mv_Lidar[index].pFile;

	return nullptr;
}

bool Project::GetLidarExists(int index)
{
	if (index >= 0 && index < GetLidarFileCount())
		return mv_Lidar[index].exists;

	return false;
}

XString Project::GetDroneInputPath()
{
	return ms_DroneInputPath;
}

void Project::SetDroneInputPath(XString path)
{
	ms_DroneInputPath = path;
	ms_DroneInputPath.Trim();
}

XString Project::GetDroneOutputPath()
{
	return ms_DroneOutputPath;
}

void Project::SetDroneOutputPath(XString path)
{
	ms_DroneOutputPath = path;
	ms_DroneOutputPath.Trim();
}

XString Project::GetCoordsFileName()
{
	return XString::CombinePath(tree.odm_georef_path, "coords.txt");
}

void Project::Update()
{
	emit projectChanged_sig();
}

void Project::InitArg()
{
	arg.dtm = false;
	// --dtm		Use this tag to build a DTM (Digital Terrain Model, ground only) using a simple morphological
	//				filter. Check the --dem* and --smrf* parameters for finer tuning. Default: False
	arg.dsm = false;
	// --dsm        Use this tag to build a DSM (Digital Surface Model, ground + objects) using a progressive
	//				morphological filter. Check the --dem* parameters for finer tuning. Default: False

	arg.dem_gapfill_steps = 3;
	// --dem-gapfill-steps <positive integer>
	//				Number of steps used to fill areas with gaps. Set to 0 to disable gap filling. Starting with a
	//				radius equal to the output resolution, N different DEMs are generated with progressively
	//				bigger radius using the inverse distance weighted (IDW) algorithm and merged together.
	//				Remaining gaps are then merged using nearest neighbor interpolation. Default: 3
	arg.dem_resolution = 5.0;
	// --dem-resolution <float>
	//				DSM/DTM resolution in cm / pixel. Note that this value is capped by a ground sampling distance
	//				(GSD) estimate. Default: 5
	arg.dem_decimation = 1;
	// --dem-decimation <positive integer>
	//				Decimate the points before generating the DEM. 1 is no decimation (full quality). 100
	//				decimates ~99% of the points. Useful for speeding up generation of DEM results in very large
	//				datasets. Default: 1
	arg.dem_euclidean_map = false;
	// --dem-euclidean-map   Computes an euclidean raster map for each DEM. The map reports the distance from each cell to
	//				the nearest NODATA value (before any hole filling takes place). This can be useful to isolate
	//				the areas that have been filled. Default: False
	arg.orthophoto_resolution = 5.0;
	// --orthophoto-resolution <float > 0.0>
	//				Orthophoto resolution in cm / pixel. Note that this value is capped by a ground sampling
	//				distance (GSD) estimate.Default: 5
	arg.crop = 3.0;
	// --crop <positive float>
	//				Automatically crop image outputs by creating a smooth buffer around the dataset boundaries,
	//				shrunk by N meters. Use 0 to disable cropping. Default: 3

	arg.ignore_gsd = false;
	// --ignore-gsd          Ignore Ground Sampling Distance (GSD).A memory and processor hungry change relative to the
	//				default behavior if set to true. Ordinarily, GSD estimates are used to cap the maximum
	//				resolution of image outputs and resizes images when necessary, resulting in faster processing
	//				and lower memory usage. Since GSD is an estimate, sometimes ignoring it can result in slightly
	//				better image output quality. Never set --ignore-gsd to true unless you are positive you need
	//				it, and even then: do not use it. Default: False

	arg.pc_classify = false;
	// --pc-classify         Classify the point cloud outputs. You can control the behavior of this option by tweaking the
	//				--dem-* parameters. Default: False
	arg.pc_rectify = false;
		// --pc-rectify          Perform ground rectification on the point cloud. This means that wrongly classified ground
	//				points will be re-classified and gaps will be filled. Useful for generating DTMs. Default:
	//				False

	arg.pc_filter = 2.5;
	// --pc-filter <positive float>
	//				Filters the point cloud by removing points that deviate more than N standard deviations from
	//				the local mean. Set to 0 to disable filtering. Default: 2.5
	arg.pc_sample = 0.0;
	// --pc-sample <positive float>
	//				Filters the point cloud by keeping only a single point around a radius N (in meters). This can
	//				be useful to limit the output resolution of the point cloud and remove duplicate points. Set
	//				to 0 to disable sampling. Default: 0

	arg.use_3dmesh = false;
	// --use-3dmesh          Use a full 3D mesh to compute the orthophoto instead of a 2.5D mesh. This option is a bit
	//				faster and provides similar results in planar areas. Default: False
	arg.skip_3dmodel = false;
	// --skip-3dmodel        Skip generation of a full 3D model. This can save time if you only need 2D results such as
	//				orthophotos and DEMs. Default: False
	arg.skip_report = false;
	// --skip-report         Skip generation of PDF report. This can save time if you don't need a report. Default: False
	arg.skip_orthophoto = false;
	// --skip-orthophoto     Skip generation of the orthophoto. This can save time if you only need 3D results or DEMs.
	//				Default: False

	arg.mesh_oct_tree_depth = 11;
	// --mesh-octree-depth <integer: 1 <= x <= 14>
	//				Octree depth used in the mesh reconstruction, increase to get more vertices, recommended
	//				values are 8-12. Default: 11
	arg.fast_orthophoto = false;
	// --fast-orthophoto     Skips dense reconstruction and 3D model generation. It generates an orthophoto directly from
	//				the sparse reconstruction. If you just need an orthophoto and do not need a full 3D model,
	//				turn on this option. Default: False

	arg.smrf_scalar = 1.25;
	// --smrf-scalar <positive float>
	//				Simple Morphological Filter elevation scalar parameter. Default: 1.25
	arg.smrf_slope = 0.15;
	// --smrf-slope <positive float>
	//				Simple Morphological Filter slope parameter (rise over run). Default: 0.15
	arg.smrf_threshold = 0.5;
	// --smrf-threshold <positive float>
	//				Simple Morphological Filter elevation threshold parameter (meters). Default: 0.5
	arg.smrf_window = 18.0;
	// --smrf-window <positive float>
	//				Simple Morphological Filter window radius parameter (meters). Default: 18.0

	//TODO:
	//core count?
	arg.max_concurrency = 16;
	// --max-concurrency <positive integer>
	//				The maximum number of processes to use in various processes. Peak memory requirement is ~1GB
	//				per thread and 2 megapixel image resolution. Default: 16

	// --min-num-features <integer>
	//				Minimum number of features to extract per image. More features can be useful for finding more
	//				matches between images, potentially allowing the reconstruction of areas with little overlap
	//				or insufficient features. More features also slow down processing. Default: 10000
	// --feature-type <string>
	//				Choose the algorithm for extracting keypoints and computing descriptors. Can be one of: akaze,
	//				hahog, orb, sift. Default: sift
	// --feature-quality <string>
	//				Set feature extraction quality. Higher quality generates better features, but requires more
	//				memory and takes longer. Can be one of: ultra, high, medium, low, lowest. Default: high
	// --matcher-type <string>
	//				Matcher algorithm, Fast Library for Approximate Nearest Neighbors or Bag of Words. FLANN is
	//				slower, but more stable. BOW is faster, but can sometimes miss valid matches. BRUTEFORCE is
	//				very slow but robust.Can be one of: bow, bruteforce, flann. Default: flann
	// --matcher-neighbors <positive integer>
	//				Perform image matching with the nearest images based on GPS exif data. Set to 0 to match by
	//				triangulation. Default: 0
	// --matcher-order <positive integer>
	//				Perform image matching with the nearest N images based on image filename order. Can speed up
	//				processing of sequential images, such as those extracted from video. It is applied only on
	//				non-georeferenced datasets. Set to 0 to disable. Default: 0
	// --use-fixed-camera-params
	//				Turn off camera parameter optimization during bundle adjustment. This can be sometimes useful
	//				for improving results that exhibit doming/bowling or when images are taken with a rolling
	//				shutter camera. Default: False
	// --cameras <json>      Use the camera parameters computed from another dataset instead of calculating them. Can be
	//				specified either as path to a cameras.json file or as a JSON string representing the contents
	//				of a cameras.json file. Default:
	// --camera-lens <string>
	//				Set a camera projection type. Manually setting a value can help improve geometric
	//				undistortion. By default the application tries to determine a lens type from the images
	//				metadata. Can be one of: auto, perspective, brown, fisheye, fisheye_opencv, spherical,
	//				equirectangular, dual. Default: auto
	// --radiometric-calibration <string>
	//				Set the radiometric calibration to perform on images. When processing multispectral and
	//				thermal images you should set this option to obtain reflectance/temperature values (otherwise
	//				you will get digital number values). [camera] applies black level, vignetting, row gradient
	//				gain/exposure compensation (if appropriate EXIF tags are found) and computes absolute
	//				temperature values. [camera+sun] is experimental, applies all the corrections of [camera],
	//				plus compensates for spectral radiance registered via a downwelling light sensor (DLS) taking
	//				in consideration the angle of the sun. Can be one of: none, camera, camera+sun. Default: none
	// --use-hybrid-bundle-adjustment
	//				Run local bundle adjustment for every image added to the reconstruction and a global
	//				adjustment every 100 images. Speeds up reconstruction for very large datasets. Default: False
	// --sfm-algorithm <string>
	//				Choose the structure from motion algorithm. For aerial datasets, if camera GPS positions and
	//				angles are available, triangulation can generate better results. For planar scenes captured at
	//				fixed altitude with nadir-only images, planar can be much faster. Can be one of: incremental,
	//				triangulation, planar. Default: incremental
	// --sfm-no-partial      Do not attempt to merge partial reconstructions. This can happen when images do not have
	//				sufficient overlap or are isolated. Default: False
	// --sky-removal         Automatically compute image masks using AI to remove the sky. Experimental. Default: False
	// --bg-removal          Automatically compute image masks using AI to remove the background. Experimental. Default:
	//				False
	// --no-gpu              Do not use GPU acceleration, even if it's available. Default: False
	// --mesh-size <positive integer>
	//				The maximum vertex count of the output mesh. Default: 200000
	// --boundary <json>     GeoJSON polygon limiting the area of the reconstruction. Can be specified either as path to a
	//				GeoJSON file or as a JSON string representing the contents of a GeoJSON file. Default:
	// --auto-boundary       Automatically set a boundary using camera shot locations to limit the area of the
	//				reconstruction. This can help remove far away background artifacts (sky, background
	//				landscapes, etc.). See also --boundary. Default: False
	// --auto-boundary-distance <positive float>
	//				Specify the distance between camera shot locations and the outer edge of the boundary when
	//				computing the boundary with --auto-boundary. Set to 0 to automatically choose a value.
	//				Default: 0
	// --pc-quality <string>
	//				Set point cloud quality. Higher quality generates better, denser point clouds, but requires
	//				more memory and takes longer. Each step up in quality increases processing time roughly by a
	//				factor of 4x.Can be one of: ultra, high, medium, low, lowest. Default: medium
	// --pc-csv              Export the georeferenced point cloud in CSV format. Default: False
	// --pc-las              Export the georeferenced point cloud in LAS format. Default: False
	// --pc-ept              Export the georeferenced point cloud in Entwine Point Tile (EPT) format. Default: False
	// --pc-copc             Save the georeferenced point cloud in Cloud Optimized Point Cloud (COPC) format. Default:
	//				False
	// --pc-skip-geometric   Geometric estimates improve the accuracy of the point cloud by computing geometrically
	//				consistent depthmaps but may not be usable in larger datasets. This flag disables geometric
	//				estimates. Default: False
	// --texturing-skip-global-seam-leveling
	//				Skip normalization of colors across all images. Useful when processing radiometric data.
	//				Default: False
	// --texturing-skip-local-seam-leveling
	//				Skip the blending of colors near seams. Default: False
	// --texturing-keep-unseen-faces
	//				Keep faces in the mesh that are not seen in any camera. Default: False
	// --texturing-single-material
	//				Generate OBJs that have a single material and a single texture file instead of multiple ones.
	//				Default: False
	// --gltf                Generate single file Binary glTF (GLB) textured models. Default: False
	// --gcp <path string>   Path to the file containing the ground control points used for georeferencing. The file needs
	//				to use the following format: EPSG:<code> or <+proj definition> geo_x geo_y geo_z im_x im_y
	//				image_name [gcp_name] [extra1] [extra2] Default: None
	// --geo <path string>   Path to the image geolocation file containing the camera center coordinates used for
	//				georeferencing. If you don't have values for yaw/pitch/roll you can set them to 0. The file
	//				needs to use the following format: EPSG:<code> or <+proj definition> image_name geo_x geo_y
	//				geo_z [yaw (degrees)] [pitch (degrees)] [roll (degrees)] [horz accuracy (meters)] [vert
	//				accuracy (meters)] Default: None
	// --align <path string>
	//				Path to a GeoTIFF DEM or a LAS/LAZ point cloud that the reconstruction outputs should be
	//				automatically aligned to. Experimental. Default: None
	// --use-exif            Use this tag if you have a GCP File but want to use the EXIF information for georeferencing
	//				instead. Default: False
	// --orthophoto-no-tiled
	//				Set this parameter if you want a striped GeoTIFF. Default: False
	// --orthophoto-png      Set this parameter if you want to generate a PNG rendering of the orthophoto. Default: False
	// --orthophoto-kmz      Set this parameter if you want to generate a Google Earth (KMZ) rendering of the orthophoto.
	//				Default: False
	// --orthophoto-compression <string>
	//				Set the compression to use for orthophotos. Can be one of: JPEG, LZW, PACKBITS, DEFLATE, LZMA,
	//				NONE. Default: DEFLATE
	// --orthophoto-cutline  Generates a polygon around the cropping area that cuts the orthophoto around the edges of
	//				features. This polygon can be useful for stitching seamless mosaics with multiple overlapping
	//				orthophotos. Default: False
	// --tiles               Generate static tiles for orthophotos and DEMs that are suitable for viewers like Leaflet or
	//				OpenLayers. Default: False
	// --3d-tiles            Generate OGC 3D Tiles outputs. Default: False
	// --rolling-shutter     Turn on rolling shutter correction. If the camera has a rolling shutter and the images were
	//				taken in motion, you can turn on this option to improve the accuracy of the results. See also
	//				--rolling-shutter-readout. Default: False
	// --rolling-shutter-readout <positive integer>
	//				Override the rolling shutter readout time for your camera sensor (in milliseconds), instead of
	//				using the rolling shutter readout database. Note that not all cameras are present in the
	//				database. Set to 0 to use the database value. Default: 0
	// --build-overviews     Build orthophoto overviews for faster display in programs such as QGIS. Default: False
	// --cog                 Create Cloud-Optimized GeoTIFFs instead of normal GeoTIFFs. Default: False
	// --copy-to <path>      Copy output results to this folder after processing.
	// --version             Displays version number and exits.
	// --video-limit <positive integer>
	//				Maximum number of frames to extract from video files for processing. Set to 0 for no limit.
	//				Default: 500
	// --video-resolution <positive integer>
	//				The maximum output resolution of extracted video frames in pixels. Default: 4000
	// --split <positive integer>
	//				Average number of images per submodel. When splitting a large dataset into smaller submodels,
	//				images are grouped into clusters. This value regulates the number of images that each cluster
	//				should have on average. Default: 999999
	// --split-overlap <positive integer>
	//				Radius of the overlap between submodels. After grouping images into clusters, images that are
	//				closer than this radius to a cluster are added to the cluster. This is done to ensure that
	//				neighboring submodels overlap. Default: 150
	// --split-image-groups <path string>
	//				Path to the image groups file that controls how images should be split into groups. The file
	//				needs to use the following format: image_name group_name Default: None
	// --sm-no-align         Skip alignment of submodels in split-merge. Useful if GPS is good enough on very large
	//				datasets. Default: False
	// --sm-cluster <string>
	//				URL to a ClusterODM instance for distributing a split-merge workflow on multiple nodes in
	//				parallel. Default: None
	// --merge <string>      Choose what to merge in the merge step in a split dataset. By default all available outputs
	//				are merged. Options: all, pointcloud, orthophoto, dem. Default: all
	// --force-gps           Use images' GPS exif data for reconstruction, even if there are GCPs present.This flag is
	//				useful if you have high precision GPS measurements. If there are no GCPs, this flag does
	//				nothing. Default: False
	// --gps-accuracy <positive float>
	//				Set a value in meters for the GPS Dilution of Precision (DOP) information for all images. If
	//				your images are tagged with high precision GPS information (RTK), this value will be
	//				automatically set accordingly. You can use this option to manually set it in case the
	//				reconstruction fails. Lowering this option can sometimes help control bowling-effects over
	//				large areas. Default: 10
	// --optimize-disk-space
	//				Delete heavy intermediate files to optimize disk space usage. This affects the ability to
	//				restart the pipeline from an intermediate stage, but allows datasets to be processed on
	//				machines that don't have sufficient disk space available. Default: False
	// --primary-band <string>
	//				When processing multispectral datasets, you can specify the name of the primary band that will
	//				be used for reconstruction. It's recommended to choose a band which has sharp details and is
	//				in focus. Default: auto
	// --skip-band-alignment
	//				When processing multispectral datasets, ODM will automatically align the images for each band.
	//				If the images have been postprocessed and are already aligned, use this option. Default: False

	// process parameters to ensure consistency

	if (arg.fast_orthophoto)
	{
		Logger::Write(__FUNCTION__, "Fast orthophoto is turned on, automatically setting --skip - 3dmodel");
		arg.skip_3dmodel = true;
	}

	if (arg.pc_rectify && arg.pc_classify == false)
	{
		Logger::Write(__FUNCTION__, "Ground rectify is turned on, automatically turning on point cloud classification");
		arg.pc_classify = true;
	}

	if (arg.dtm && arg.pc_classify == false)
	{
		Logger::Write(__FUNCTION__, "DTM is turned on, automatically turning on point cloud classification");
		arg.pc_classify = true;
	}

	if (arg.skip_3dmodel && arg.use_3dmesh)
	{
		Logger::Write(__FUNCTION__, "--skip-3dmodel is set, but so is --use-3dmesh. --skip-3dmodel will be ignored.");
		arg.skip_3dmodel = false;
	}

	//if (arg.orthophoto_cutline and not arg.crop)
	{
		//Logger::Write(__FUNCTION__, "--orthophoto-cutline is set, but --crop is not. --crop will be set to 0.01");
		//arg.crop = 0.01;
	}
}

void Project::InitTree()
{
	//tree.dataset_raw = ms_DroneInputPath;

	tree.opensfm = XString::CombinePath(ms_DroneOutputPath, "opensfm");
	tree.opensfm_reconstruction = XString::CombinePath(tree.opensfm, "reconstruction.json");
	tree.opensfm_reconstruction_nvm = XString::CombinePath(tree.opensfm, "undistorted/reconstruction.nvm");
	tree.topocentric_reconstruction = XString::CombinePath(tree.opensfm, "reconstruction.topocentric.json");
	tree.geocoords_reconstruction = XString::CombinePath(tree.opensfm, "reconstruction.geocoords.json");

	tree.openmvs = XString::CombinePath(tree.opensfm, "undistorted/openmvs");
	tree.openmvs_model = XString::CombinePath(tree.openmvs, "scene_dense_dense_filtered.ply");

	tree.odm_filterpoints = XString::CombinePath(ms_DroneOutputPath, "odm_filterpoints");
	tree.filtered_point_cloud = XString::CombinePath(tree.odm_filterpoints, "point_cloud.ply");
	tree.filtered_point_cloud_stats = XString::CombinePath(tree.odm_filterpoints, "point_cloud_stats.json");

	tree.odm_meshing = XString::CombinePath(ms_DroneOutputPath, "odm_meshing");
	tree.odm_mesh = XString::CombinePath(tree.odm_meshing, "odm_mesh.ply");
	tree.odm_25dmesh = XString::CombinePath(tree.odm_meshing, "odm_25dmesh.ply");

	tree.odm_texturing = XString::CombinePath(ms_DroneOutputPath, "odm_texturing");
	tree.odm_25dtexturing = XString::CombinePath(ms_DroneOutputPath, "odm_texturing_25d");
	tree.odm_textured_model_obj = "odm_textured_model_geo.obj";

	tree.odm_georef_path = XString::CombinePath(ms_DroneOutputPath, "odm_georeferencing");
	tree.odm_georeferencing_model_laz = XString::CombinePath(tree.odm_georef_path, "odm_georeferenced_model.laz");

	tree.odm_dem = XString::CombinePath(ms_DroneOutputPath, "odm_den");
	tree.odm_dem_dtm = XString::CombinePath(tree.odm_dem, "dtm.tif");
	tree.odm_dem_dsm = XString::CombinePath(tree.odm_dem, "dsm.tif");

	tree.odm_orthophoto = XString::CombinePath(ms_DroneOutputPath, "odm_orthophoto");
	tree.odm_orthophoto_tif = XString::CombinePath(tree.odm_orthophoto, "odm_orthophoto.tif");
	tree.odm_orthophoto_corners = XString::CombinePath(tree.odm_orthophoto, "odm_orthophoto_corners.txt");
	tree.odm_orthophoto_log = XString::CombinePath(tree.odm_orthophoto, "odm_orthophoto_log.txt");
	tree.odm_orthophoto_render = XString::CombinePath(tree.odm_orthophoto, "odm_orthophoto_render.tif");

	tree.odm_report = XString::CombinePath(ms_DroneOutputPath, "odm_report");

	// executables

	XString lib_path = GetApp()->GetOdmLibPath();

	// QProcess that runs these is very picky about search paths / file extensions; it's 
	// safest to always provide the exension.
	tree.prog_opensfm = XString::CombinePath(lib_path, "SuperBuild/bin/opensfm/bin/opensfm.bat");
	tree.prog_densify = XString::CombinePath(lib_path, "SuperBuild/bin/OpenMVS/DensifyPointCloud.exe");
	tree.prog_orthophoto = XString::CombinePath(lib_path, "SuperBuild/bin/odm_orthophoto.exe");
	tree.prog_recon_tex = XString::CombinePath(lib_path, "SuperBuild/bin/texrecon.exe");
	tree.prog_recon_mesh = XString::CombinePath(lib_path, "SuperBuild/bin/OpenMVS/ReconstructMesh.exe");
	tree.prog_poisson = XString::CombinePath(lib_path, "SuperBuild/bin/PoissonRecon.exe");
	tree.prog_dem2points = XString::CombinePath(lib_path, "SuperBuild/bin/dem2points.exe");
	tree.prog_dem2mesh = XString::CombinePath(lib_path, "SuperBuild/bin/dem2mesh.exe");
	tree.prog_filter = XString::CombinePath(lib_path, "SuperBuild/bin/FPCFilter.exe");
	tree.prog_pdal = XString::CombinePath(lib_path, "SuperBuild/bin/pdal.exe");

	tree.prog_gdal_buildvrt = XString::CombinePath(lib_path, "venv/Lib/site-packages/osgeo/gdalbuildvrt.exe");
	tree.prog_gdal_translate = XString::CombinePath(lib_path, "venv/Lib/site-packages/osgeo/gdal_translate.exe");
}
