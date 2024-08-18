#ifndef DRONEWINDOW_H
#define DRONEWINDOW_H

#include "AeroMap.h"
#include "VertexBufferGL.h"

#include <QMainWindow>
#include <QOpenGLWidget>

class DroneWindow : public QOpenGLWidget
{
	Q_OBJECT

public:

	DroneWindow(QWidget* parent = nullptr);
	~DroneWindow();

	void ResetView();

protected:

	// QGLWidget
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;

	// QWidget
	virtual void contextMenuEvent(QContextMenuEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

private:

	struct ImageType
	{
		XString file_name;
		VertexBufferGL* pVB;

		ImageType()
			: pVB(nullptr)
		{

		}
	};
	std::vector<ImageType> m_ImageList;

	bool mb_Selecting;
	bool mb_DebugInfo;

	int m_ThumbWidth;
	int m_ThumbHeight;
	
	double m_colorBack[4];
	double m_colorText[4];

	ShaderGL m_shaderImage;		// 2d image shader
	ShaderGL m_shaderSS;		// screen space shader

	SizeType	m_winSize;			// client area dimensions
	PointType  	m_ptAnchor;			// anchor point for current operation
	PointType	m_ptLastMouse;		// last recorded mouse position
	RectType   	m_rctSel;			// current 2d selection

	QFont m_Font;

	QMainWindow* mp_Parent;

private:

	int LoadImageList();
	int GetImageCount();

	void RenderSelectBox();
	void RenderText(QPainter* pPainter);

	void SetBackColor(UInt8 R, UInt8 G, UInt8 B);
	void GetBackColor(UInt8& R, UInt8& G, UInt8& B);

	bool InitializeShaders();
};

#endif // #ifndef DRONEWINDOW_H
