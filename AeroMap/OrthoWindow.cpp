// OrthoWindow.cpp
// Orthophoto window.
//

#include "TextFile.h"
#include "MetaDataDlg.h"
#include "MainWindow.h"
#include "OrthoWindow.h"

#include <QStatusBar>
#include <QGraphicsLineItem>

OrthoWindow::OrthoWindow(QWidget* parent)
	: QGraphicsView(parent)
	, mb_Selecting(false)
	, mb_DebugInfo(true)
	, mb_HaveExt(false)
	, mp_actionClear(nullptr)
	, mp_actionProp(nullptr)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setMinimumSize(90, 90);

	mp_Parent = static_cast<MainWindow*>(parent);
	assert(mp_Parent != nullptr);

	setMouseTracking(true);
	setAcceptDrops(true);
	grabKeyboard();

	CreateActions();

	// set up fonts

	m_Font.setFamily("Consolas");
	m_Font.setPointSize(10.0);
	m_Font.setItalic(false);

	CreateScene();
}

OrthoWindow::~OrthoWindow()
{
	delete mp_actionClear;
	delete mp_actionProp;

	m_scene.clear();
}

void OrthoWindow::CreateScene()
{
	// Recreate entire scene.
	//

	mb_HaveExt = false;

	if (QFile::exists(tree.odm_orthophoto_tif.c_str()) == false)
		return;

	if (m_pixmap.width() == 0)
	{
		if (m_pixmap.load(tree.odm_orthophoto_tif.c_str()) == false)
		{
			Logger::Write(__FUNCTION__, "Unable to load orthophoto file: '%s'", tree.odm_orthophoto_tif.c_str());
			return;
		}
	}

	GetApp()->LogWrite("Loading orthophoto...");

	// these coordinate, in meters, are relative to the center of the scene, not utm coords
	if (QFile::exists(tree.odm_orthophoto_corners.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "Unable to load orthophoto extents: '%s'", tree.odm_orthophoto_corners.c_str());
	}
	else
	{
		TextFile textFile(tree.odm_orthophoto_corners.c_str());
		XString line = textFile.GetLine(0).c_str();
		int token_count = line.Tokenize(" \t");
		if (token_count < 4)
		{
			Logger::Write(__FUNCTION__, "Unable to parse orthophoto extents: '%s'", line.c_str());
		}
		else
		{
			m_Extent.x0 = line.GetToken(0).GetDouble();
			m_Extent.x1 = line.GetToken(2).GetDouble();
			m_Extent.y0 = line.GetToken(1).GetDouble();
			m_Extent.y1 = line.GetToken(3).GetDouble();

			mb_HaveExt = true;
		}
	}

	m_scene.clear();

	QBrush brushBack(SCENE_BACK_COLOR, Qt::SolidPattern);
	m_scene.setBackgroundBrush(brushBack);

	m_scene.addPixmap(m_pixmap);

	// make scene exactly same size as image
	m_scene.setSceneRect(0.0, 0.0, m_pixmap.width(), m_pixmap.height());

	setScene(&m_scene);
	setMouseTracking(true);

	GetApp()->LogWrite("Orthophoto loaded");
}

void OrthoWindow::CreateActions()
{
	mp_actionClear = new QAction(QIcon(""), tr("Clear"), this);
	mp_actionClear->setStatusTip(tr("Clear edit artifacts"));
	connect(mp_actionClear, SIGNAL(triggered()), this, SLOT(OnClear()));

	mp_actionProp = new QAction(QIcon(""), tr("Properties"), this);
	mp_actionProp->setStatusTip(tr("Show properties"));
	connect(mp_actionProp, SIGNAL(triggered()), this, SLOT(OnProp()));
}

void OrthoWindow::showEvent(QShowEvent* event)
{
	// Triggered when we first show 
	// the widget.
	//

	Q_UNUSED(event);

	// for some reason, can get multiple 'show' events;
	// of course we only want to scale it once
	static bool scaled = false;
	if (scaled == false)
	{
		// scale to window size initially

		m_winSize.cx = rect().width();
		m_winSize.cy = rect().height();
		double sf = (double)m_winSize.cx / (double)m_pixmap.width();
		if (sf < 1.0)
			scale(sf, sf);

		scaled = true;
	}
}

void OrthoWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive mouse 
	// double click events for the widget.
	// The default implementation generates a normal mouse press event.
	// Note: The widget will also receive mouse press and mouse release events in addition to 
	// the double click event. It is up to the developer to ensure that the application interprets 
	// these events correctly.

	Q_UNUSED(event);

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Distance:
		// basic way to stop measuring
		GetApp()->m_Tool.Clear();
		break;
	case Tool::ToolType::Area:
		GetApp()->m_Tool.Clear();
		break;
	default:
		break;
	}

	update();
}

void OrthoWindow::mousePressEvent(QMouseEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive mouse press events 
	// for the widget.
	// 
	// If you create new widgets in the mousePressEvent() the mouseReleaseEvent() may not 
	// end up where you expect, depending on the underlying window system (or X11 window manager), 
	// the widgets' location and maybe more.
	// 
	// The default implementation implements the closing of popup widgets when you click outside 
	// the window. For other widget types it does nothing.
	//

	PointD pt_m = PixelToMeters(event->x(), event->y());

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Select:
	case Tool::ToolType::ViewZoom:
	case Tool::ToolType::ViewPan:
		m_ptAnchor.x = event->x();
		m_ptAnchor.y = event->y();
		break;
	case Tool::ToolType::Distance:
		AddDistancePoint(pt_m);
		break;
	case Tool::ToolType::Area:
		// start/stop area rect
		//if (m_rctArea.x0 < 0)
		//{
		//	m_rctArea.x0 = event->x();
		//	m_rctArea.y0 = event->y();
		//	m_rctArea.x1 = event->x();
		//	m_rctArea.y1 = event->y();
		//}
		//else
		//{
		//	m_rctArea.x0 = -1;
		//}
		break;
	case Tool::ToolType::Volume:
		break;
	default:   // no active tool
		break;
	}

	update();

	m_ptLastMouse.x = event->x();  // last recorded mouse position
	m_ptLastMouse.y = event->y();

	XString str = XString::Format("Meters: %0.1f, %0.1f", pt_m.x, pt_m.y);
	mp_Parent->statusBar()->showMessage(str.c_str());
}

void OrthoWindow::mouseReleaseEvent(QMouseEvent* /* event */)
{
	// This event handler can be reimplemented in a subclass to receive 
	// mouse release events for the widget.
	//

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Select:   // finished selecting
		break;
	default:
		break;
	}

	update();
}

void OrthoWindow::mouseMoveEvent(QMouseEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive mouse move 
	// events for the widget.
	// 
	// If mouse tracking is switched off, mouse move events only occur if a mouse button
	// is pressed while the mouse is being moved. If mouse tracking is switched on, mouse 
	// move events occur even if no mouse button is pressed.
	// 
	// QMouseEvent::pos() reports the position of the mouse cursor, relative to this widget. 
	// For press and release events, the position is usually the same as the position of the 
	// last mouse move event, but it might be different if the user's hand shakes. This is a 
	// feature of the underlying window system, not Qt.
	// 
	// If you want to show a tooltip immediately, while the mouse is moving (e.g., to get the
	// mouse coordinates with QMouseEvent::pos() and show them as a tooltip), you must first 
	// enable mouse tracking as described above. Then, to ensure that the tooltip is updated 
	// immediately, you must call QToolTip::showText() instead of setToolTip() in your 
	// implementation of mouseMoveEvent().
	//

	PointD pt_m = PixelToMeters(event->x(), event->y());

	if (event->buttons() & Qt::LeftButton)		// left button is depressed
	{
		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::Select:
		case Tool::ToolType::ViewZoom:
			break;
		case Tool::ToolType::ViewPan:
			break;
		case Tool::ToolType::None:     // no tool selected
			break;
		default:
			break;
		}

		update();
	}       // if left button
	else if (event->buttons() == 0)
	{
		// no button depressed
		switch (GetApp()->m_Tool.GetTool()) {
		case Tool::ToolType::None:     // no tool selected
			break;
		case Tool::ToolType::Distance:
			{
				// in process of measuring distance
				m_ptAnchor.x = event->x();
				m_ptAnchor.y = event->y();
			}
			break;
		case Tool::ToolType::Area:
			//m_rctArea.x1 = event->x();
			//m_rctArea.y1 = event->y();
			break;
		}
	}

	m_ptLastMouse.x = event->x();
	m_ptLastMouse.y = event->y();

	PointType pt_pix = MetersToPixel(pt_m.x, pt_m.y);
	XString str = XString::Format("Meters: %0.1f, %0.1f  Pixel: %d, %d  Calc Pixel: %d, %d", pt_m.x, pt_m.y, event->x(), event->y(), pt_pix.x, pt_pix.y);
	mp_Parent->statusBar()->showMessage(str.c_str());
}

void OrthoWindow::wheelEvent(QWheelEvent* event)
{
	//	This event handler, for event event, can be reimplemented in a subclass to receive wheel events
	//	for the widget.
	//
	//	If you reimplement this handler, it is very important that you ignore() the event if you do not
	//	handle it, so that the widget's parent can interpret it.
	//
	//	The default implementation ignores the event.
	//

	int delta = event->angleDelta().y();
	if (delta)
	{
		update();
	}
	else
	{
		event->ignore();
	}
}

void OrthoWindow::keyPressEvent(QKeyEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive key press 
	// events for the widget.
	// 
	// A widget must call setFocusPolicy() to accept focus initially and have focus 
	// in order to receive a key press event.
	// 
	// If you reimplement this handler, it is very important that you call the base 
	// class implementation if you do not act upon the key.
	// 
	// The default implementation closes popup widgets if the user presses Esc. 
	// Otherwise the event is ignored, so that the widget's parent can interpret it.
	// 
	// Note that QKeyEvent starts with isAccepted() == true, so you do not need to 
	// call QKeyEvent::accept() - just do not call the base class implementation if
	// you act upon the key.
	//

	switch (event->key()) {
	case Qt::Key_D:
		mb_DebugInfo = !mb_DebugInfo;
		break;
	case Qt::Key_S:
		break;
	case Qt::Key_W:
		break;
	}

	update();

	__super::keyPressEvent(event);
}

void OrthoWindow::keyReleaseEvent(QKeyEvent* event)
{
	// This event handler can be reimplemented in a subclass to receive key release 
	// events for the widget.
	// 
	// A widget must accept focus initially and have focus in order to receive a key 
	// release event.
	// 
	// If you reimplement this handler, it is very important that you call the base 
	// class implementation if you do not act upon the key.
	// 
	// The default implementation ignores the event, so that the widget's parent can 
	// interpret it.
	// 
	// Note that QKeyEvent starts with isAccepted() == true, so you do not need to 
	// call QKeyEvent::accept() - just do not call the base class implementation if
	// you act upon the key.
	//

	__super::keyReleaseEvent(event);
}

void OrthoWindow::contextMenuEvent(QContextMenuEvent* event)
{
	Q_UNUSED(event);

	QMenu menu(this);
	menu.addAction(mp_actionClear);
	menu.addAction(mp_actionProp);
	menu.exec(event->globalPos());
}

void OrthoWindow::ZoomIn()
{
	scale(2.0, 2.0);
	update();
}

void OrthoWindow::ZoomOut()
{
	scale(0.5, 0.5);
	update();
}

void OrthoWindow::ResetView()
{
	update();
}

int OrthoWindow::AddDistancePoint(VEC2 pos_m)
{
	m_DistancePts.push_back(pos_m);
	if (m_DistancePts.size() > 1)
	{
		// add visual for new line segment
		int pts = GetDistancePointCount();

		PointType pt0 = MetersToImage(m_DistancePts[pts - 2].x, m_DistancePts[pts - 2].y);
		PointType pt1 = MetersToImage(m_DistancePts[pts - 1].x, m_DistancePts[pts - 1].y);

		// width of this line can't be fixed - think needs to be relative to image scale?
		QPen penLine(QColor(1, 1, 255), 25, Qt::SolidLine);
		// these seem to be in the units of the original image
		QGraphicsLineItem* pLine = m_scene.addLine(pt0.x, pt0.y, pt1.x, pt1.y, penLine);
		pLine->show();
		m_ItemList.push_back(pLine);	// add items to list so we can clear them without clearing entire scene

		// get current scale factor
		double sf = transform().m11();

		double d = GetDistance(pts - 1);
		XString str = XString::Format("%0.1f m", d);
		QGraphicsTextItem* pItem = m_scene.addText(str.c_str(), m_Font);
		m_ItemList.push_back(pItem);
		pItem->setScale(1.0 / sf);			// reverse scale factor or get tiny text
		pItem->moveBy(pt1.x, pt1.y);		// move to end of current line
	}

	return static_cast<int>(m_DistancePts.size());
}

int OrthoWindow::AddDistancePoint(PointD pos_m)
{
	return AddDistancePoint(VEC2(pos_m.x, pos_m.y));
}

int OrthoWindow::GetDistancePointCount() const
{
	return static_cast<int>(m_DistancePts.size());
}

VEC2 OrthoWindow::GetDistancePoint(unsigned int index)
{
	if (index < m_DistancePts.size())
		return m_DistancePts[index];

	return VEC2();
}

void OrthoWindow::ClearDistancePoints()
{
	return m_DistancePts.clear();
}

double OrthoWindow::GetDistance(unsigned int index)
{
	// return distance from m_DistancePts[index-1] to m_DistancePts[index]

	if (index > 0 && index < m_DistancePts.size())
		return (m_DistancePts[index] - m_DistancePts[index - 1]).Magnitude();

	return 0.0;
}

PointD OrthoWindow::PixelToMeters(int x, int y)
{
	PointD pt;
	
	QPointF pt_scene = mapToScene(QPoint(x, y));
	if (pt_scene.x() > 0.0 && pt_scene.x() < m_scene.width())
	{
		if (pt_scene.y() > 0.0 && pt_scene.y() < m_scene.height())
		{
			double w = m_scene.width();
			double h = m_scene.height();

			double dx = (double)pt_scene.x() / w;
			double dy = 1.0 - (double)pt_scene.y() / h;

			pt.x = m_Extent.x0 + dx * m_Extent.DX();
			pt.y = m_Extent.y0 + dy * m_Extent.DY();
		}
	}

	return pt;
}

PointType OrthoWindow::MetersToPixel(double x, double y)
{
	// Map coordinates in meters to screen pixels.
	//

	PointType pt;

	if (m_Extent.Contains(x, y))
	{
		// bounds of scaled image, pixels
		QPolygon scene_poly = mapFromScene(sceneRect());
		int scene_x0 = scene_poly[0].x();
		int scene_x1 = scene_poly[1].x();
		int scene_y0 = scene_poly[1].y();
		int scene_y1 = scene_poly[2].y();

		double dx = (x - m_Extent.x0) / m_Extent.DX();
		double dy = 1.0 - (y - m_Extent.y0) / m_Extent.DY();

		pt.x = scene_x0 + dx * (scene_x1 - scene_x0 + 1);
		pt.y = scene_y0 + dy * (scene_y1 - scene_y0 + 1);
	}

	return pt;
}

PointType OrthoWindow::MetersToImage(double x, double y)
{
	// Map coordinates in meters to image pixels.
	//

	PointType pt;

	if (m_Extent.Contains(x, y))
	{
		// size of image, pixels
		QRectF scene_rect = sceneRect();

		double dx = (x - m_Extent.x0) / m_Extent.DX();
		double dy = 1.0 - (y - m_Extent.y0) / m_Extent.DY();

		pt.x = scene_rect.x() + dx * scene_rect.width();
		pt.y = scene_rect.y() + dy * scene_rect.height();
	}

	return pt;
}

void OrthoWindow::OnClear()
{
	// Clear editing artifacts.

	for (QGraphicsItem* pItem : m_ItemList)
	{
		m_scene.removeItem(pItem);
		delete pItem;
	}
	m_ItemList.clear();

	ClearDistancePoints();
	mb_Selecting = false;
	GetApp()->m_Tool.Clear();

	update();
}

void OrthoWindow::OnProp()
{
	MetaDataDlg dlg(this, "Properties");

	XString meta = XString::Format("File Name: %s\n", tree.odm_orthophoto_tif.c_str());
	meta += XString::Format("Extents\n");
	meta += XString::Format("    X: %0.3f - %0.3f\n", m_Extent.x0, m_Extent.x1);
	meta += XString::Format("    Y: %0.3f - %0.3f\n", m_Extent.y0, m_Extent.y1);
	meta += XString::Format("Image Size: %dx%d\n", m_pixmap.width(), m_pixmap.height());

	dlg.SetMetaData(meta);
	dlg.exec();
}

