// DroneProcDlg.cpp
// Drone photogrammetry options.
//

#include "AeroMap.h"			// application header
#include "DroneProcDlg.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

DroneProcDlg::DroneProcDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	cboInitStage->clear();
	cboInitStage->addItem("Load Dataset", (int)Stage::Id::Dataset);
	cboInitStage->addItem("OpenSFM", (int)Stage::Id::OpenSFM);
	cboInitStage->addItem("OpenMVS", (int)Stage::Id::OpenMVS);
	cboInitStage->addItem("Filter Points", (int)Stage::Id::Filter);
	cboInitStage->addItem("Mesh", (int)Stage::Id::Mesh);
	cboInitStage->addItem("Texture", (int)Stage::Id::Texture);
	cboInitStage->addItem("Georeference", (int)Stage::Id::Georeference);
	cboInitStage->addItem("DEM", (int)Stage::Id::DEM);
	cboInitStage->addItem("Orthophoto", (int)Stage::Id::Orthophoto);
	cboInitStage->addItem("Report", (int)Stage::Id::Report);

	// load defaults
	spinCrop->setValue(arg.crop);
	spinDemResolution->setValue(arg.dem_resolution);
	spinOrthoResolution->setValue(arg.orthophoto_resolution);
	spinGapFill->setValue(arg.dem_gapfill_steps);
	chkDSM->setChecked(arg.dsm);
	chkDTM->setChecked(arg.dtm);

	// signals and slots
	verify_connect(cmdRun, SIGNAL(clicked()), this, SLOT(OnRun()));
	verify_connect(cmdCancel, SIGNAL(clicked()), this, SLOT(OnClose()));
}

DroneProcDlg::~DroneProcDlg()
{
}

Stage::Id DroneProcDlg::GetInitStage()
{
	return (Stage::Id)cboInitStage->currentData().toInt();
}

void DroneProcDlg::OnRun()
{
	arg.crop = spinCrop->value();
	arg.dem_resolution = spinDemResolution->value();
	arg.orthophoto_resolution = spinOrthoResolution->value();
	arg.dem_gapfill_steps = spinGapFill->value();
	arg.dsm = chkDSM->isChecked();
	arg.dtm = chkDTM->isChecked();

	accept();
}

void DroneProcDlg::OnClose()
{
    reject();
}
