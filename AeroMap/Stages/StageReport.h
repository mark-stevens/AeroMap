#ifndef STAGEREPORT_H
#define STAGEREPORT_H

#include "Stage.h"

class StageReport : Stage
{
public:

	virtual int Run() override;

private:
	
	QPainter* mp_Painter;

	QFont m_fontHeader;
	QFont m_fontHeader1;
	QFont m_fontBody;

	int m_resolution;		// dpi

	XString m_ProcessDate;
	XString m_StartDate;
	XString m_EndDate;
	double m_AreaCovered;

private:

	void OutputSummary();
	void OutputOpenSFM();
	void OutputOrthophoto();
	void OutputParameters();

	int OutputHeader(bool new_page = false);
	void OutputText(int x, int y, const char* text, ...);

	void InitFonts();
	void LoadOpenSfmStats();
	QPdfWriter* GetPdfWriter();
};

#endif // #ifndef STAGEREPORT_H
