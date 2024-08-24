#ifndef STAGEDEM_H
#define STAGEDEM_H

#include "Stage.h"

class StageDEM : Stage
{
public:

	virtual int Run() override;

private:

	void CreateTerrainModel(XString input_geotiff);
};

#endif // #ifndef STAGEDEM_H
