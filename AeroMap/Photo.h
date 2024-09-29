#ifndef PHOTO_H
#define PHOTO_H

#include "Project.h"

class Photo
{
public:

	Photo(XString file_name);
	~Photo();

	bool is_thermal();
	bool is_rgb();

	XString GetMake();
	XString GetModel();

	static SizeType find_largest_photo_dims(const std::vector<Project::ImageType>& photos);
	static int find_largest_photo_dim(const std::vector<Project::ImageType>& photos);
	static Project::ImageType* find_largest_photo(const std::vector<Project::ImageType>& photos);
	static double get_mm_per_unit(int resolution_unit);

private:

	XString ms_file_name;

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

	double m_focal_ratio;
	XString m_utc_time;			// capture time, "%Y:%m:%d %H:%M:%S" format
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
	//        self.yaw = None
	//        self.pitch = None
	//        self.roll = None
	//        self.omega = None
	//        self.phi = None
	//        self.kappa = None

	// DLS
	//        self.sun_sensor = None
	//        self.dls_yaw = None
	//        self.dls_pitch = None
	//        self.dls_roll = None

	// Aircraft speed
	//        self.speed_x = None
	//        self.speed_y = None
	//        self.speed_z = None

	// self.center_wavelength = None
	// self.bandwidth = None

	// RTK
	//        self.gps_xy_stddev = None # Dilution of Precision X/Y
	//        self.gps_z_stddev = None # Dilution of Precision Z

	double m_yaw;
	double m_pitch;
	double m_roll;

	double m_omega;
	double m_phi;
	double m_kappa;

private:

	int parse_exif_values(XString file_name);
	void compute_opk();
};

#endif // #ifndef PHOTO_H
