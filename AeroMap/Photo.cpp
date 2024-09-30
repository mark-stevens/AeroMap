// Photo.cpp
// Port of odm photo.py
//

#include <iostream> // std::cout
#include <fstream>  // std::ifstream
#include <algorithm>

#include "exif.h"       // easy exif header
#include "Logger.h"
#include "TinyEXIF.h"
#include "Photo.h"

//import logging
//import re
//import os
//import math
//
//import exifread
//import numpy as np
//from six import string_types
//from datetime import datetime, timedelta, timezone
//import pytz
//
//from opendm import io
//from opendm import log
//from opendm import system
//from opendm.rollingshutter import get_rolling_shutter_readout
//import xmltodict as x2d
//from opendm import get_image_size
//from xml.parsers.expat import ExpatError
//from opensfm.sensors import sensor_data
//from opensfm.geo import ecef_from_lla
//
//projections = ['perspective', 'fisheye', 'fisheye_opencv', 'brown', 'dual', 'equirectangular', 'spherical']

Photo::Photo(XString file_name)
    : ms_file_name(file_name)
    , m_width(0), m_height(0)
    , m_exif_width(0), m_exif_height(0)
    , m_orientation(1)
    , m_gps_lat(0.0), m_gps_lon(0.0), m_gps_alt(0.0)
    , m_focal_ratio(0.0)
    , m_focal_length(0.0)
    , m_band_name("RGB")
    , m_band_index(0)
    , m_fnumber(0.0)
    , m_exposure_time(0.0)
    , m_iso_speed(0)
    , m_epoch(0)
    , m_yaw(0.0), m_pitch(0.0), m_roll(0.0)
    , m_omega(0.0), m_phi(0.0), m_kappa(0.0)
    , m_has_geo(false), m_has_ypr(false), m_has_speed(false)
{
	// self.mask = None

	// Misc SFM
	//        self.camera_projection = 'brown'
	
	// parse values from metadata
    parse_exif_values(file_name);
}

Photo::~Photo()
{

}

int Photo::parse_exif_values(XString file_name)
{
    if ((file_name.EndsWithNoCase(".jpg") == false) && (file_name.EndsWithNoCase(".jpeg") == false))
        return -1;

    //tags = {}
    //xtags = {}

//TODO:
// i think i like the idea of move values "up" to class level like odm,
// rather than constantly accessing "image.exif." - also decouples image
// properties from any particular exif lib

//TODO:
//parsing twice - eventually plan to drop easy exif

        // open a stream to read just the necessary parts of the image file
        std::ifstream istream(file_name.c_str(), std::ifstream::binary);

        // parse image EXIF and XMP metadata
        TinyEXIF::EXIFInfo imageEXIF(istream);
        if (imageEXIF.Fields)
        {
            std::cout
                << "Image Description " << imageEXIF.ImageDescription << "\n"
                << "Image Resolution " << imageEXIF.ImageWidth << "x" << imageEXIF.ImageHeight << " pixels\n"
                << "Camera Model " << imageEXIF.Make << " - " << imageEXIF.Model << "\n"
                << "Focal Length " << imageEXIF.FocalLength << " mm" << std::endl;

            m_has_geo = imageEXIF.GeoLocation.hasLatLon();
            m_has_ypr = imageEXIF.GeoLocation.hasOrientation();
            m_has_speed = imageEXIF.GeoLocation.hasSpeed();

            //int status = imageEXIF.parseFromXMPSegment(buf, len);

            // how to read band info?
            ////            band_name = self.get_xmp_tag(xtags, ['Camera:BandName', '@Camera:BandName', 'FLIR:BandName'])

            //??? imageEXIF.parseFromXMPSegment()
        }

        // Read the JPEG file into a buffer
        FILE* fp = fopen(file_name.c_str(), "rb");
        if (!fp)
        {
            Logger::Write(__FUNCTION__, "Unable to open image file: '%s'", file_name.c_str());
            return -1;
        }
        fseek(fp, 0, SEEK_END);
        unsigned long fsize = ftell(fp);
        rewind(fp);
        unsigned char* buf = new unsigned char[fsize];
        if (fread(buf, 1, fsize, fp) != fsize)
        {
            Logger::Write(__FUNCTION__, "Unable to open read file: '%s'", file_name.c_str());
            delete[] buf;
            return -1;
        }
        fclose(fp);

        // Parse EXIF
        easyexif::EXIFInfo exif;
        int code = exif.parseFrom(buf, fsize);
        delete[] buf;
        if (code)
        {
            Logger::Write(__FUNCTION__, "Error parsing EXIF: '%s' (code %d)", file_name.c_str(), code);
            return -1;
        }

        // pre-computed values

        m_focal_ratio = CalcFocalRatio(exif);      //TODO: move here from AeroLib
        m_date_time = exif.DateTime.c_str();
        m_epoch = CalcUnixEpoch(m_date_time);
        m_camera_str_osfm = GetCameraString(exif, true);
        m_camera_str_odm = GetCameraString(exif, false);

    //with open(_path_file, 'rb') as f:
    //    tags = exifread.process_file(f, details=True, extract_thumbnail=False)
    //    try:

        m_width = exif.ImageWidth;
        m_height = exif.ImageHeight;
        m_orientation = exif.Orientation;

        m_exif_width = m_width;
        m_exif_height = m_height;

        m_camera_make = exif.Make.c_str();
        m_camera_model = exif.Model.c_str();

        m_gps_lat = exif.GeoLocation.Latitude;
        m_gps_lon = exif.GeoLocation.Longitude;
        m_gps_alt = exif.GeoLocation.Altitude;

        //    # Warn if GPS coordinates are suspiciously wrong
        //    if self.latitude is not None and self.latitude == 0 and \
        //        self.longitude is not None and self.longitude == 0:
        //        log.ODM_WARNING("%s has GPS position (0,0), possibly corrupted" % self.filename)

        // Capture info
        m_exposure_time = exif.ExposureTime;
        m_iso_speed = exif.ISOSpeedRatings;
        m_bits_per_sample = exif.BitsPerSample;
        // self.vignetting_center = None
        // self.vignetting_polynomial = None
        // self.spectral_irradiance = None
        // self.horizontal_irradiance = None
        // self.irradiance_scale_to_si = None

        m_focal_length = exif.FocalLength;

    //    try:
    //        if 'Image Tag 0xC61A' in tags:
    //            self.black_level = self.list_values(tags['Image Tag 0xC61A'])
    //        elif 'BlackLevel' in tags:
    //            self.black_level = self.list_values(tags['BlackLevel'])
    //        elif 'Image BlackLevel' in tags:
    //            self.black_level = self.list_values(tags['Image BlackLevel'])

    //        if 'MakerNote SpeedX' in tags and \
    //            'MakerNote SpeedY' in tags and \
    //            'MakerNote SpeedZ' in tags:
    //            self.speed_x = self.float_value(tags['MakerNote SpeedX'])
    //            self.speed_y = self.float_value(tags['MakerNote SpeedY'])
    //            self.speed_z = self.float_value(tags['MakerNote SpeedZ'])

    //    except Exception as e:
    //        log.ODM_WARNING("Cannot read extended EXIF tags for %s: %s" % (self.filename, str(e)))

    //    # Extract XMP tags
    //    f.seek(0)
    //    xmp = self.get_xmp(f)

    //    for xtags in xmp:
    //        try:
    //            band_name = self.get_xmp_tag(xtags, ['Camera:BandName', '@Camera:BandName', 'FLIR:BandName'])
    //            if band_name is not None:
    //                self.band_name = band_name.replace(" ", "")

    //            self.set_attr_from_xmp_tag('band_index', xtags, [
    //                'DLS:SensorId', # Micasense RedEdge
    //                '@Camera:RigCameraIndex', # Parrot Sequoia, Sentera 21244-00_3.2MP-GS-0001
    //                'Camera:RigCameraIndex', # MicaSense Altum
    //            ])

    //            self.set_attr_from_xmp_tag('radiometric_calibration', xtags, [
    //                'MicaSense:RadiometricCalibration',
    //            ])

    //            self.set_attr_from_xmp_tag('vignetting_center', xtags, [
    //                'Camera:VignettingCenter',
    //                'Sentera:VignettingCenter',
    //            ])

    //            self.set_attr_from_xmp_tag('vignetting_polynomial', xtags, [
    //                'Camera:VignettingPolynomial',
    //                'Sentera:VignettingPolynomial',
    //            ])

    //            self.set_attr_from_xmp_tag('horizontal_irradiance', xtags, [
    //                'Camera:HorizontalIrradiance'
    //            ], float)

    //            self.set_attr_from_xmp_tag('irradiance_scale_to_si', xtags, [
    //                'Camera:IrradianceScaleToSIUnits'
    //            ], float)

    //            self.set_attr_from_xmp_tag('sun_sensor', xtags, [
    //                'Camera:SunSensor',
    //            ], float)

    //            self.set_attr_from_xmp_tag('spectral_irradiance', xtags, [
    //                'Camera:SpectralIrradiance',
    //                'Camera:Irradiance',
    //            ], float)

    //            self.set_attr_from_xmp_tag('capture_uuid', xtags, [
    //                '@drone-dji:CaptureUUID', # DJI
    //                'MicaSense:CaptureId', # MicaSense Altum
    //                '@Camera:ImageUniqueID', # sentera 6x
    //                '@Camera:CaptureUUID', # Parrot Sequoia
    //            ])

    //            self.set_attr_from_xmp_tag('gain', xtags, [
    //                '@drone-dji:SensorGain'
    //            ], float)

    //            self.set_attr_from_xmp_tag('gain_adjustment', xtags, [
    //                '@drone-dji:SensorGainAdjustment'
    //            ], float)

    //            # Camera make / model for some cameras is stored in the XMP
    //            if self.camera_make == '':
    //                self.set_attr_from_xmp_tag('camera_make', xtags, [
    //                    '@tiff:Make'
    //                ])
    //            if self.camera_model == '':
    //                self.set_attr_from_xmp_tag('camera_model', xtags, [
    //                    '@tiff:Model'
    //                ])

    //            # DJI GPS tags
    //            self.set_attr_from_xmp_tag('longitude', xtags, [
    //                '@drone-dji:Longitude'
    //            ], float)
    //            self.set_attr_from_xmp_tag('latitude', xtags, [
    //                '@drone-dji:Latitude'
    //            ], float)
    //            self.set_attr_from_xmp_tag('altitude', xtags, [
    //                '@drone-dji:AbsoluteAltitude'
    //            ], float)

    //            # Phantom 4 RTK
    //            if '@drone-dji:RtkStdLon' in xtags:
    //                y = float(self.get_xmp_tag(xtags, '@drone-dji:RtkStdLon'))
    //                x = float(self.get_xmp_tag(xtags, '@drone-dji:RtkStdLat'))
    //                self.gps_xy_stddev = max(x, y)

    //                if '@drone-dji:RtkStdHgt' in xtags:
    //                    self.gps_z_stddev = float(self.get_xmp_tag(xtags, '@drone-dji:RtkStdHgt'))
    //            else:
    //                self.set_attr_from_xmp_tag('gps_xy_stddev', xtags, [
    //                    '@Camera:GPSXYAccuracy',
    //                    'GPSXYAccuracy'
    //                ], float)
    //                self.set_attr_from_xmp_tag('gps_z_stddev', xtags, [
    //                    '@Camera:GPSZAccuracy',
    //                    'GPSZAccuracy'
    //                ], float)

    //            # DJI Speed tags
    //            if '@drone-dji:FlightXSpeed' in xtags and \
    //                '@drone-dji:FlightYSpeed' in xtags and \
    //                '@drone-dji:FlightZSpeed' in xtags:
    //                self.set_attr_from_xmp_tag('speed_x', xtags, [
    //                    '@drone-dji:FlightXSpeed'
    //                ], float)
    //                self.set_attr_from_xmp_tag('speed_y', xtags, [
    //                    '@drone-dji:FlightYSpeed',
    //                ], float)
    //                self.set_attr_from_xmp_tag('speed_z', xtags, [
    //                    '@drone-dji:FlightZSpeed',
    //                ], float)

    //            # Account for over-estimation
    //            if self.gps_xy_stddev is not None:
    //                self.gps_xy_stddev *= 2.0
    //            if self.gps_z_stddev is not None:
    //                self.gps_z_stddev *= 2.0

    //            if 'DLS:Yaw' in xtags:
    //                self.set_attr_from_xmp_tag('dls_yaw', xtags, ['DLS:Yaw'], float)
    //                self.set_attr_from_xmp_tag('dls_pitch', xtags, ['DLS:Pitch'], float)
    //                self.set_attr_from_xmp_tag('dls_roll', xtags, ['DLS:Roll'], float)

    //            camera_projection = self.get_xmp_tag(xtags, ['@Camera:ModelType', 'Camera:ModelType'])
    //            if camera_projection is not None:
    //                camera_projection = camera_projection.lower()

    //                # Parrot Sequoia's "fisheye" model maps to "fisheye_opencv"
    //                if camera_projection == "fisheye" and self.camera_make.lower() == "parrot" and self.camera_model.lower() == "sequoia":
    //                    camera_projection = "fisheye_opencv"

    //                if camera_projection in projections:
    //                    self.camera_projection = camera_projection

    //            # OPK
    //            self.set_attr_from_xmp_tag('yaw', xtags, ['@drone-dji:FlightYawDegree', '@Camera:Yaw', 'Camera:Yaw'], float)
    //            self.set_attr_from_xmp_tag('pitch', xtags, ['@drone-dji:GimbalPitchDegree', '@Camera:Pitch', 'Camera:Pitch'], float)
    //            self.set_attr_from_xmp_tag('roll', xtags, ['@drone-dji:GimbalRollDegree', '@Camera:Roll', 'Camera:Roll'], float)

    //            # Normalize YPR conventions (assuming nadir camera)
    //            # Yaw: 0 --> top of image points north
    //            # Yaw: 90 --> top of image points east
    //            # Yaw: 270 --> top of image points west
    //            # Pitch: 0 --> nadir camera
    //            # Pitch: 90 --> camera is looking forward
    //            # Roll: 0 (assuming gimbal)
    //            if self.has_ypr():
    //                if self.camera_make.lower() in ['dji', 'hasselblad']:
    //                    self.pitch = 90 + self.pitch

    //                if self.camera_make.lower() == 'sensefly':
    //                    self.roll *= -1

    //        except Exception as e:
    //            log.ODM_WARNING("Cannot read XMP tags for %s: %s" % (self.filename, str(e)))

    //        # self.set_attr_from_xmp_tag('center_wavelength', xtags, [
    //        #     'Camera:CentralWavelength'
    //        # ], float)

    //        # self.set_attr_from_xmp_tag('bandwidth', xtags, [
    //        #     'Camera:WavelengthFWHM'
    //        # ], float)

    // Special case band handling for AeroVironment Quantix images
    // for some reason, they don't store band information in EXIFs
    //if self.camera_make.lower() == 'aerovironment' and \
    //    self.camera_model.lower() == 'quantix':
    //    matches = re.match("IMG_(\d+)_(\w+)\.\w+", self.filename, re.IGNORECASE)
    //    if matches:
    //        band_aliases = {
    //            'GRN': 'Green',
    //            'NIR': 'Nir',
    //            'RED': 'Red',
    //            'RGB': 'RedGreenBlue',
    //        }
    //        self.capture_uuid = matches.group(1)
    //        self.band_name = band_aliases.get(matches.group(2), matches.group(2))

    //# Sanitize band name since we use it in folder paths
    //self.band_name = re.sub('[^A-Za-z0-9]+', '', self.band_name)

    //self.compute_focal(tags, xtags)
    //self.compute_opk()

    return 0;
}

XString Photo::GetCameraString(easyexif::EXIFInfo exif, bool opensfm)
{
    // Return camera id string.
    //

    double focal_ratio = CalcFocalRatio(exif);

    XString camera_str = XString::Format("%s %s %d %d brown %0.4f",
        exif.Make.c_str(), exif.Model.c_str(),
        exif.ImageWidth, exif.ImageHeight,
        focal_ratio);

    // opensfm and odm have slightly different formats
    if (opensfm)
        camera_str.Insert(0, "v2 ");

    camera_str.MakeLower();

    return camera_str;
}

SizeType Photo::find_largest_photo_dims(const std::vector<Project::ImageType>& photos)
{
	SizeType max_dims;
	unsigned int max_mp = 0;

	for (auto p : photos)
	{
		unsigned int w = p.exif.ImageWidth;
		unsigned int h = p.exif.ImageHeight;

		if (w == 0 || h == 0)
			continue;

		unsigned int mp = w * h;
		if (mp > max_mp)
		{
			max_mp = mp;
			max_dims.cx = (long)w;
			max_dims.cy = (long)h;
		}
	}

	return max_dims;
}

int Photo::find_largest_photo_dim(const std::vector<Project::ImageType>& photos)
{
	int max_dim = 0;

	for (auto p : photos)
	{
		int w = p.exif.ImageWidth;
		int h = p.exif.ImageHeight;

		if (w == 0 || h == 0)
			continue;

		max_dim = max(max_dim, max(w, h));
	}

	return max_dim;
}

Project::ImageType* Photo::find_largest_photo(const std::vector<Project::ImageType>& photos)
{
	Project::ImageType* max_p = nullptr;

	int max_area = 0;
	for (auto p : photos)
	{
		int w = p.exif.ImageWidth;
		int h = p.exif.ImageHeight;

		if (w == 0 || h == 0)
			continue;

		int area = w * h;

        if (area > max_area)
		{
			max_area = area;
			max_p = &p;
		}
	}
	
	return max_p;
}

double Photo::get_mm_per_unit(int resolution_unit)
{
	// Length of a resolution unit in millimeters.
	//
	// Uses the values from the EXIF specs in
	// https://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/EXIF.html
	//
	// Args:
	//    resolution_unit: the resolution unit value given in the EXIF
	//

	double mm_per_unit = 0.0;

	if (resolution_unit == 2)			// inch
		mm_per_unit = 25.4;
	else if (resolution_unit == 3)		// cm
		mm_per_unit = 10;
	else if (resolution_unit == 4)		// mm
		mm_per_unit = 1;
	else if (resolution_unit == 5)		// um
		mm_per_unit = 0.001;
	else
	{
		Logger::Write(__FUNCTION__, "Unknown EXIF resolution unit: %d", resolution_unit);
		assert(false);
	}

	return mm_per_unit;
}

//    def __str__(self):
//        return '{} | camera: {} {} | dimensions: {} x {} | lat: {} | lon: {} | alt: {} | band: {} ({})'.format(
//                            self.filename, self.camera_make, self.camera_model, self.width, self.height,
//                            self.latitude, self.longitude, self.altitude, self.band_name, self.band_index)
//
//    def set_mask(self, mask):
//        self.mask = mask
//
//    def update_with_geo_entry(self, geo_entry):
//        self.latitude = geo_entry.y
//        self.longitude = geo_entry.x
//        self.altitude = geo_entry.z
//        if geo_entry.yaw is not None and geo_entry.pitch is not None and geo_entry.roll is not None:
//            self.yaw = geo_entry.yaw
//            self.pitch = geo_entry.pitch
//            self.roll = geo_entry.roll
//            self.dls_yaw = geo_entry.yaw
//            self.dls_pitch = geo_entry.pitch
//            self.dls_roll = geo_entry.roll
//        self.gps_xy_stddev = geo_entry.horizontal_accuracy
//        self.gps_z_stddev = geo_entry.vertical_accuracy

//    def set_attr_from_xmp_tag(self, attr, xmp_tags, tags, cast=None):
//        v = self.get_xmp_tag(xmp_tags, tags)
//        if v is not None:
//            if cast is None:
//                setattr(self, attr, v)
//            else:
//                # Handle fractions
//                if (cast == float or cast == int) and "/" in v:
//                    v = self.try_parse_fraction(v)
//                setattr(self, attr, cast(v))

//    def get_xmp_tag(self, xmp_tags, tags):
//        if isinstance(tags, str):
//            tags = [tags]
//
//        for tag in tags:
//            if tag in xmp_tags:
//                t = xmp_tags[tag]
//
//                if isinstance(t, string_types):
//                    return str(t)
//                elif isinstance(t, dict):
//                    items = t.get('rdf:Seq', {}).get('rdf:li', {})
//                    if items:
//                        if isinstance(items, string_types):
//                            return items
//                        return " ".join(items)
//                elif isinstance(t, int) or isinstance(t, float):
//                    return t

//    # From https://github.com/mapillary/OpenSfM/blob/master/opensfm/exif.py
//    def get_xmp(self, file):
//        img_bytes = file.read()
//        xmp_start = img_bytes.find(b'<x:xmpmeta')
//        xmp_end = img_bytes.find(b'</x:xmpmeta')
//
//        if xmp_start < xmp_end:
//            xmp_str = img_bytes[xmp_start:xmp_end + 12].decode('utf8')
//            try:
//                xdict = x2d.parse(xmp_str)
//            except ExpatError as e:
//                from bs4 import BeautifulSoup
//                xmp_str = str(BeautifulSoup(xmp_str, 'xml'))
//                xdict = x2d.parse(xmp_str)
//                log.ODM_WARNING("%s has malformed XMP XML (but we fixed it)" % self.filename)
//            xdict = xdict.get('x:xmpmeta', {})
//            xdict = xdict.get('rdf:RDF', {})
//            xdict = xdict.get('rdf:Description', {})
//            if isinstance(xdict, list):
//                return xdict
//            else:
//                return [xdict]
//        else:
//            return []

//    def float_values(self, tag):
//        if isinstance(tag.values, list):
//            result = []
//            for v in tag.values:
//                if isinstance(v, int):
//                    result.append(float(v))
//                elif isinstance(v, tuple) and len(v) == 1 and isinstance(v[0], float):
//                    result.append(v[0])
//                elif v.den != 0:
//                    result.append(float(v.num) / float(v.den))
//                else:
//                    result.append(None)
//            return result
//        elif hasattr(tag.values, 'den'):
//            return [float(tag.values.num) / float(tag.values.den) if tag.values.den != 0 else None]
//        else:
//            return [None]

//    def float_value(self, tag):
//        v = self.float_values(tag)
//        if len(v) > 0:
//            return v[0]

//    def int_values(self, tag):
//        if isinstance(tag.values, list):
//            return [int(v) for v in tag.values]
//        elif isinstance(tag.values, str) and tag.values == '':
//            return []
//        else:
//            return [int(tag.values)]

//    def int_value(self, tag):
//        v = self.int_values(tag)
//        if len(v) > 0:
//            return v[0]

//    def list_values(self, tag):
//        return " ".join(map(str, tag.values))

//    def try_parse_fraction(self, val):
//        parts = val.split("/")
//        if len(parts) == 2:
//            try:
//                num, den = map(float, parts)
//                return num / den if den != 0 else val
//            except ValueError:
//                pass
//        return val

//    def get_radiometric_calibration(self):
//        if isinstance(self.radiometric_calibration, str):
//            parts = self.radiometric_calibration.split(" ")
//            if len(parts) == 3:
//                return list(map(float, parts))
//
//        return [None, None, None]

//    def get_dark_level(self):
//        if self.black_level:
//            levels = np.array([float(v) for v in self.black_level.split(" ")])
//            return levels.mean()

//    def get_gain(self):
//        if self.gain is not None:
//            return self.gain
//        elif self.iso_speed:
//            #(gain = ISO/100)
//            return self.iso_speed / 100.0

//    def get_vignetting_center(self):
//        if self.vignetting_center:
//            parts = self.vignetting_center.split(" ")
//            if len(parts) == 2:
//                return list(map(float, parts))
//        return [None, None]

//    def get_vignetting_polynomial(self):
//        if self.vignetting_polynomial:
//            parts = self.vignetting_polynomial.split(" ")
//            if len(parts) > 0:
//                coeffs = list(map(float, parts))
//
//                # Different camera vendors seem to use different ordering for the coefficients
//                if self.camera_make != "Sentera":
//                    coeffs.reverse()
//
//                return coeffs

//    def get_utc_time(self):
//        if self.utc_time:
//            return datetime.fromtimestamp(self.utc_time / 1000, timezone.utc)
//
//    def get_photometric_exposure(self):
//        # H ~= (exposure_time) / (f_number^2)
//        if self.fnumber is not None and self.exposure_time is not None and self.exposure_time > 0 and self.fnumber > 0:
//            return self.exposure_time / (self.fnumber * self.fnumber)
//
//    def get_horizontal_irradiance(self):
//        if self.horizontal_irradiance is not None:
//            scale = 1.0 # Assumed
//            if self.irradiance_scale_to_si is not None:
//                scale = self.irradiance_scale_to_si
//
//            return self.horizontal_irradiance * scale
//        elif self.camera_make == "DJI" and self.spectral_irradiance is not None:
//            # Phantom 4 Multispectral saves this value in @drone-dji:Irradiance
//            return self.spectral_irradiance
//
//    def get_sun_sensor(self):
//        if self.sun_sensor is not None:
//            # TODO: Presence of XMP:SunSensorExposureTime
//            # and XMP:SunSensorSensitivity might
//            # require additional logic. If these two tags are present,
//            # then sun_sensor is not in physical units?
//            return self.sun_sensor / 65535.0 # normalize uint16 (is this correct?)
//        elif self.spectral_irradiance is not None:
//            scale = 1.0 # Assumed
//            if self.irradiance_scale_to_si is not None:
//                scale = self.irradiance_scale_to_si
//
//            return self.spectral_irradiance * scale
//
//    def get_dls_pose(self):
//        if self.dls_yaw is not None:
//            return [self.dls_yaw, self.dls_pitch, self.dls_roll]
//        return [0.0, 0.0, 0.0]
//
//    def get_bit_depth_max(self):
//        if self.bits_per_sample:
//            return float(2 ** self.bits_per_sample)
//        else:
//            # If it's a JPEG, this must be 256
//            _, ext = os.path.splitext(self.filename)
//            if ext.lower() in [".jpeg", ".jpg"]:
//                return 256.0
//
//        return None
//
//    def get_capture_id(self):
//        # Use capture UUID first, capture time as fallback
//        if self.capture_uuid is not None:
//            return self.capture_uuid
//
//        return self.get_utc_time()
//
//    def get_gps_dop(self):
//        val = -9999
//        if self.gps_xy_stddev is not None:
//            val = self.gps_xy_stddev
//        if self.gps_z_stddev is not None:
//            val = max(val, self.gps_z_stddev)
//        if val > 0:
//            return val
//
//        return None
//
//    def override_gps_dop(self, dop):
//        self.gps_xy_stddev = self.gps_z_stddev = dop
//
//    def override_camera_projection(self, camera_projection):
//        if camera_projection in projections:
//            self.camera_projection = camera_projection
//
//    def camera_id(self):
//        return " ".join(
//                [
//                    "v2",
//                    self.camera_make.strip(),
//                    self.camera_model.strip(),
//                    str(int(self.width)),
//                    str(int(self.height)),
//                    self.camera_projection,
//                    str(float(self.focal_ratio))[:6],
//                ]
//            ).lower()

//    def to_opensfm_exif(self, rolling_shutter = False, rolling_shutter_readout = 0):
//        capture_time = 0.0
//        if self.utc_time is not None:
//            capture_time = self.utc_time / 1000.0
//
//        gps = {}
//        has_gps = self.latitude is not None and self.longitude is not None
//        if has_gps:
//            gps['latitude'] = self.latitude
//            gps['longitude'] = self.longitude
//            if self.altitude is not None:
//                gps['altitude'] = self.altitude
//            else:
//                gps['altitude'] = 0.0
//
//            dop = self.get_gps_dop()
//            if dop is None:
//                dop = 10.0 # Default
//
//            gps['dop'] = dop
//
//        d = {
//            "make": self.camera_make,
//            "model": self.camera_model,
//            "width": self.width,
//            "height": self.height,
//            "projection_type": self.camera_projection,
//            "focal_ratio": self.focal_ratio,
//            "orientation": self.orientation,
//            "capture_time": capture_time,
//            "gps": gps,
//            "camera": self.camera_id()
//        }
//
//        if self.has_opk():
//            d['opk'] = {
//                'omega': self.omega,
//                'phi': self.phi,
//                'kappa': self.kappa
//            }
//
//        # Speed is not useful without GPS
//        if self.has_speed() and has_gps:
//            d['speed'] = [self.speed_y, self.speed_x, self.speed_z]
//
//        if rolling_shutter:
//            d['rolling_shutter'] = get_rolling_shutter_readout(self, rolling_shutter_readout)
//
//        return d

//    def has_ypr(self):
//        return self.yaw is not None and \
//            self.pitch is not None and \
//            self.roll is not None

//    def has_opk(self):
//        return self.omega is not None and \
//            self.phi is not None and \
//            self.kappa is not None

//    def has_speed(self):
//        return self.speed_x is not None and \
//                self.speed_y is not None and \
//                self.speed_z is not None

//    def has_geo(self):
//        return self.latitude is not None and \
//            self.longitude is not None

void Photo::compute_opk()
{
    // OPK = Omega/Phi/Kappa angles 
    //       orientation of sensor (vs yaw/pitch/roll, orientation of aircraft)

    if (has_ypr() && has_geo())
    {
        //y, p, r = math.radians(self.yaw), math.radians(self.pitch), math.radians(self.roll)
    
        //# Ref: New Calibration and Computing Method for Direct
        //# Georeferencing of Image and Scanner Data Using the
        //# Position and Angular Data of an Hybrid Inertial Navigation System
        //# by Manfred Bäumker

        //# YPR rotation matrix
        //cnb = np.array([[ math.cos(y) * math.cos(p), math.cos(y) * math.sin(p) * math.sin(r) - math.sin(y) * math.cos(r), math.cos(y) * math.sin(p) * math.cos(r) + math.sin(y) * math.sin(r)],
        //                [ math.sin(y) * math.cos(p), math.sin(y) * math.sin(p) * math.sin(r) + math.cos(y) * math.cos(r), math.sin(y) * math.sin(p) * math.cos(r) - math.cos(y) * math.sin(r)],
        //                [ -math.sin(p), math.cos(p) * math.sin(r), math.cos(p) * math.cos(r)],
        //                ])

        //# Convert between image and body coordinates
        //# Top of image pixels point to flying direction
        //# and camera is looking down.
        //# We might need to change this if we want different
        //# camera mount orientations (e.g. backward or sideways)

        //# (Swap X/Y, flip Z)
        //cbb = np.array([[0, 1, 0],
        //                [1, 0, 0],
        //                [0, 0, -1]])

        //delta = 1e-7

        //alt = self.altitude if self.altitude is not None else 0.0
        //p1 = np.array(ecef_from_lla(self.latitude + delta, self.longitude, alt))
        //p2 = np.array(ecef_from_lla(self.latitude - delta, self.longitude, alt))
        //xnp = p1 - p2
        //m = np.linalg.norm(xnp)

        //if m == 0:
        //    log.ODM_WARNING("Cannot compute OPK angles, divider = 0")
        //    return

        //# Unit vector pointing north
        //xnp /= m

        //znp = np.array([0, 0, -1]).T
        //ynp = np.cross(znp, xnp)

        //cen = np.array([xnp, ynp, znp]).T

        //# OPK rotation matrix
        //ceb = cen.dot(cnb).dot(cbb)

        //self.omega = math.degrees(math.atan2(-ceb[1][2], ceb[2][2]))
        //self.phi = math.degrees(math.asin(ceb[0][2]))
        //self.kappa = math.degrees(math.atan2(-ceb[0][1], ceb[0][0]))
    }
}

//    def get_capture_megapixels(self):
//        if self.exif_width is not None and self.exif_height is not None:
//            # Accurate so long as resizing / postprocess software
//            # did not fiddle with the tags
//            return self.exif_width * self.exif_height / 1e6
//        elif self.width is not None and self.height is not None:
//            # Fallback, might not be accurate since the image
//            # could have been resized
//            return self.width * self.height / 1e6
//        else:
//            return 0.0

double Photo::CalcFocalRatio(easyexif::EXIFInfo exif)
{
    double focal_ratio = 0.0;

    double sensor_width = 0.0;
    if (exif.LensInfo.FocalPlaneResolutionUnit > 0 && exif.LensInfo.FocalPlaneXResolution > 0.0)
    {
        int resolution_unit = exif.LensInfo.FocalPlaneResolutionUnit;
        double mm_per_unit = 0.0;
        if (resolution_unit == 2)		// inch
            mm_per_unit = 25.4;
        else if (resolution_unit == 3)		// cm
            mm_per_unit = 10;
        else if (resolution_unit == 4)		// mm
            mm_per_unit = 1;
        else if (resolution_unit == 5)		// um
            mm_per_unit = 0.001;
        else
        {
            Logger::Write(__FUNCTION__, "Unknown EXIF resolution unit: %d", resolution_unit);
            assert(false);
            return focal_ratio;
        }

        if (mm_per_unit > 0.0)
        {
            double pixels_per_unit = exif.LensInfo.FocalPlaneXResolution;
            if (pixels_per_unit <= 0.0 && exif.LensInfo.FocalPlaneYResolution > 0.0)
                pixels_per_unit = exif.LensInfo.FocalPlaneYResolution;

            if ((pixels_per_unit > 0.0) && (exif.ImageWidth > 0))
            {
                double units_per_pixel = 1.0 / pixels_per_unit;
                sensor_width = (double)exif.ImageWidth * units_per_pixel * mm_per_unit;
            }
        }
    }

    //focal_35 = None
    double focal;
    //if "EXIF FocalLengthIn35mmFilm" in tags:
    //	focal_35 = self.float_value(tags["EXIF FocalLengthIn35mmFilm"])
    if (exif.FocalLength > 0.0)
        focal = exif.FocalLength;
    //if focal is None and "@aux:Lens" in xtags:
    //	lens = self.get_xmp_tag(xtags, ["@aux:Lens"])
    //	matches = re.search('([\d\.]+)mm', str(lens))
    //	if matches:
    //		focal = float(matches.group(1))

    //if focal_35 is not None and focal_35 > 0:
    //	focal_ratio = focal_35 / 36.0  # 35mm film produces 36x24mm pictures.
    //else:
    //	if not sensor_width:
    //		sensor_width = sensor_data().get(sensor_string, None)
    if ((sensor_width > 0.0) && (focal > 0.0))
        focal_ratio = focal / sensor_width;
    else
        focal_ratio = 0.85;

    return focal_ratio;
}

int Photo::yisleap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int Photo::get_yday(int mon, int day, int year)
{
    static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };
    int leap = yisleap(year);

    return days[leap][mon] + day;
}

__int64 Photo::CalcUnixEpoch(XString dateTime)
{
    // Return seconds since Jan 1, 1950.
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

XString Photo::GetMake()
{
    return m_camera_make;
}

XString Photo::GetModel()
{
    return m_camera_model;
}

int Photo::GetWidth()
{
    return m_width;
}

int Photo::GetHeight()
{
    return m_height;
}

double Photo::GetLatitude()
{
    return m_gps_lat;
}

double Photo::GetLongitude()
{
    return m_gps_lon;
}

double Photo::GetAltitude()
{
    return m_gps_alt;
}

double Photo::GetExposureTime()
{
    return m_exposure_time;
}

int Photo::GetIsoSpeed()
{
    return m_iso_speed;
}

int Photo::GetBitsPerSample()
{
    return m_bits_per_sample;
}

XString Photo::GetDateTime()
{
    return m_date_time;
}

__int64 Photo::GetEpoch()
{
    return m_epoch;
}

double Photo::GetFocalRatio()
{
    return m_focal_ratio;
}

double Photo::GetFocalLength()
{
    return m_focal_length;
}

bool Photo::is_thermal()
{
    // Added for support M2EA camera sensor
    if ((m_camera_make == "DJI") && (m_camera_model == "MAVIC2-ENTERPRISE-ADVANCED") && (m_width == 640 && m_height == 512))
        return true;
    // Added for support DJI H20T camera sensor
    if ((m_camera_make == "DJI") && (m_camera_model == "ZH20T") && (m_width == 640 && m_height == 512))
        return true;
    //return self.band_name.upper() in ["LWIR"] # TODO: more?
    return false;
}

bool Photo::is_rgb()
{
    //        return self.band_name.upper() in ["RGB", "REDGREENBLUE"]
    return true;
}

bool Photo::has_geo()
{
    return m_has_geo;
}

bool Photo::has_ypr()
{
    return m_has_ypr;
}

bool Photo::has_speed()
{
    return m_has_speed;
}

XString Photo::GetCameraStrOSFM()
{
    return m_camera_str_osfm;
}

XString Photo::GetCameraStrODM()
{
    return m_camera_str_odm;
}
