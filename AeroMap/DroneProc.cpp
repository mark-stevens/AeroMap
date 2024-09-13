// DroneProc.cpp
// Drone photogrammetry pipeline.
//

#include <stdio.h>
#include <assert.h>

#include <QDir>
#include <QString>
#include <QProgressDialog>

#include "AeroMap.h"
#include "Logger.h"			// message logging facility
#include "DroneProc.h"

DroneProc::DroneProc(QObject* parent)
	: QObject(parent)
	, mp_StageDataset(new StageDataset())
	, mp_StageSFM(new StageSFM())
	, mp_StageMVS(new StageMVS())
	, mp_StageFilter(new StageFilter())
	, mp_StageMesh(new StageMesh())
	, mp_StageTexture(new StageTexture())
	, mp_StageGeoref(new StageGeoref())
	, mp_StageDEM(new StageDEM())
	, mp_StageOrtho(new StageOrtho())
	, mp_StageReport(new StageReport())
{
}

DroneProc::~DroneProc()
{
	delete mp_StageDataset;
	delete mp_StageSFM;
	delete mp_StageMVS;
	delete mp_StageFilter;
	delete mp_StageMesh;
	delete mp_StageTexture;
	delete mp_StageGeoref;
	delete mp_StageDEM;
	delete mp_StageOrtho;
	delete mp_StageReport;
}

int DroneProc::Run(Stage::Id initStage)
{
	int status = 0;

	if (GetProject().LoadImageList() == 0)
	{
		GetApp()->LogWrite("No images found.");
		return status;
	}

	AeroLib::InitRunLog();
	PreprocessParameters();

	// tried synchronous process dialog, but see that only async is worth doing
	// and for that really need drone pipeline to be async...
	// 
	//QProgressDialog progress("Processing drone imagery...", "Cancel", 0, 9, (QWidget*)this->parent());
	//// remove question mark from title bar
	//progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	//progress.setWindowModality(Qt::WindowModal);
	//progress.setValue(0);
	//progress.setMinimumDuration(0);

	if (initStage <= Stage::Id::Dataset)
		status = mp_StageDataset->Run();
	emit sig_stage_finished((int)Stage::Id::Dataset);

	if ((status == 0) && (initStage <= Stage::Id::OpenSFM))
		status = mp_StageSFM->Run();
	emit sig_stage_finished((int)Stage::Id::OpenSFM);

	if ((status == 0) && (initStage <= Stage::Id::OpenMVS))
		status = mp_StageMVS->Run();
	emit sig_stage_finished((int)Stage::Id::OpenMVS);

	if ((status == 0) && (initStage <= Stage::Id::Filter))
		status = mp_StageFilter->Run();
	emit sig_stage_finished((int)Stage::Id::Filter);

	if ((status == 0) && (initStage <= Stage::Id::Mesh))
		status = mp_StageMesh->Run();
	emit sig_stage_finished((int)Stage::Id::Mesh);

	if ((status == 0) && (initStage <= Stage::Id::Texture))
		status = mp_StageTexture->Run();
	emit sig_stage_finished((int)Stage::Id::Texture);

	if ((status == 0) && (initStage <= Stage::Id::Georeference))
		status = mp_StageGeoref->Run();
	emit sig_stage_finished((int)Stage::Id::Georeference);

	if ((status == 0) && (initStage <= Stage::Id::DEM))
		status = mp_StageDEM->Run();
	emit sig_stage_finished((int)Stage::Id::DEM);

	if ((status == 0) && (initStage <= Stage::Id::Orthophoto))
		status = mp_StageOrtho->Run();
	emit sig_stage_finished((int)Stage::Id::Orthophoto);

	if ((status == 0) && (initStage <= Stage::Id::Report))
		status = mp_StageReport->Run();
	emit sig_stage_finished((int)Stage::Id::Report);

	emit sig_drone_proc_finished(status);

	GetApp()->LogWrite("Drone image processing complete.");

	return status;
}

void DroneProc::PreprocessParameters()
{
	// Set parameters that must be set/unset based on other parameter values.
	// 

	if (arg.fast_orthophoto)
	{
		Logger::Write(__FUNCTION__, "Fast orthophoto is turned on, automatically setting --skip-3dmodel");
		arg.skip_3dmodel = true;
	}

	if (arg.pc_rectify && (arg.pc_classify == false))
	{
		Logger::Write(__FUNCTION__, "Ground rectify is turned on, automatically turning on point cloud classification");
		arg.pc_classify = true;
	}

	if (arg.dtm && (arg.pc_classify == false))
	{
		Logger::Write(__FUNCTION__, "DTM is turned on, automatically turning on point cloud classification");
		arg.pc_classify = true;
	}

	if (arg.skip_3dmodel && arg.use_3dmesh)
	{
		Logger::Write(__FUNCTION__, "--skip-3dmodel is set, but so is --use-3dmesh. --skip-3dmodel will be ignored.");
		arg.skip_3dmodel = false;
	}

	//if (arg.orthophoto_cutline && not arg.crop)
	{
		//  Logger::Write(__FUNCTION__, "--orthophoto-cutline is set, but --crop is not. --crop will be set to 0.01");
		//  arg.crop = 0.01
	}
}
