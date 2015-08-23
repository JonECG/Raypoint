#pragma once
#include <QtWidgets\qtextedit.h>

class CodeEditWidget : public QTextEdit
{
public:
	CodeEditWidget();
	void CodeEditWidget::keyPressEvent( QKeyEvent * e );
};

