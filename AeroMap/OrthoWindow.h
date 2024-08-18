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

	bool mb_Selecting;
	bool mb_DebugInfo;

	SizeType	m_winSize;			// client area dimensions
	PointType  	m_ptAnchor;			// anchor point for current operation
	PointType	m_ptLastMouse;		// last recorded mouse position
	RectType   	m_rctSel;			// current 2d selection

	QFont m_Font;

	QMainWindow* mp_Parent;

private:

	void CreateScene();

	void SetBackColor(UInt8 R, UInt8 G, UInt8 B);
	void GetBackColor(UInt8& R, UInt8& G, UInt8& B);
};

#endif // #ifndef ORTHOWINDOW_H
