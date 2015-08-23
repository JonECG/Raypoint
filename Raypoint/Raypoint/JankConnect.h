#pragma once
#include <functional>
#include <QtCore\qobject.h>

class JankConnect : public QObject
{
	Q_OBJECT;
	std::function<void()> func;

public:
	JankConnect(std::function<void()> func);

	inline static JankConnect* connect( const QObject* sender, const char * signal, std::function<void()> func )
	{
		JankConnect * result = new JankConnect( func );
		QObject::connect( sender, signal, result, SLOT(proxySlot()) );
		QObject::connect( sender, &QObject::destroyed, [result](){ delete result; } );
		return result;
	}
public slots:
	void proxySlot();
};

