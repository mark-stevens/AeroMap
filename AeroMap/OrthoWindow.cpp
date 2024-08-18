// OrthoWindow.cpp
// Orthophoto window.
//

#include "MainWindow.h"
#include "OrthoWindow.h"

#include <QStatusBar>

const int IMAGE_MARGIN = 12;	// spacing around image, pixels

OrthoWindow::OrthoWindow(QWidget* parent)
	: QGraphicsView(parent)
	, mb_Selecting(false)
	, mb_DebugInfo(true)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setMinimumSize(90, 90);

	mp_Parent = static_cast<MainWindow*>(parent);
	assert(mp_Parent != nullptr);

	setMouseTracking(true);
	setAcceptDrops(true);
	grabKeyboard();

	// set up fonts

	m_Font.setFamily("Consolas");
	m_Font.setPointSize(10.0);
	m_Font.setItalic(false);

	CreateScene();
}

OrthoWindow::~OrthoWindow()
{
	m_scene.clear();

	// clean up opengl state, other gl windows will
	// re-inititialize it
	GLManager::Shutdown();
}

void OrthoWindow::CreateScene()
{
	// Recreate entire scene.
	//

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

	m_scene.clear();

	QBrush brushBack(SCENE_BACK_COLOR, Qt::SolidPattern);

	m_scene.addPixmap(m_pixmap);

	// make scene exactly same size as image
	m_scene.setSceneRect(0.0, 0.0, m_pixmap.width(), m_pixmap.height());

	setScene(&m_scene);
	setMouseTracking(true);
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

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::Select:
	case Tool::ToolType::ViewZoom:
	case Tool::ToolType::ViewPan:
		m_ptAnchor.x = event->x();
		m_ptAnchor.y = event->y();
		break;
	case Tool::ToolType::Distance:
		break;
	case Tool::ToolType::Area:
		break;
	case Tool::ToolType::Volume:
		break;
	default:   // no active tool
		break;
	}

	update();

	m_ptLastMouse.x = event->x();  // last recorded mouse position
	m_ptLastMouse.y = event->y();
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

	m_ptLastMouse.x = event->x();
	m_ptLastMouse.y = event->y();
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
