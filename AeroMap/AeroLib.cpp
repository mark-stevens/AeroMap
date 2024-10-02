// AeroLib.cpp
// Common routines for AeroMap.
//

#include <gdal_alg.h>

#include "TextFile.h"
#include "AeroLib.h"

constexpr char* RUN_LOG = "run_log.txt";

namespace AeroLib
{
	int RunProgramEnv(const char* prog, QStringList args, const char* output_file)
	{
		// Run external program, setting environment variables as needed.
		//
		// Inputs:
		//		prog = executable, full path with extension
		//		args = arguments
		//		output_file = optional file to capture output
		//
		// *** This call will block ***
		//

		int status = 0;

		if (QFile::exists(prog) == false)
		{
			Logger::Write(__FUNCTION__, "Program file not found: '%s'", prog);
			assert(false);
			return -1;
		}

		// verify parameters
		for (XString arg : args)
		{
			if (arg.BeginsWith(" ") || arg.EndsWith(" "))
			{
				Logger::Write(__FUNCTION__, "Invalid parameter format: '%s'", arg.c_str());
				assert(false);
			}
		}

		// log program being executed
		XString run_log = XString::CombinePath(GetProject().GetDroneOutputPath(), RUN_LOG);
		FILE* pFile = fopen(run_log.c_str(), "at");
		if (pFile)
		{
			fprintf(pFile, "Running: %s", prog);
			for (XString arg : args)
				fprintf(pFile, " %s", arg.c_str());
			fprintf(pFile, "\n");
			fclose(pFile);
		}

		XString lib_path = GetApp()->GetOdmLibPath();

		// write 'virtual env'
		XString env_cfg = XString::CombinePath(lib_path, "venv/pyvenv.cfg");
		pFile = fopen(env_cfg.c_str(), "wt");
		if (pFile)
		{
			fprintf(pFile, "%s/venv/Scripts\n", lib_path.c_str());
			fprintf(pFile, "include-system-site-packages = false\n");
			fclose(pFile);
		}

		// The normal scenario starts from the current environment by calling QProcessEnvironment::systemEnvironment() and then proceeds to adding, 
		// changing, or removing specific variables. The resulting variable roster can then be applied to a QProcess with setProcessEnvironment().

		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

		env.insert("GDAL_DATA", XString::CombinePath(lib_path, "venv/Lib/site-packages/osgeo/data/gdal").c_str());
		env.insert("GDAL_DRIVER_PATH", XString::CombinePath(lib_path, "venv/Lib/site-packages/osgeo/gdalplugins").c_str());
		env.insert("OSFMBASE", XString::CombinePath(lib_path, "SuperBuild/bin/opensfm/bin").c_str());
		env.insert("PDAL_DRIVER_PATH", XString::CombinePath(lib_path, "SuperBuild/bin").c_str());
		env.insert("PROJ_LIB", XString::CombinePath(lib_path, "venv/Lib/site-packages/osgeo/data/proj").c_str());
		env.insert("PYENVCFG", XString::CombinePath(lib_path, "venv/pyvenv.cfg").c_str());
		env.insert("PYTHONPATH", XString::CombinePath(lib_path, "venv").c_str());
		env.insert("PYTHONPYCACHEPREFIX", "C:/ProgramData/AeroMap/pycache");
		env.insert("SBBIN", XString::CombinePath(lib_path, "SuperBuild/bin").c_str());
		env.insert("VIRTUAL_ENV", XString::CombinePath(lib_path, "venv").c_str());

		XString path = XString::CombinePath(lib_path, "venv/Scripts;");
		path += XString::CombinePath(lib_path, "venv/Lib/site-packages/osgeo;");
		path += XString::CombinePath(lib_path, "SuperBuild/bin;");
		path += XString::CombinePath(lib_path, "SuperBuild/bin/opensfm/bin;");
		env.insert("Path", path.c_str());

		QProcess* pProc = new QProcess();
		pProc->setProcessEnvironment(env);
		pProc->start(prog, args);

		// block until program completes
		bool wait_status = pProc->waitForFinished(-1);
		if (wait_status == false)
		{
			Logger::Write(__FUNCTION__, "waitForFinished() failed");
		}
			
		QProcess::ExitStatus exit_status = pProc->exitStatus();
		if (exit_status != QProcess::NormalExit)
		{
			Logger::Write(__FUNCTION__, "Program returned exit status: %d", (int)exit_status);
			assert(false);
			return -2;
		}

		XString std_out = QString(pProc->readAllStandardOutput());
		XString std_err = QString(pProc->readAllStandardError());

		if (std_err.GetLength() > 0)
		{
			std_err.Replace("\r\n", "\n");

			FILE* pFile = fopen(run_log.c_str(), "at");
			if (pFile)
			{
				fprintf(pFile, "%s", std_err.c_str());
				fclose(pFile);
			}
		}

		if (output_file != nullptr)
		{
			if (std_out.GetLength() > 0)
			{
				std_out.Replace("\r\n", "\n");

				FILE* pFile = fopen(output_file, "wt");
				if (pFile)
				{
					fprintf(pFile, "%s", std_out.c_str());
					fclose(pFile);
				}
			}
		}

		return status;
	}

	int RunProgramEnv(XString prog, QStringList args, XString output_file)
	{
		return RunProgramEnv(prog.c_str(), args, output_file.GetLength() > 0 ? output_file.c_str() : nullptr);
	}

	void InitRunLog()
	{
		XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), RUN_LOG);
		FILE* pFile = fopen(file_name.c_str(), "wt");
		fclose(pFile);
	}

	Georef ReadGeoref()
	{
		// Return georeference information from 'coords.txt'
		//

		Georef georef;

		georef.is_valid = false;

		if (QFile::exists(GetProject().GetCoordsFileName().c_str()))
		{
			TextFile textFile(GetProject().GetCoordsFileName().c_str());
			if (textFile.GetLineCount() >= 2)
			{
				// WGS84 UTM 32N
				// 322263 5157982
				XString line0 = textFile.GetLine(0).c_str();
				if (line0.Tokenize(" \t") == 3)
				{
					XString line1 = textFile.GetLine(1).c_str();
					if (line1.Tokenize(" \t") == 2)
					{
						XString s = line0.GetToken(2);
						if (s.EndsWith("N") || s.EndsWith("S"))
						{
							georef.hemi = s.Right(1)[0];

							s.DeleteRight(1);
							georef.utm_zone = s.GetInt();
							if (georef.utm_zone >= 1 && georef.utm_zone <= 60)
							{
								georef.x = line1.GetToken(0).GetDouble();
								georef.y = line1.GetToken(1).GetDouble();

								georef.is_valid = true;
							}
						}
					}
				}
			}
		}

		return georef;
	}

	int yisleap(int year)
	{
		return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	}

	int get_yday(int mon, int day, int year)
	{
		static const int days[2][13] = {
			{0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
			{0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
		};
		int leap = yisleap(year);

		return days[leap][mon] + day;
	}

	__int64 CalcUnixEpoch(const char* dateTime)
	{
		// Return seconds since Jan 1, 1970.
		//
		// Inputs:
		//		dateTime = "YYYY:MM:SS hh:mm:ss"
		// Outputs:
		//		return = seconds since Jan 1, 1970
		//

		int epoch = 0;

		XString str = dateTime;
		if (str.GetLength() == 19)
		{
			__int64 tm_year = str.Mid(0, 4).GetLong();
			__int64 tm_month = str.Mid(5, 2).GetLong();
			__int64 tm_day = str.Mid(8, 2).GetLong();
			__int64 tm_hour = str.Mid(11, 2).GetLong();
			__int64 tm_min = str.Mid(14, 2).GetLong();
			__int64 tm_sec = str.Mid(17, 2).GetLong();

			__int64 tm_yday = get_yday(tm_month, tm_day, tm_year);

			// yday is 1-based
			--tm_yday;

			tm_year -= 1900;

			epoch = tm_sec + tm_min * 60 + tm_hour * 3600 + tm_yday * 86400 +
				(tm_year - 70) * 31536000 + ((tm_year - 69) / 4) * 86400 -
				((tm_year - 1) / 100) * 86400 + ((tm_year + 299) / 400) * 86400;
		}

		return epoch;
	}

	double Median(std::vector<double>& v)
	{
		// Return median of v.
		// 
		// After returning, the elements in v may be reordered 
		// and the resulting order is implementation defined.
		//

		if (v.empty())
		{
			return 0.0;
		}
		auto n = v.size() / 2;
		nth_element(v.begin(), v.begin() + n, v.end());
		auto med = v[n];
		if (!(v.size() & 1))	// if size is even
		{
			auto max_it = max_element(v.begin(), v.begin() + n);
			med = (*max_it + med) / 2.0;
		}
		return med;
	}

	double Mean(std::vector<double>& v)
	{
		double mean = 0.0;

		// size of vector 
		int n = (int)v.size();
		if (n > 0)
		{
			// sum of the vector elements 
			double sum = accumulate(v.begin(), v.end(), 0.0);

			// average of the vector elements 
			mean = sum / (double)n;
		}

		return mean;
	}

	void CreateFolder(XString path)
	{
		QDir dir(path.c_str());
		if (dir.exists() == false)
			dir.mkpath(path.c_str());
	}

	void DeleteFolder(XString path)
	{
		QDir dir(path.c_str());
		if (dir.exists())
			dir.removeRecursively();
	}

	void Replace(XString src_file, XString dest_file)
	{
		// Drop-in replacement for python os.replace(),
		// for files only.
		//

		if (QFile::exists(src_file.c_str()))
		{
			if (QFile::exists(dest_file.c_str()))
				QFile::remove(dest_file.c_str());

			QFile::rename(src_file.c_str(), dest_file.c_str());
		}
	}

	XString related_file_path(XString input_file_path, XString prefix, XString postfix)
	{
		// For example : related_file_path("/path/to/file.ext", "a.", ".b")
		// -- > "/path/to/a.file.b.ext"
		//

		XString path = input_file_path.GetPathName();
		XString file_name = input_file_path.GetFileName();
		// path = path / to
		// filename = file.ext

		int pos = file_name.ReverseFind('.');
		if (pos > -1)
		{
			XString basename = file_name.Left(pos);
			XString ext = file_name.Mid(pos);
			// basename = file
			// ext = .ext

			XString root_name = XString::Format("%s%s%s%s", prefix.c_str(), basename.c_str(), postfix.c_str(), ext.c_str());
			return XString::CombinePath(path, root_name);
		}

		return input_file_path;
	}

	CPLErr gdal_fillnodata(XString src_file, XString dst_file, double max_distance, int smooth_iterations, int src_band, XString format)
	{
		// Fill nodata areas in a raster by interpolation
		//
		// Port of Frank Warmerdam's gdal_fillnodata.py
		//
		// Inputs:
		//		src_file		=
		//		dst_file		=
		//		max_distance	= maximum distance (in pixels) that the algorithm will search out for values to interpolate.The default is 100 pixels.
		//		smooth_iterations	=
		//		src_band		= 1-based
		//		format			= name of gdal driver to use
		//

		CPLErr result = CPLErr::CE_None;
		
		//	mask = 'default'

		// Open source file

		GDALDataset* src_ds = nullptr;
		if (dst_file.GetLength() == 0)
			src_ds = (GDALDataset*)GDALOpen(src_file.c_str(), GA_Update);
		else
			src_ds = (GDALDataset*)GDALOpen(src_file.c_str(), GA_ReadOnly);

		if (src_ds == nullptr)
		{
			Logger::Write(__FUNCTION__, "Unable to open '%s'", src_file.c_str());
			assert(false);
			//return nullptr;
		}

		GDALRasterBand* pSrcBand = src_ds->GetRasterBand(src_band);

		GDALDataset* mask_ds = nullptr;
		//if (mask == "default")
		//	maskband = srcband.GetMaskBand()
		//else if (mask == "none")
		//	maskband = nullptr;
		//else
		//{
		//	mask_ds = gdal.Open(mask)
		//	maskband = mask_ds.GetRasterBand(1)
		//}

		// Create output file if one is specified.

		GDALDataset* dst_ds = nullptr;
		if (dst_file.GetLength() > 0)
		{
			GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(format.c_str()));
			assert(pDriver != nullptr);

			// params: output file, src datasource, strict flag, options, progress...
			dst_ds = pDriver->CreateCopy(dst_file.c_str(), src_ds, 0, nullptr, nullptr, nullptr);
			assert(dst_ds != nullptr);

			XString wkt = src_ds->GetProjectionRef();
			if (wkt.GetLength() > 0)
				dst_ds->SetProjection(wkt);

			double geoTransform[6] = { 0 };
			CPLErr err = src_ds->GetGeoTransform(geoTransform);
			if (err == CPLErr::CE_None)
				dst_ds->SetGeoTransform(geoTransform);

			GDALRasterBand* pDestBand = dst_ds->GetRasterBand(1);

			int success = 0;
			double ndv = pSrcBand->GetNoDataValue(&success);
			if (success)
				pDestBand->SetNoDataValue(ndv);

			GDALColorInterp color_interp = pSrcBand->GetColorInterpretation();
			pDestBand->SetColorInterpretation(color_interp);
			if (color_interp == GCI_PaletteIndex)
			{
				GDALColorTable* color_table = pSrcBand->GetColorTable();
				pDestBand->SetColorTable(color_table);
			}

			char** fill_opts = nullptr;
			
			GDALRasterBandH hTargetBand = GDALRasterBand::ToHandle(pDestBand);
			GDALRasterBandH hMaskBand = 0;

			result = GDALFillNodata(
				hTargetBand,			// hTargetBand -- the raster band to be modified in place. 
				hMaskBand,				// hMaskBand -- mask band indicating pixels to be interpolated (zero valued). If hMaskBand is set to NULL, 
										// this method will internally use the mask band returned by GDALGetMaskBand(hTargetBand). 
				max_distance,			// dfMaxSearchDist -- the maximum number of pixels to search in all directions to find values to interpolate from. 
				0,						// bDeprecatedOption -- unused argument, should be zero.
				smooth_iterations,		// nSmoothingIterations -- the number of 3x3 smoothing filter passes to run(0 or more).
				fill_opts,				// TEMP_FILE_DRIVER, NODATA, INTERPOLATION
				nullptr,				// GDALProgressFunc --  progress function to report completion.
				nullptr);				// void* pProgressArg -- callback data for progress function
		}

		GDALClose(src_ds);
		GDALClose(dst_ds);
		GDALClose(mask_ds);

		return result;
	}

	unsigned long long get_max_memory_bytes(int minimum, double use_at_most)
	{
		// Inputs:
		//		minimum = minimum value to return (return value will never be lower than this)
		//		use_at_most = use at most this fraction of the available memory. 0.5 = use at most 50% of available memory
		// Outputs:
		//		return = value of memory to use, bytes
		//

		// typedef struct _MEMORYSTATUSEX {
		//		DWORD dwLength;						// Size of the structure, in bytes. You must set this member before calling	GlobalMemoryStatusEx.
		//		DWORD dwMemoryLoad;					// Number between 0 and 100 that specifies the approximate percentage of physical memory that is in use
		//		DWORDLONG ullTotalPhys;				// Amount of actual physical memory, in bytes.	
		//		DWORDLONG ullAvailPhys;				// Amount of physical memory currently available, in bytes. 
		//											// This is the amount of physical memory that can be immediately reused without having to write its contents to disk first. 
		//											// It is the sum of the size of the standby, free, and zero lists.
		//		DWORDLONG ullTotalPageFile;			// Current committed memory limit for the system or the current process, whichever is smaller, in bytes
		//		DWORDLONG ullAvailPageFile;			// Maximum amount of memory the current process can commit, in bytes
		//		DWORDLONG ullTotalVirtual;			// Size of the user-mode portion of the virtual address space of the calling process, in bytes. This value depends on the type of process, 
		//											// the type of processor, and the configuration of the operating system.
		//		DWORDLONG ullAvailVirtual;			// Amount of unreserved and uncommitted memory currently in the user-mode portion of the virtual address space of the calling process, in bytes.
		//		DWORDLONG ullAvailExtendedVirtual;	// Reserved
		// } MEMORYSTATUSEX, * LPMEMORYSTATUSEX;

		MEMORYSTATUSEX memory_status;
		ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
		memory_status.dwLength = sizeof(MEMORYSTATUSEX);
		if (GlobalMemoryStatusEx(&memory_status))
		{
			Logger::Write(__FUNCTION__, "GlobalMemoryStatusEx():");
			Logger::Write(__FUNCTION__, "    Total RAM: %ull MB", memory_status.ullTotalPhys / (1024 * 1024));
			Logger::Write(__FUNCTION__, "    Avail RAM: %ull MB", memory_status.ullAvailPhys / (1024 * 1024));
			Logger::Write(__FUNCTION__, "    Total Virtual: %ull MB", memory_status.ullTotalVirtual / (1024 * 1024));
			Logger::Write(__FUNCTION__, "    Avail Virtual: %ull MB", memory_status.ullAvailVirtual / (1024 * 1024));
		}
		else
		{
			Logger::Write(__FUNCTION__, "GlobalMemoryStatusEx() call failed.");
		}

		unsigned long long vmem_avail = memory_status.ullAvailVirtual;
		unsigned long long vmem_max = (unsigned long long)(vmem_avail * use_at_most);
		return std::max((unsigned long long)minimum, vmem_max);
	}

	double get_max_memory_percent(double minimum, double use_at_most)
	{
		// Inputs:
		//		minimum minimum value to return (return value will never be lower than this)
		//		use_at_most use at most this fraction of the available memory. 0.5 = use at most 50 % of available memory
		// Outputs:
		//		return = percentage value of memory to use (75 = 75 %).
		//

		MEMORYSTATUSEX memory_status;
		ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
		memory_status.dwLength = sizeof(MEMORYSTATUSEX);
		if (GlobalMemoryStatusEx(&memory_status) == false)
		{
			Logger::Write(__FUNCTION__, "GlobalMemoryStatusEx() call failed.");
		}

		unsigned long long vmem_total = memory_status.ullTotalVirtual;
		unsigned long long vmem_avail = memory_status.ullAvailVirtual;
		double percent_avail = (double)vmem_avail / (double)vmem_total;
		percent_avail *= 100.0;

		return std::max(minimum, percent_avail * use_at_most);
	}
}
