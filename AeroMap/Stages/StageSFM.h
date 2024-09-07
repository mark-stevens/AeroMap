#ifndef STAGESFM_H
#define STAGESFM_H

#include "AeroMap.h"		// application header
#include "exif.h"			// easy exif header
#include "Stage.h"			// base class

class StageSFM : Stage
{
public:

	virtual int Run() override;

private:
	
	int Setup();

	int WriteExif();
	int WriteImageListText();
	int WriteCameraModelsJson();
	int WriteReferenceLLA();
	int WriteConfigYaml();
	int UpdateConfigYaml(XString key, int value);

	int WriteCamerasJson();
};

#endif //#ifndef STAGESFM_H
