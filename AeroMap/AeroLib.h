#ifndef AEROLIB_H
#define AEROLIB_H

#include "AeroMap.h"

namespace AeroLib
{
	int  RunProgramEnv(const char* prog, QStringList args, const char* output_file = nullptr);
	int  RunProgramEnv(XString prog, QStringList args, XString output_file = "");
	void InitRunLog();

	struct Georef
	{
		int utm_zone;
		char hemi;
		double x;
		double y;
		bool is_valid;

		Georef()
			: utm_zone(0)
			, hemi(' ')
			, x(0.0)
			, y(0.0)
			, is_valid(false)
		{

		}
	};

	Georef ReadGeoref();

	__int64 CalcUnixEpoch(const char* dateTime);

	double Median(std::vector<double>& v);
	double Mean(std::vector<double>& v);

	void CreateFolder(XString path);
	void DeleteFolder(XString path);
	void Replace(XString src_file, XString dest_file);
	XString related_file_path(XString input_file_path, XString prefix = "", XString postfix = "");

	CPLErr gdal_fillnodata(XString src_file, XString dst_file, 
		double max_distance = 100.0, int smooth_iterations = 0, int src_band = 1, XString format = "GTiff");

	unsigned long long get_max_memory_bytes(int minimum = 100, double use_at_most = 0.5);
	double get_max_memory_percent(double minimum = 5.0, double use_at_most = 0.5);
}

#endif // #ifndef AEROLIB_H
