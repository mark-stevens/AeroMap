// Reconstruction.cpp
// Port of odm reconstruction class.
//

#include "Logger.h"
#include "Reconstruction.h"

Reconstruction::Reconstruction()
{
	m_photos.clear();
	m_multi_camera.clear();
}

Reconstruction::Reconstruction(std::vector<Photo>& photos)
	: m_photos(photos)
{
	//class ODM_Reconstruction(object):
	//def __init__(self, photos):
	//    m_photos = photos
	//    self.georef = None
	//    self.gcp = None
	//    self.multi_camera = self.detect_multi_camera()
	//    self.filter_photos()

	detect_multi_camera();
}

Reconstruction::~Reconstruction()
{

}

void Reconstruction::detect_multi_camera()
{
	// Looks at the reconstruction photos and determines if this
	// is a single or multi-camera setup.
	//
	// --> this is where we build the 'multi_camera' data structure
	//
	//    band_photos = {}
	//    band_indexes = {}

	m_multi_camera.clear();

	for (auto p : m_photos)
	{
		XString band = p.GetBandName();

		// if entry doesn't exist, create it
		int band_index = get_band_index(band);
		if (band_index == -1)
		{
			band_type b;
			b.name = band;
			m_multi_camera.push_back(b);
			band_index = (int)m_multi_camera.size() - 1;
		}

	//        if not p.band_name in band_photos:
	//            band_photos[p.band_name] = []
	//        if not p.band_name in band_indexes:
	//            band_indexes[p.band_name] = str(p.band_index)
	
		// add photo to this band's list
		m_multi_camera[band_index].photos.push_back(p);
	}

	int bands_count = (int)m_multi_camera.size();
	if (bands_count >= 2 && bands_count <= 8)
	{
		// Validate that all bands have the same number of images,
		// otherwise this is not a multi-camera setup
		
		int img_per_band = (int)m_multi_camera[0].photos.size();
		for (auto band : m_multi_camera)
		{
			if (img_per_band != band.photos.size())
			{
				Logger::Write(__FUNCTION__, "Multi-camera setup detected, but band '%s' has only %d images (instead of %d).",
					band.name.c_str(), band.photos.size(), img_per_band);
				assert(false);
			}
		}

	//        mc = []
	//        for band_name in band_indexes:
	//            mc.append({'name': band_name, 'photos': band_photos[band_name]})
	//
	//        # Sort by band index
	//        mc.sort(key=lambda x: band_indexes[x['name']])
	//
	//        return mc
	}
	//    return None
}

void Reconstruction::filter_photos()
{
	//    if not self.multi_camera:
//        return # Nothing to do, use all images
//
//    else:
//        # Sometimes people might try process both RGB + Blue/Red/Green bands
//        # because these are the contents of the SD card from a drone (e.g. DJI P4 Multispectral)
//        # But we don't want to process both, so we discard the RGB files in favor
//        bands = {}
//        for b in self.multi_camera:
//            bands[b['name'].lower()] = b['name']
//
//        bands_to_remove = []
//
//        if 'rgb' in bands or 'redgreenblue' in bands:
//            if 'red' in bands and 'green' in bands and 'blue' in bands:
//                bands_to_remove.append(bands['rgb'] if 'rgb' in bands else bands['redgreenblue'])
//            else:
//                for b in ['red', 'green', 'blue']:
//                    if b in bands:
//                        bands_to_remove.append(bands[b])
//
//        if len(bands_to_remove) > 0:
//            log.ODM_WARNING("Redundant bands detected, probably because RGB images are mixed with single band images. We will trim some bands as needed")
//
//            for band_to_remove in bands_to_remove:
//                self.multi_camera = [b for b in self.multi_camera if b['name'] != band_to_remove]
//                photos_before = len(m_photos)
//                m_photos = [p for p in m_photos if p.band_name != band_to_remove]
//                photos_after = len(m_photos)
//
//                log.ODM_WARNING("Skipping %s band (%s images)" % (band_to_remove, photos_before - photos_after))
}

//def is_georeferenced(self):
//    return self.georef is not None

//def has_gcp(self):
//    return self.is_georeferenced() and self.gcp is not None and self.gcp.exists()

//def has_geotagged_photos(self):
//    for photo in m_photos:
//        if photo.latitude is None and photo.longitude is None:
//            return False
//
//    return True

//def georeference_with_gcp(self, gcp_file, output_coords_file, output_gcp_file, output_model_txt_geo, rerun=False):
//    if not io.file_exists(output_coords_file) or not io.file_exists(output_gcp_file) or rerun:
//        gcp = GCPFile(gcp_file)
//        if gcp.exists():
//            if gcp.entries_count() == 0:
//                raise RuntimeError("This GCP file does not have any entries. Are the entries entered in the proper format?")
//
//            gcp.check_entries()
//
//            # Convert GCP file to a UTM projection since the rest of the pipeline
//            # does not handle other SRS well.
//            rejected_entries = []
//            utm_gcp = GCPFile(gcp.create_utm_copy(output_gcp_file, filenames=[p.filename for p in m_photos], rejected_entries=rejected_entries, include_extras=True))
//
//            if not utm_gcp.exists():
//                raise RuntimeError("Could not project GCP file to UTM. Please double check your GCP file for mistakes.")
//
//            for re in rejected_entries:
//                log.ODM_WARNING("GCP line ignored (image not found): %s" % str(re))
//
//            if utm_gcp.entries_count() > 0:
//                log.ODM_INFO("%s GCP points will be used for georeferencing" % utm_gcp.entries_count())
//            else:
//                raise RuntimeError("A GCP file was provided, but no valid GCP entries could be used. Note that the GCP file is case sensitive (\".JPG\" is not the same as \".jpg\").")
//
//            self.gcp = utm_gcp
//
//            # Compute RTC offsets from GCP points
//            x_pos = [p.x for p in utm_gcp.iter_entries()]
//            y_pos = [p.y for p in utm_gcp.iter_entries()]
//            x_off, y_off = int(np.round(np.mean(x_pos))), int(np.round(np.mean(y_pos)))
//
//            # Create coords file, we'll be using this later
//            # during georeferencing
//            with open(output_coords_file, 'w') as f:
//                coords_header = gcp.wgs84_utm_zone()
//                f.write(coords_header + "\n")
//                f.write("{} {}\n".format(x_off, y_off))
//                log.ODM_INFO("Generated coords file from GCP: %s" % coords_header)
//
//            # Deprecated: This is mostly for backward compatibility and should be
//            # be removed at some point
//            shutil.copyfile(output_coords_file, output_model_txt_geo)
//            log.ODM_INFO("Wrote %s" % output_model_txt_geo)
//        else:
//            log.ODM_WARNING("GCP file does not exist: %s" % gcp_file)
//            return
//    else:
//        log.ODM_INFO("Coordinates file already exist: %s" % output_coords_file)
//        log.ODM_INFO("GCP file already exist: %s" % output_gcp_file)
//        self.gcp = GCPFile(output_gcp_file)
//
//    self.georef = ODM_GeoRef.FromCoordsFile(output_coords_file)
//    return self.georef

//def georeference_with_gps(self, images_path, output_coords_file, output_model_txt_geo, rerun=False):
//    try:
//        if not io.file_exists(output_coords_file) or rerun:
//            location.extract_utm_coords(m_photos, images_path, output_coords_file)
//        else:
//            log.ODM_INFO("Coordinates file already exist: %s" % output_coords_file)
//
//        # Deprecated: This is mostly for backward compatibility and should be
//        # be removed at some point
//        if not io.file_exists(output_model_txt_geo) or rerun:
//            with open(output_coords_file, 'r') as f:
//                with open(output_model_txt_geo, 'w+') as w:
//                    w.write(f.readline()) # CRS
//                    w.write(f.readline()) # Offset
//        else:
//            log.ODM_INFO("Model geo file already exist: %s" % output_model_txt_geo)
//
//        self.georef = ODM_GeoRef.FromCoordsFile(output_coords_file)
//    except:
//        log.ODM_WARNING('Could not generate coordinates file. The orthophoto will not be georeferenced.')
//
//    self.gcp = GCPFile(None)
//    return self.georef

//def save_proj_srs(self, file):
//    # Save proj to file for future use (unless this
//    # dataset is not georeferenced)
//    if self.is_georeferenced():
//        with open(file, 'w') as f:
//            f.write(self.get_proj_srs())

//def get_proj_srs(self):
//    if self.is_georeferenced():
//        return self.georef.proj4()

//def get_proj_offset(self):
//    if self.is_georeferenced():
//        return (self.georef.utm_east_offset, self.georef.utm_north_offset)
//    else:
//        return (None, None)

//def get_photo(self, filename):
//    for p in m_photos:
//        if p.filename == filename:
//            return p

int Reconstruction::get_band_index(XString band_name)
{
	int index = -1;

	for (int i = 0; i < m_multi_camera.size(); ++i)
	{
		if (band_name.CompareNoCase(m_multi_camera[i].name))
		{
			index = i;
			break;
		}
	}

	return index;
}

Reconstruction::band_type& Reconstruction::get_band(int band_index)
{
	return m_multi_camera[band_index];
}

bool Reconstruction::is_multi_camera()
{
	return m_multi_camera.size() > 1;
}
