#include "AnimationGridEditor.h"
#include <QtGui\QPaintEvent>
#include <QtGui\QMouseEvent>
#include <QtGui\qpainter.h>
#include <QtGui\QWheelEvent>
#include "SceneModel.h"
#include <math.h>
#include <QtWidgets\qpushbutton.h>

const int ROW_HEIGHT = 32;
const int VARIABLE_NAME_WIDTH = 128;
const int VARIABLE_NAME_PADDING = 4;

const int TIME_MARKER_WIDTH = 24;
const int TIME_MARKER_HEIGHT = 16;

const int SECONDS_COUNT_HEIGHT = 32;
const int SECOND_DIVISIONS = 10;

const int NODE_RADIUS = 10;

const int START_ROWS_Y = std::max( SECONDS_COUNT_HEIGHT, TIME_MARKER_HEIGHT );

AnimationGridEditor::AnimationGridEditor(SceneModel ** scene)
{
	isPlaying = false;
	QPushButton * butt = new QPushButton( "Preview" );
	butt->resize( VARIABLE_NAME_WIDTH, START_ROWS_Y );
	butt->setParent( this );

	connect( butt, &QPushButton::clicked, [this,butt](){
		isPlaying = !isPlaying;
		if( isPlaying )
		{
			playPosition = std::max( 0.0f, markerPosition );
			timeStart = markerPosition - timeSpan/2;
			highlightedNode = -1;
			selectionChanged( highlightedRow, highlightedNode, playPosition );
			butt->setText( "Stop Preview" );
		}
		else
		{
			timeStart = markerPosition - timeSpan/2;
			markerChanged( markerPosition );
			butt->setText( "Preview" );
		}
		repaint();
	});

	this->scene = scene;
	timeStart = 0;
	timeSpan = 1;
	highlightedRow = 0;
	highlightedNode = -1;
	markerPosition = 0.5f;
	mouseTracking = false;
	setFocusPolicy( Qt::FocusPolicy::ClickFocus );

	dtTimer.interval();

	connect( &qtimer, &QTimer::timeout, [this](){
		float dt = dtTimer.interval();

		if( isPlaying )
		{
			playPosition += dt;
			timeStart = playPosition - timeSpan/2;
			markerChanged( playPosition );
			repaint();
		}
	});
	qtimer.start(10);
}

static std::string ftostr(float f)
{
	std::string s = std::to_string( f );
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    return (s[s.size()-1] == '.') ? s.substr(0, s.size()-1) : s;
}

void AnimationGridEditor::paintEvent( QPaintEvent * e )
{
	QPainter paint( this );

	float selectTime = (isPlaying) ? playPosition : markerPosition;

	//timeStart += 0.01f;
	//timeSpan += 0.02f;

	//const char * testStuff[] = { "Camera", "VariableOne", "VariableTwo" };
	int count = (*scene)->variables.size();

	//COLOR
	paint.fillRect( QRect( 0, START_ROWS_Y, width(), height() ), QColor( 150, 150, 150 ) );
	paint.fillRect( QRect( VARIABLE_NAME_WIDTH, START_ROWS_Y, width()-VARIABLE_NAME_WIDTH-1, count*ROW_HEIGHT ), QColor( 180, 180, 180 ) );
	for( int i = 0; i < count; i++ )
	{
		int col = ( i % 2 == 0 ) ? 220 : 240;
		paint.fillRect( QRect( 0, START_ROWS_Y + i* ROW_HEIGHT, VARIABLE_NAME_WIDTH, ROW_HEIGHT ), QColor( col, col, col ) );
		paint.fillRect( QRect( VARIABLE_NAME_WIDTH, START_ROWS_Y + i* ROW_HEIGHT, width() - VARIABLE_NAME_WIDTH, ROW_HEIGHT ), QColor( col - 70, col - 70, col - 70 ) );
	}
	if( timeStart < 0 )
		paint.fillRect( QRect( VARIABLE_NAME_WIDTH, START_ROWS_Y, timeInPixels(0)-VARIABLE_NAME_WIDTH, count*ROW_HEIGHT ), QColor( 80,80,80 ) );
	paint.fillRect( QRect( std::max( 0, timeInPixels(0) ), START_ROWS_Y + highlightedRow* ROW_HEIGHT, width(), ROW_HEIGHT + 1), QColor( 250, 250, 180 ) );

	//OUTLINE
	/*for( int i = 0; i < count; i++ )
	{
		paint.drawRect( QRect( 0, START_ROWS_Y + i* ROW_HEIGHT, VARIABLE_NAME_WIDTH, ROW_HEIGHT ) );
	}*/
	paint.drawRect( QRect( VARIABLE_NAME_WIDTH, START_ROWS_Y, width()-VARIABLE_NAME_WIDTH-1, count*ROW_HEIGHT ) );

	//ELEMENTS
	
	//Connection
	unsigned int foundLesser;
	bool found = false;
	if( highlightedRow != -1 && highlightedNode == -1 )
	{
		for( unsigned int g = 0; !found && g < (*scene)->variables[highlightedRow].animation.size(); g++ )
		{
			foundLesser = g;
			if( (*scene)->variables[highlightedRow].animation[g].time > selectTime )
			{
				found = true;
				foundLesser--;
			}
		}

		paint.setBrush( QColor(255,200,0) );
		if( (*scene)->variables[highlightedRow].animation.size() > 1 && foundLesser > -1 && foundLesser < (*scene)->variables[highlightedRow].animation.size() - 1 )
		{
			int p1x = timeInPixels( (*scene)->variables[highlightedRow].animation[foundLesser].time );
			int p2y = timeInPixels( (*scene)->variables[highlightedRow].animation[foundLesser+1].time );
			paint.fillRect( QRect( p1x, START_ROWS_Y + highlightedRow * ROW_HEIGHT + ROW_HEIGHT/4, p2y-p1x, ROW_HEIGHT/2 ), QColor(255,200,0) );
		}
	}

	//Nodes
	paint.setPen( QColor(0,0,0) );
	for( int i = 0; i < count; i++ )
	{
		for( unsigned int j = 0; j < (*scene)->variables[i].animation.size(); j++ )
		{
			KeyPoint pt = (*scene)->variables[i].animation[j];

			paint.setBrush( QColor(0,0,200) );
			if( i == highlightedRow )
			{
				if(highlightedNode != -1 )
				{
					if( j == highlightedNode )
						paint.setBrush( QColor(255,200,0) );

					if( std::abs( int( j - highlightedNode ) ) == 1 )
						paint.setBrush( QColor(255,50,50) );
				}
				else
				{
					if( j - foundLesser == 0 || j - foundLesser == 1 )
						paint.setBrush( QColor(255,50,50) );
					if( j - foundLesser == -1 || j - foundLesser == 2 )
						paint.setBrush( QColor(200,40,255) );
				}
			}
			
			paint.drawEllipse( QPoint( timeInPixels( pt.time ), START_ROWS_Y + i * ROW_HEIGHT + ROW_HEIGHT/2 ), NODE_RADIUS, NODE_RADIUS );
		}
	}

	//Time Indicators
	paint.setPen( QColor(100,100,100) );
	float closestPow = std::floor( std::logf( timeSpan / ( SECOND_DIVISIONS / 2 ) ) / std::logf( SECOND_DIVISIONS ) + 0.5f );
	float closestIndicator = std::pow( SECOND_DIVISIONS, closestPow );
	for( float time = std::max( 0.0f, closestIndicator * std::ceil( timeStart /closestIndicator ) ); time < timeStart + timeSpan; time += closestIndicator )
	{
		int posX = timeInPixels( time );
		paint.drawRect( QRect( posX, 0, 0, START_ROWS_Y + count*ROW_HEIGHT ) );
		paint.drawText( QRect( posX, 0, 50, 20 ), ( ftostr( time ) + "s" ).c_str() );
	}

	//Time Marker
	if( markerPosition >= timeStart && markerPosition <= timeStart + timeSpan )
	{
		paint.setPen( QColor( ((isPlaying) ? 128 : 255),0,0) );
		int markerX = timeInPixels( markerPosition );
		paint.drawRect( QRect( markerX, START_ROWS_Y, 0, count*ROW_HEIGHT ) );
		paint.fillRect( QRect( markerX - TIME_MARKER_WIDTH/2, START_ROWS_Y - TIME_MARKER_HEIGHT, TIME_MARKER_WIDTH, TIME_MARKER_HEIGHT ), QColor( ((isPlaying) ? 128 : 255),0,0) );
	}

	//PlayMarker
	if( isPlaying && playPosition >= timeStart && playPosition <= timeStart + timeSpan )
	{
		paint.setPen( QColor(50, 255, 50) );
		int markerX = timeInPixels( playPosition );
		paint.drawRect( QRect( markerX, START_ROWS_Y, 0, count*ROW_HEIGHT ) );
		//paint.fillRect( QRect( markerX - TIME_MARKER_WIDTH/2, START_ROWS_Y - TIME_MARKER_HEIGHT, TIME_MARKER_WIDTH, TIME_MARKER_HEIGHT ), QColor( 255, 0, 0 ) );
	}

	//VariableNames
	for( int i = 0; i < count; i++ )
	{
		int col = ( i % 2 == 0 ) ? 220 : 240;
		paint.fillRect( QRect( 0, START_ROWS_Y + i* ROW_HEIGHT, VARIABLE_NAME_WIDTH, ROW_HEIGHT ), QColor( col, col, col ) );
	}
	paint.setPen( QColor(0,0,0) );
	paint.setBrush( QColor(0,0,0,0) );
	for( int i = 0; i < count; i++ )
	{
		paint.drawRect( QRect( 0, START_ROWS_Y + i* ROW_HEIGHT, VARIABLE_NAME_WIDTH, ROW_HEIGHT ) );
	}
	for( int i = 0; i < count; i++ )
	{
		paint.drawText( QRect( VARIABLE_NAME_PADDING, START_ROWS_Y + i*ROW_HEIGHT + VARIABLE_NAME_PADDING, VARIABLE_NAME_WIDTH - VARIABLE_NAME_PADDING*2, ROW_HEIGHT - VARIABLE_NAME_PADDING*2 ), (*scene)->variables[i].name.c_str() );
	}

	

	setMinimumHeight( START_ROWS_Y + ROW_HEIGHT * count + 1 );
	
}

int AnimationGridEditor::getNodeAt( int x, int y, int row )
{
	bool nodeClicked = false;
	int node = -1;

	for( unsigned int i = 0; !nodeClicked && i < (*scene)->variables[row].animation.size(); i++ )
	{
		int nodeX = timeInPixels( (*scene)->variables[row].animation[i].time );

		if( (x-nodeX)*(x-nodeX) + (y-(row*ROW_HEIGHT+START_ROWS_Y+ROW_HEIGHT/2))*(y-(row*ROW_HEIGHT+START_ROWS_Y+ROW_HEIGHT/2)) < NODE_RADIUS*NODE_RADIUS )
		{
			nodeClicked = true;
			node = i;
		}
	}

	return node;
}

void AnimationGridEditor::mousePressEvent( QMouseEvent * e )
{
	if( e->button() == Qt::MouseButton::RightButton )
	{
		if( !isPlaying && e->modifiers() & Qt::ControlModifier && isInsideRows( e->x(), e->y() ) )
		{
			highlightedRow = getRow( e->y() );
			int node = getNodeAt( e->x(), e->y(), highlightedRow );

			if( node != -1 )
			{
				(*scene)->variables[highlightedRow].animation.erase( (*scene)->variables[highlightedRow].animation.begin() + node );
				highlightedNode = -1;
				selectionChanged( highlightedRow, highlightedNode, markerPosition );
				markerChanged( markerPosition );
				repaint();
				e->accept();
			}
		}
	}

	if( e->button() == Qt::MouseButton::LeftButton )
	{
		if( !isPlaying && e->y() > START_ROWS_Y - TIME_MARKER_HEIGHT && e->y() < START_ROWS_Y && e->x() > timeInPixels( markerPosition ) - TIME_MARKER_WIDTH/2 && e->x() < timeInPixels( markerPosition ) + TIME_MARKER_WIDTH/2 )
		{
			mouseTracking = true;
			mouseTrackedX = e->x();
			highlightedNode = -1;
			selectionChanged( highlightedRow, highlightedNode, markerPosition );
			mouseTrackButton = Qt::MouseButton::LeftButton;
			repaint();
			e->accept();
		}

		if( isInsideRows( e->x(), e->y() ) )
		{
		
			highlightedRow = getRow( e->y() );
			
			int nodeClicked = -1;
			if( !isPlaying )
			{
				nodeClicked = getNodeAt( e->x(), e->y(), highlightedRow );
				if( nodeClicked != -1 )
				{
					highlightedNode = nodeClicked;
					markerPosition = (*scene)->variables[highlightedRow].animation[highlightedNode].time;
					markerChanged( markerPosition );
				}
			}

			if( nodeClicked == -1 )
			{
				highlightedNode = -1;

				if( !isPlaying )
					markerPosition = getTime( e->x() );
			
				if( !isPlaying && e->modifiers() & Qt::ControlModifier )
				{
					int insertIndex = -1;
					for( unsigned int i = 0; insertIndex == -1 &&i < (*scene)->variables[highlightedRow].animation.size(); i++ )
					{
						if( (*scene)->variables[highlightedRow].animation[i].time > markerPosition )
							insertIndex = i;
					}

					KeyPoint point = (*scene)->variables[highlightedRow].makePoint( markerPosition );

					if( insertIndex == -1 )
						(*scene)->variables[highlightedRow].animation.push_back( point );
					else
						(*scene)->variables[highlightedRow].animation.insert( (*scene)->variables[highlightedRow].animation.begin() + insertIndex, point );

					highlightedNode = ( insertIndex != -1 ) ? insertIndex : (*scene)->variables[highlightedRow].animation.size()-1;

				}

				if( !isPlaying )
					markerChanged( markerPosition );
			}

			selectionChanged( highlightedRow, highlightedNode, markerPosition );

			repaint();
			e->accept();
		}
	}
	else
	if( !isPlaying && e->button() == Qt::MouseButton::MiddleButton )
	{
		mouseTracking = true;
		mouseTrackedX = getTime( e->x() );
		mouseTrackButton = Qt::MouseButton::MiddleButton;
		e->accept();
	}
}

void AnimationGridEditor::mouseReleaseEvent( QMouseEvent * e )
{
	if ( e->button() == mouseTrackButton && mouseTracking )
	{
		mouseTracking = false;
		e->accept();
	}
}

void AnimationGridEditor::mouseMoveEvent( QMouseEvent * e )
{
	if ( mouseTracking )
	{
		if( mouseTrackButton == Qt::MouseButton::MiddleButton )
		{
			float mouseIsHovering = getTime(e->x());

			timeStart -= mouseIsHovering - mouseTrackedX;

			timeStart = std::max( -timeSpan/2, timeStart );
			repaint();
			e->accept();
		}

		if( mouseTrackButton == Qt::MouseButton::LeftButton )
		{
			markerPosition = getTime( e->x() );
			markerChanged( markerPosition );
			repaint();
			e->accept();
		}
	}
}

void AnimationGridEditor::wheelEvent( QWheelEvent * e )
{
	float timeHold;

	if( isInsideRows( e->x(), e->y(), &timeHold ) )
	{
		float oldTimeSpan = timeSpan;

		float mouseWasHovering = timeHold;

		timeSpan *= ( e->delta() > 0 ) ? 5/6.0f : 6/5.0f;
		timeSpan = std::max( 0.01f, std::min( 1000.0f, timeSpan ) );

		float mouseIsHovering = getTime( e->x() );

		timeStart -= mouseIsHovering - mouseWasHovering;

		timeStart = std::max( -timeSpan/2, timeStart );

		repaint();
		e->accept();
	}
}
bool AnimationGridEditor::isInsideRows( int x, int y, float* timePtr, int* rowPtr )
{
	int count = (*scene)->variables.size();

	bool result = x > VARIABLE_NAME_WIDTH && x < width() &&  y > START_ROWS_Y && y < START_ROWS_Y + count * ROW_HEIGHT;

	if( result )
	{
		if( timePtr )
		{
			*timePtr = getTime(x);
		}
		if( rowPtr )
		{
			*rowPtr = getRow(y);
		}
	}

	return result;
}
float AnimationGridEditor::getTime( int pixelX )
{
	return timeStart + timeSpan * (pixelX-VARIABLE_NAME_WIDTH)/(width()-VARIABLE_NAME_WIDTH);
}
int AnimationGridEditor::getRow( int pixelY )
{
	return (pixelY-START_ROWS_Y) / ROW_HEIGHT;
}
int AnimationGridEditor::timeInPixels( float time )
{
	return (int)( VARIABLE_NAME_WIDTH + ( width()-VARIABLE_NAME_WIDTH-1 ) * (time-timeStart)/timeSpan );
}
float AnimationGridEditor::getCurrentPosition()
{
	return (isPlaying) ? playPosition : markerPosition;
}
bool AnimationGridEditor::getIsPlaying()
{
	return isPlaying;
}
int AnimationGridEditor::getCurrentSelectedNode()
{
	return highlightedNode;
}
int AnimationGridEditor::getCurrentSelectedRow()
{
	return highlightedRow;
}