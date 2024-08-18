// AboutDlg.cpp
// About dialog.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "AeroMap.h"			// application header
#include "AboutDlg.h"

#include <QtGui>

AboutDlg::AboutDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	// signals and slots
	verify_connect(cmdClose, SIGNAL(clicked()), this, SLOT(OnClose()));
}

AboutDlg::~AboutDlg()
{
}

void AboutDlg::OnClose()
{
    reject();
}
