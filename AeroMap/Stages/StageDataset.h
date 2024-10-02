#ifndef STAGEDATASET_H
#define STAGEDATASET_H

#include "AeroMap.h"		// application header
#include "Stage.h"			// base class

class StageDataset : Stage
{
public:

	virtual int Run() override;

private:
	
	int InitGeoref();

	int WriteImageListText();
	int WriteImageListJson();
};

#endif // #ifndef STAGEDATASET_H
