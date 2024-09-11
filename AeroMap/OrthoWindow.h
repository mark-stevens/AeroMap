#ifndef ORTHOWINDOW_H
#define ORTHOWINDOW_H

#include "AeroMap.h"

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>

class OrthoWindow : public QGraphicsView
{
	Q_OBJECT

public:

	OrthoWindow(QWidget* parent = nullptr);
	~OrthoWindow();

	void ZoomIn();
	void ZoomOut();
	void ResetView();

protected:

	// QWidget
	virtual void contextMenuEvent(QContextMenuEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void showEvent(QShowEvent* event) override;

private:

	QPixmap m_pixmap;
	QGraphicsScene m_scene;		// backing scene
	std::vector<QGraphicsItem*> m_ItemList;

	bool mb_Selecting;
	bool mb_DebugInfo;

	SizeType	m_winSize;			// client area dimensions
	PointType  	m_ptAnchor;			// anchor point for current operation
	PointType	m_ptLastMouse;		// last recorded mouse position
	RectType   	m_rctSel;			// current 2d selection
	
	RectD	m_Extent;				// orthophoto extents
	bool	mb_HaveExt;				// m_Extent valid

	std::vector<VEC2> m_DistancePts;	// connected points defining distance measurement

	QFont m_Font;

	QMainWindow* mp_Parent;

	QAction* mp_actionClear;
	QAction* mp_actionProp;

private slots:

	void OnClear();
	void OnProp();

private:

	void CreateScene();
	void CreateActions();

	// measurement

	int		AddDistancePoint(VEC2 pos_m);
	int		AddDistancePoint(PointD pos_m);
	int		GetDistancePointCount() const;
	VEC2	GetDistancePoint(unsigned int index);
	double	GetDistance(unsigned int index);
	void	ClearDistancePoints();

	PointD PixelToMeters(int x, int y);
	PointType MetersToPixel(double x, double y);
	PointType MetersToImage(double x, double y);
};

#endif // #ifndef ORTHOWINDOW_H
