// StageReport.cpp
// Generate pdf report.
//

#include <fstream>
#include <nlohmann/json.h>
using json = nlohmann::json;

#include "StageReport.h"

constexpr char* TITLE = "AeroMap Process Report";

int StageReport::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_report
	//			report.pdf
	//

	int status = 0;

	BenchmarkStart();
	GetApp()->LogWrite("Generate report...");

	AeroLib::CreateFolder(tree.odm_report);

	XString file_name = XString::CombinePath(tree.odm_report, "report.pdf");

	QPdfWriter pdfWriter(file_name.c_str());
	mp_Painter = new QPainter(&pdfWriter);
	m_resolution = pdfWriter.resolution();

	pdfWriter.setTitle(TITLE);
	pdfWriter.setPageSize(QPageSize(QPageSize::A4));

	LoadOpenSfmStats();
	InitFonts();

	OutputSummary();
	OutputOpenSFM();
	OutputOrthophoto();
	OutputParameters();

	mp_Painter->end();
	delete mp_Painter;

	BenchmarkStop("Report");

	return status;
}

void StageReport::OutputSummary()
{
	int y = OutputHeader();

	mp_Painter->setFont(m_fontHeader1);
	OutputText(m_resolution / 2, y += 400, "Summary");

	y += 200;

	AeroLib::Georef georef = AeroLib::ReadGeoref();

	mp_Painter->setFont(m_fontBody);
	OutputText(m_resolution, y += 200, "Images: %d", GetProject().GetImageCount());
	if (georef.is_valid)
	{
		double lat, lon;
		GIS::XYToLatLon_UTM(georef.utm_zone, georef.hemi, georef.x, georef.y, lat, lon, GIS::Ellipsoid::WGS_84);

		OutputText(m_resolution, y += 200, "Location: %0.2f %c / %0.2f %c", lat, lat >= 0.0 ? 'N' : 'S', lon, lon >= 0.0 ? 'E' : 'W');
	}
	OutputText(m_resolution, y += 200, "Date Captured: %s", m_EndDate.c_str());
	OutputText(m_resolution, y += 200, "Date Processed: %s", m_ProcessDate.c_str());
	OutputText(m_resolution, y += 200, "Area Covered: %0.6f sq km", m_AreaCovered);
}

void StageReport::OutputOpenSFM()
{
	if (QFile::exists(tree.opensfm.c_str()) == false)
		return;

	// since images embedded into pdf file, scale down to reasonable size

	XString top_view = XString::CombinePath(tree.opensfm, "stats/topview.png");
	QImage image;
	if (image.load(top_view.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "Unable to load: '%s'", top_view.c_str());
		return;
	}

	int y = OutputHeader(true);

	mp_Painter->setFont(m_fontHeader1);
	OutputText(m_resolution / 2, y += 400, "OpenSFM Outputs");

	double width_in = 6.0;
	double aspect = (double)image.width() / (double)image.height();
	double height_in = width_in / aspect;

	QRect rectTarget(m_resolution, m_resolution, width_in * m_resolution, height_in *m_resolution);
	QRect rectSrc = image.rect();

	mp_Painter->drawImage(rectTarget, image, rectSrc);
}

void StageReport::OutputOrthophoto()
{
	if (QFile::exists(tree.odm_orthophoto_tif.c_str()) == false)
		return;

	// since images embedded into pdf file, scale down to reasonable size

	QImage image;
	if (image.load(tree.odm_orthophoto_tif.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "Unable to load orthophoto: '%s'", tree.odm_orthophoto.c_str());
		return;
	}

	const int MAX_SIZE = 1000;		// maximum image dimension, pixels

	double aspect = (double)image.width() / (double)image.height();
	int max_dim = image.width() > image.height() ? image.width() : image.height();
	if (max_dim > MAX_SIZE)
	{
		double sf = (double)MAX_SIZE / (double)max_dim;
		image = image.scaled(image.width() * sf, image.height() * sf, Qt::KeepAspectRatio);
	}

	XString file_name = XString::CombinePath(tree.odm_report, "ortho.png");
	image.save(file_name.c_str(), "PNG");

	int y = OutputHeader(true);

	mp_Painter->setFont(m_fontHeader1);
	OutputText(m_resolution / 2, y += 400, "Orthophoto");

	double width_in = 6.5;
	double height_in = width_in / aspect;

	QRect rectTarget(m_resolution, m_resolution, width_in * m_resolution, height_in * m_resolution);
	QRect rectSrc = image.rect();

	mp_Painter->drawImage(rectTarget, image, rectSrc);
}

void StageReport::OutputParameters()
{
	int y = OutputHeader(true);

	mp_Painter->setFont(m_fontHeader1);
	OutputText(m_resolution / 2, y += 400, "Parameters");

	y += 200;

	mp_Painter->setFont(m_fontBody);
	OutputText(m_resolution, y += 200, "DEM Resolution: %0.2f cm/pixel", arg.dem_resolution);
	OutputText(m_resolution, y += 200, "Orthophoto Resolution: %0.2f cm/pixel", arg.orthophoto_resolution);
	OutputText(m_resolution, y += 200, "Crop Buffer: %0.2f m", arg.crop);
	OutputText(m_resolution, y += 200, "DTM: %s", arg.dtm ? "True" : "False");
	OutputText(m_resolution, y += 200, "DSM: %s", arg.dsm ? "True" : "False");
}

int StageReport::OutputHeader(bool new_page)
{
	if (new_page)
		GetPdfWriter()->newPage();

	int y = m_resolution / 4;

	mp_Painter->setFont(m_fontHeader);
	OutputText(m_resolution / 2, y += 400, TITLE);

	return y;
}

void StageReport::InitFonts()
{
	m_fontHeader.setFamily("Cambria");
	m_fontHeader.setPointSizeF(12.0);
	m_fontHeader.setBold(true);

	m_fontHeader1.setFamily("Cambria");
	m_fontHeader1.setPointSizeF(11.0);
	m_fontHeader1.setBold(true);

	m_fontBody.setFamily("Cambria");
	m_fontBody.setPointSizeF(10.0);
	m_fontBody.setBold(false);
}

void StageReport::LoadOpenSfmStats()
{
	m_ProcessDate.Clear();
	m_StartDate.Clear();
	m_EndDate.Clear();
	m_AreaCovered = 0.0;

	XString stats_file = XString::CombinePath(tree.opensfm, "stats/stats.json");

	if (QFile::exists(stats_file.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "'%s' not found.");
		return;
	}

	std::ifstream f(stats_file.c_str());
	json data = json::parse(f);

	if (data.contains("processing_statistics"))
	{
		json proc_stat = data["processing_statistics"];

		//"date": "05/08/2024 at 08:38:45",
		m_ProcessDate = proc_stat["date"].template get<std::string>().c_str();
		//"start_date" : "04/03/2011 at 08:42:16",
		m_StartDate = proc_stat["start_date"].template get<std::string>().c_str();
		//"end_date" : "04/03/2011 at 08:48:18",
		m_EndDate = proc_stat["end_date"].template get<std::string>().c_str();
		//"area" : 209092.46879010607
		m_AreaCovered = proc_stat["area"];
		m_AreaCovered *= 1E-6;
	}
	else
	{
		Logger::Write(__FUNCTION__, "'%s' does not contain 'processing_statistics' key.", stats_file.c_str());
	}
}

void StageReport::OutputText(int x, int y, const char* text, ...)
{
	// Inputs:
	//		function = function name
	//		text     = printf style formatting string
	//		...	     = parameters to formatting string
	//

	char buf[255] = { 0 };

	va_list ap;
	va_start(ap, text);
	vsprintf(buf, text, ap);
	va_end(ap);

	mp_Painter->drawText(x, y, buf);
}

QPdfWriter* StageReport::GetPdfWriter()
{
	return (QPdfWriter*)mp_Painter->paintEngine()->paintDevice();
}
