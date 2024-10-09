#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "AeroMap.h"

class Reconstruction
{
public:

	Reconstruction(const std::vector<Photo>& photos);
	~Reconstruction();

private:

	const std::vector<Photo>& m_photos;

private:

	void filter_photos();
	void detect_multi_camera();
};

#endif // #ifndef RECONSTRUCTION_H
