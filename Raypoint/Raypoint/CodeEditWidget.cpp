#include "CodeEditWidget.h"
#include <QtGui\qkeyevent>
#include <QtGui\qtextdocumentfragment.h>
#include <QtGui\QTextBlock>
#include <QtGui\qfont.h>
#include <iostream>

CodeEditWidget::CodeEditWidget(void)
{
	QFont f("Courier");
	f.setPixelSize( 16 );
	f.setStyleHint(QFont::Monospace);
	this->setStyleSheet( tr(
		"QTextEdit:enabled {"
			"background-color: #FFD;"
		"}"
		));
	this->setFont( f );
	this->setTabStopWidth( (int)( this->font().pixelSize() * 2.5 ) );
}

void CodeEditWidget::keyPressEvent( QKeyEvent * e )
{
	QTextCursor curs = textCursor();
	int start = textCursor().selectionStart();
	int end = textCursor().selectionEnd();

	QString plain = this->toPlainText();

	int lineCount = 1;

	for( int i = start; i < end; i++ )
	{
		if( plain[i] == '\r' || plain[i] == '\n' )
			lineCount++;
	}

	if( e->key() == Qt::Key::Key_Tab && lineCount != 1 )
	{
		if( curs.position() > start )
		{
			for( int i = 0; i < lineCount-1; i++ )
			{
				curs.movePosition( QTextCursor::Up );
			}
		}
		while( lineCount > 0 )
		{
			curs.movePosition( QTextCursor::StartOfLine );
			setTextCursor( curs );
			insertPlainText( "\t" );
			curs.movePosition( QTextCursor::Down );
			curs.movePosition( QTextCursor::StartOfLine );
			lineCount--;
			/*if( plain[index] == '\n' )
			{
				curs.movePosition( QTextCursor::StartOfLine );
				setTextCursor( curs );
				insertPlainText( "\t" );
				curs.movePosition( QTextCursor::Down );
				curs.movePosition( QTextCursor::StartOfLine );
			}
			index++;*/
		}
		//std::cout << firstLine << std::endl;
		std::cout << lineCount << std::endl;
	}
	else
	{
		QTextEdit::keyPressEvent( e );
	}

}