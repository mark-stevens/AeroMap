#ifndef PROJECT_H
#define PROJECT_H

#include "exif.h"			// easy exif header
#include "Gis.h"
#include "XString.h"

#include "gdal_priv.h"
#include "cpl_conv.h"		// for CPLMalloc()
#include "ogrsf_frmts.h"

#include "LasFile.h"

#include <QObject>

#include <vector>

struct ArgType
{
	bool dtm;
	bool dsm;

	double crop;

	double	dem_resolution;
	int		dem_gapfill_steps;
	int		dem_decimation;
	bool	dem_euclidean_map;

	double orthophoto_resolution;

	bool use_3dmesh = false;
	bool skip_3dmodel = false;
	bool skip_report = false;
	bool skip_orthophoto = false;
	bool fast_orthophoto = false;

	int mesh_oct_tree_depth = 11;
	
	bool pc_classify;
	bool pc_rectify;
	double pc_filter;
	double pc_sample;

	double smrf_scalar;
	double smrf_slope;
	double smrf_threshold;
	double smrf_window;

	bool ignore_gsd;

	int max_concurrency;
};
extern ArgType arg;

struct TreeType
{
	XString opensfm;
	XString opensfm_reconstruction;
	XString opensfm_reconstruction_nvm;
	XString topocentric_reconstruction;
	XString geocoords_reconstruction;

	XString openmvs;
	XString openmvs_model;

	XString odm_filterpoints;				// root output filter path
	XString filtered_point_cloud;
	XString filtered_point_cloud_stats;

	XString odm_meshing;					// root output mesh path
	XString odm_mesh;
	XString odm_25dmesh;

	XString odm_georef_path;				// root output georef path
	XString odm_georeferencing_model_laz;

	XString odm_dem;						// root output dem path
	XString odm_dem_dtm;					// .tif files
	XString odm_dem_dsm;
	XString dem_dtm;						// terrain model files
	XString dem_dsm;

	XString odm_texturing;					// root output texture path
	XString odm_25dtexturing;
	XString odm_textured_model_obj;

	XString odm_orthophoto;					// root output orthophoto path
	XString odm_orthophoto_tif;
	XString odm_orthophoto_corners;
	XString odm_orthophoto_log;
	XString odm_orthophoto_render;

	XString odm_report;						// root output report path

	// executables
	
	XString prog_opensfm;
	XString prog_densify;
	XString prog_pdal;
	XString prog_orthophoto;
	XString prog_recon_tex;
	XString prog_recon_mesh;
	XString prog_poisson;
	XString prog_dem2points;
	XString prog_dem2mesh;
	XString prog_filter;
	XString prog_gdal_buildvrt;
	XString prog_gdal_translate;
	XString prog_gdal_warp;
};
extern TreeType tree;

class Project : public QObject
{
	Q_OBJECT

public:

	struct ImageType
	{
		XString file_name;
		easyexif::EXIFInfo exif;

		// pre-computed values
		double focal_ratio;
		__int64 epoch;					// capture time, unix epoch
		XString camera_str_osfm;		// camera id string, opensfm format
		XString camera_str_odm;			// camera id string, odm format

		ImageType()
		{
		}
	};
		
	struct LidarType
	{
		GIS::GEODATA type;		// geospatial data type, las or laz only
		XString src;			// full path/name of lidar source file
		LasFile* pFile;
		bool exists;			// false if file not found

		LidarType()
			: type(GIS::GEODATA::None)
			, pFile(nullptr)
		{
		}
	};
	
public:

	Project();
	~Project();

	int Load(const char* fileName);
	int Save(const char* fileName = nullptr);
	void Clear();
	void Update();
	bool IsDirty();

	int LoadImageList();
	int GetImageCount();
	const std::vector<ImageType>& GetImageList();

	XString GetFileName();
	XString GetName();

	XString GetDroneInputPath();
	void    SetDroneInputPath(XString path);
	XString GetDroneOutputPath();
	void    SetDroneOutputPath(XString path);

	XString GetCoordsFileName();

	int		 GetLidarFileCount();
	void	 AddLidarFile(const char* pathName);
	void	 RemoveLidarFile(int index);
	XString  GetLidarFileName(int index);
	LasFile* GetLasFile(int index);
	bool	 GetLidarExists(int index);

	static const char* GetDefaultExt();		// default file extension
	
signals:

	void projectChanged_sig();				// any change to project structure

private:

	std::vector<ImageType> m_ImageList;		// drone photogrammetry input images
	std::vector<LidarType> mv_Lidar;		// lidar files
	
	XString ms_FileName;		// project path/file name
	XString ms_ProjectName;		// project name

	XString ms_DroneInputPath;	// location of drone photogrammetry inputs
	XString ms_DroneOutputPath;	// root folder that will receive outputs

	int m_ErrorCtr;				// # of errors in project file
	bool mb_IsDirty;			// unsaved changes

private:

	int  GetIndentLevel(XString& str);
	void FreeResources();
	void LoadData();

	void InitArg();
	void InitTree();

	// private methods to update internal component lists
	// with correctly formatted data; these do not set dirty
	// flags or emit signals
	void AddLidarFileToList(const char* pathName);
};

#endif // #ifndef PROJECT_H
