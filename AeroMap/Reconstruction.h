#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "Photo.h"
#include "XString.h"

class Reconstruction
{
public:

	Reconstruction();
	Reconstruction(std::vector<Photo>& photos);
	~Reconstruction();

	bool is_multi_camera();

private:

	std::vector<Photo> m_photos;

	// 'band name' -> list of photos for that band
	struct band_type
	{
		XString name;					// "RGB", "Green", "Red", "Rededge", "NIR", etc.
		std::vector<Photo> photos;
	};
	std::vector<band_type> m_multi_camera;

private:

	void filter_photos();
	void detect_multi_camera();

	int get_band_index(XString band_name);
	band_type& get_band(int band_index);
};

#endif // #ifndef RECONSTRUCTION_H
