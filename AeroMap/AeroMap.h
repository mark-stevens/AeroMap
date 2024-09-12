#ifndef AEROMAP_H
#define AEROMAP_H

#include <assert.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include <GL/glew.h>

#include <QtGui>
#include <QApplication>
#include <QComboBox>

#include "Version.h"
#include "MarkTypes.h"
#include "MarkLib.h"
#include "XString.h"
#include "Logger.h"
#include "Calc.h"
#include "Gis.h"

#include "Project.h"
#include "Tool.h"
#include "OutputWindow.h"

#define APP_NAME "AeroMap"

#define ORG_KEY "GorgeGeo"
#define APP_KEY "AeroMap"

#define PATH_APPDATA_KEY		"AppDataPath"
#define STARTUP_STATE_KEY		"StartupState"
#define LAST_FILE_KEY			"LastFile"			// most recently loaded project file

#define CONTOUR_INTERVAL_KEY	"ContourInterval"
#define CONTOUR_START_KEY		"ContourStart"
#define CONTOUR_COLOR_KEY		"ContourColor"

#define LAST_TERRAIN_SCALE_KEY	"TerrainScale"
#define LAST_LIDAR_SCALE_KEY	"LidarScale"

#define SCENE_BACK_COLOR QColor(0xF0, 0xF0, 0xFF)	// default background color for graphics scenes

// connection macro for the following overload that:
//		1. asserts on failure
//		2. logs failure location
//
//static QMetaObject::Connection connect(const QObject *sender, const QMetaMethod &signal,
//	const QObject *receiver, const QMetaMethod &method,
//	Qt::ConnectionType type = Qt::AutoConnection);

#define verify_connect(parm_sender, parm_signal, parm_receiver, parm_method) \
{ \
	QMetaObject::Connection cn = connect(parm_sender, parm_signal, parm_receiver, parm_method); \
	if (cn == nullptr ) \
	{ \
		Logger::Write(__FUNCTION__, "Qt connection failed @ line %d.", __LINE__); \
		assert(false); \
	} \
}

class MainWindow;

class AeroMap : public QApplication
{
	Q_OBJECT

public:

	enum class ViewType
	{
		None,
		Drone,			// drone photogrammetry workspace
		DroneOrtho,		// orthophoto window
		Lidar,			// lidar window
		Terrain			// terrain model window
	};

	enum class OutputItem		// ids for individual outputs
	{
		None,
		Ortho,			// orthophoto
		DTM,			// digital terrain model
		DSM				// digital surface model
	};

	Tool m_Tool;		// active tool

public:

	AeroMap(int& argc, char** argv);
	~AeroMap();

	XString GetAppDataPath() { return ms_AppDataPath; }
	XString GetOdmLibPath() { return ms_OdmLibPath; }

	MainWindow* GetMainWindow();

	Project& GetProject() { return m_Project; }

	void ActivateView(AeroMap::ViewType view, int subItem = -1);
	ViewType GetActiveView();

	void SetOutputWindow(OutputWindow* pOutputWindow) { mp_OutputWindow = pOutputWindow; }
	OutputWindow* GetOutputWindow() { return mp_OutputWindow; }

	XString GetEnvValue(const char* envKey);
	void LogWrite(const char* text, ...);

	void DumpRaster(GDALDataset* pDS, const char* fileName);
	void DumpVector(GDALDataset* pDS, const char* fileName);

	static void SetComboByUserData(QComboBox* cbo, int userData);

signals:

	void view_change_sig(AeroMap::ViewType, int);	// emitted when active view changes, optional subitem to activate

private:

	XString ms_AppDataPath;				// location of application data
	XString ms_OdmLibPath;

	Project m_Project;					// active project
	ViewType m_ViewType;
	OutputWindow* mp_OutputWindow;

private:

	void LoadSettings();
	XString ResolveAppDataPath();
	XString ResolveOdmLibPath(int& argc, char** argv);
};

inline AeroMap* GetApp()
{
	return static_cast<AeroMap*>(qApp);
}

inline Project& GetProject()
{ 
	return GetApp()->GetProject();
}

#endif // #ifndef AEROMAP_H
