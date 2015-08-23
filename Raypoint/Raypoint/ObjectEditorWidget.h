#pragma once
#include <QtWidgets\qwidget.h>
#include <QtCore\qstring.h>
#include <string>
class ConsoleWidget;
class QItemSelection;
class QTabWidget;
class QTableWidget;
class QLineEdit;
class QSplitter;
class ObjectListWidget;
class ValueTableWidget;
struct StructureObject;

class ObjectEditorWidget : public QWidget
{
	bool hasChanged;
	int editingIndex;
	QString originalName;
	QSplitter * currentObjectContainer;
	QWidget * noObjectSelected;
	QWidget * outletContainer;
	ObjectListWidget *objectList;
	ConsoleWidget *console;
	ValueTableWidget *outlets;
	QTabWidget *codeEditTabs;
	QLineEdit *nameEditor;
public:
	ObjectEditorWidget();
	void saveCurrent();
	void display( StructureObject obj );
	void hide();
	void keyPressEvent( QKeyEvent *e );
	void changingSelection();
};

