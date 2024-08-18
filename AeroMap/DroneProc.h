#ifndef DRONEPROC_H
#define DRONEPROC_H

#include <QObject>
#include <QThread>

// stages
#include "StageDataset.h"		// load dataset
#include "StageSFM.h"			// structure from motion
#include "StageMVS.h"			// multi view stereo
#include "StageFilter.h"		// point filtering
#include "StageMesh.h"			// generate meshes
#include "StageTexture.h"		// apply textures
#include "StageGeoref.h"		// georeference point cloud
#include "StageDEM.h"			// generate DTM/DSM
#include "StageOrtho.h"			// generate orthophoto
#include "StageReport.h"		// generate pdf report
	
class DroneProc : public QObject
{
	Q_OBJECT

public:
	DroneProc(QObject* parent);
	~DroneProc();

	int Run(Stage::Id initStage);

private:

	StageDataset* mp_StageDataset;
	StageSFM* mp_StageSFM;
	StageMVS* mp_StageMVS;
	StageFilter* mp_StageFilter;
	StageMesh* mp_StageMesh;
	StageTexture* mp_StageTexture;
	StageGeoref* mp_StageGeoref;
	StageDEM* mp_StageDEM;
	StageOrtho* mp_StageOrtho;
	StageReport* mp_StageReport;

private:

	void PreprocessParameters();

public slots:

	//void RunAsync(Stage::Id initStage);

signals:

	//void sig_stage_finished(int stage_id);		// emitted after each stage completes
	//void sig_proc_finished();						// emitted after entire process complete
};

#endif // #ifndef DRONEPROC_H
