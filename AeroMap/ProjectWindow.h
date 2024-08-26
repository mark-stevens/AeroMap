#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include "AeroMap.h"        // application header
#include "LightWindow.h"

#include <QWidget>
#include <QTreeWidget>
#include <QGridLayout>
#include <QAction>

class ProjectWindow : public QWidget
{
	Q_OBJECT

public:

	explicit ProjectWindow(QWidget* parent = nullptr);
	~ProjectWindow();

	void SaveState();

public slots:

	void OnProjectChanged();

protected:

	void closeEvent(QCloseEvent* event) override;

private:

	enum class ItemType
	{
		None,

		ProjectRoot,		// root node for project

		DroneRoot,			// root node for drone photogrammetry
		DroneInput,			// location of drone photogrammetry inputs
		DroneOutput,		// root folder that will receive outputs
		DroneOutputDTM,
		DroneOutputDSM,
		DroneOutputOrtho,

		LidarRoot,			// root node for lidar workspace
		LidarFile,			// single file entry (las or laz)
	};

	QTreeWidget* mp_Tree;
	QTreeWidgetItem* mp_ItemRoot;			// root tree node
	QTreeWidgetItem* mp_ItemDroneRoot;
	QTreeWidgetItem* mp_ItemDroneInput;
	QTreeWidgetItem* mp_ItemDroneOutput;
	QTreeWidgetItem* mp_ItemLidarRoot;

	QGridLayout* mp_gridLayout;

	QAction* mp_actionActivate;			// activate selected view

	QAction* mp_actionAddLidarFile;		// add lidar file action
	QAction* mp_actionRemoveLidarFile;	// remove lidar file
	QAction* mp_actionOpenFolder;		// general open containing folder
	QAction* mp_actionProperties;		// item properties

	LightWindow* mp_LightWindow;

	bool m_ItemChangeMask;				// mask out change events

private slots:

	void OnItemClicked(QTreeWidgetItem* pItem, int column);
	void OnItemDoubleClicked(QTreeWidgetItem* pItem, int column);

	// context menu handlers
	void OnActivateView();
	void OnLoadFile();
	void OnAddLidarFile();
	void OnRemoveLidarFile();

	void OnProperties();
	void OnOpenFolder();

private:

	// QWidget
	void contextMenuEvent(QContextMenuEvent* event) override;

	void CreateActions();
	void DisplayProperties(QTreeWidgetItem* pItem);
	void DisplayLidarProperties(QTreeWidgetItem* pItem);
	void DisplayTerrainProperties(QTreeWidgetItem* pItem);
	XString SelectFolder();
	void UpdateUI();
};

#endif // #ifndef PROJECTWINDOW_H
