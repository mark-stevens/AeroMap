#ifndef PHOTO_H
#define PHOTO_H

#include <opencv2/opencv.hpp>	// OpenCV

#include "TinyEXIF.h"
#include "Calc.h"

class Photo
{
public:

	Photo(XString file_name);
	~Photo();

	XString GetFileName();

	int GetWidth();
	int GetHeight();
	int GetExifWidth();
	int GetExifHeight();
	int GetOrientation();

	XString GetMake();
	XString GetModel();

	double GetLatitude();
	double GetLongitude();
	double GetAltitude();

	double GetExposureTime();
	double GetFNumber();
	int GetIsoSpeed();
	int GetBitsPerSample();

	XString GetDateTime();
	__int64 GetEpoch();

	double GetFocalRatio();
	double GetFocalLength();

	XString GetCameraStrOSFM();
	XString GetCameraStrODM();

	XString GetBandName();
	int     GetBandIndex();

	bool is_thermal();
	bool is_rgb();

	bool has_speed();
	VEC3 get_speed();

	bool has_geo();
	bool has_ypr();

	static SizeType find_largest_photo_dims(const std::vector<Photo>& photos);
	static int find_largest_photo_dim(const std::vector<Photo>& photos);
	static Photo* find_largest_photo(const std::vector<Photo>& photos);
	static double get_mm_per_unit(int resolution_unit);

private:

	XString ms_file_name;
	cv::Mat m_image;
	TinyEXIF::EXIFInfo m_imageEXIF;

	int m_width;
	int m_height;
	int m_orientation;

	int m_exif_width;			// Original image width/height at capture time (before possible resizes)
	int m_exif_height;

	XString m_camera_make;
	XString m_camera_model;

	double m_gps_lat;
	double m_gps_lon;
	double m_gps_alt;

	double m_focal_length;
	double m_focal_ratio;
	XString m_date_time;			// capture time, "%Y:%m:%d %H:%M:%S" format
	__int64 m_epoch;			// capture time, unix epoch

	// Multi-band fields
	XString m_band_name;		// TODO: if 'multi-band' why isnt this a list?
	int m_band_index;
	//capture_uuid = None

	// Multi-spectral fields
	double m_fnumber;
	// self.radiometric_calibration = None
	// self.black_level = None
	// self.gain = None
	// self.gain_adjustment = None

	// Capture info
	double m_exposure_time;
	int m_iso_speed;
	int m_bits_per_sample;
	//        self.vignetting_center = None
	//        self.vignetting_polynomial = None
	//        self.spectral_irradiance = None
	//        self.horizontal_irradiance = None
	//        self.irradiance_scale_to_si = None
	//        self.utc_time = None

	// OPK angles
	double m_yaw_deg;
	double m_pitch_deg;
	double m_roll_deg;
	double m_omega;
	double m_phi;
	double m_kappa;

	// DLS - Downwelling Light Sensor
	//        self.sun_sensor = None
	//        self.dls_yaw = None			// xmp tag 'DLS:Yaw'
	//        self.dls_pitch = None			// 'DLS:Pitch'
	//        self.dls_roll = None			// 'DLS:Roll'

	// Aircraft speed
	double m_speedx;
	double m_speedy;
	double m_speedz;

	// self.center_wavelength = None
	// self.bandwidth = None

	// RTK
	//        self.gps_xy_stddev = None # Dilution of Precision X/Y
	//        self.gps_z_stddev = None # Dilution of Precision Z


	bool m_has_geo;
	bool m_has_ypr;
	bool m_has_speed;

	XString m_camera_str_osfm;		// camera id, opensfm format
	XString m_camera_str_odm;		// open drone map format

private:

	int parse_exif_values(XString file_name);
	void compute_opk();

	int yisleap(int year);
	int get_yday(int mon, int day, int year);
	__int64 CalcUnixEpoch(XString dateTime);

	double CalcFocalRatio();
	XString GetCameraString(bool opensfm);
};

#endif // #ifndef PHOTO_H
