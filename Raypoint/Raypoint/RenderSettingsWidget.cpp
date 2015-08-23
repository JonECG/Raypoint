#include "RenderSettingsWidget.h"

#include "SceneModel.h"
#include <QtWidgets\qpushbutton.h>

#include <QtWidgets\qlayout.h>
#include "NewRayMarchCL.h"
#include <thread>

#include <opencv2\opencv.hpp>

#include <QtWidgets\qfiledialog.h>
#include <QtWidgets\qformlayout.h>
#include <QtWidgets\qlabel.h>

#include <QtWidgets\qspinbox.h>
#include <QtWidgets\qprogressbar.h>
#include <QtCore\qtimer.h>

#include <QtWidgets\qtabwidget.h>

#include <QtWidgets\qmessagebox.h>

RenderSettingsWidget::RenderSettingsWidget( SceneModel** scene )
{
	selectedPath = false;
	this->scene = scene;

	setLayout( new QHBoxLayout() );

	QWidget *form = new QWidget;
	form->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
	QFormLayout *formLay = new QFormLayout;
	form->setLayout( formLay );

	QSpinBox *fpsInput = new QSpinBox;
	fpsInput->setMinimum( 1 ); fpsInput->setMaximum( 120 ); fpsInput->setValue( 30 );
	formLay->addRow( tr( "FPS" ), fpsInput );

	QSpinBox *widthInput = new QSpinBox, *heightInput = new QSpinBox;
	widthInput->setMinimum( 1 ); widthInput->setMaximum( 1920 ); widthInput->setValue( 1920 );
	heightInput->setMinimum( 1 ); heightInput->setMaximum( 1080 ); heightInput->setValue( 1080 );
	formLay->addRow( tr( "Width" ), widthInput );
	formLay->addRow( tr( "Height" ), heightInput );

	QWidget *pathSelect = new QWidget;
	pathSelect->setLayout( new QHBoxLayout );
	pathSelect->layout()->setSpacing(3);
	pathSelect->layout()->setContentsMargins(0,0,5,0);
	QLabel *currentPath = new QLabel( "No path selected" );
	QPushButton *pathButt = new QPushButton( "Select" );
	pathButt->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
	currentPath->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	pathSelect->layout()->addWidget( pathButt );
	pathSelect->layout()->addWidget( currentPath );
	formLay->addRow( tr( "Path" ), pathSelect );

	connect( pathButt, &QPushButton::clicked, [this, currentPath](){

		QString fileName = QFileDialog::getSaveFileName(this,tr("Save Video"), "", tr("Video (*.avi)"));
		if( !fileName.isEmpty() )
		{
			selectedPath = true;
			path = fileName;

			QString prev = fileName;

			if( prev.length() > 40 )
			{
				int lastIndex = prev.lastIndexOf( '/' );
				int currentSize = prev.length() - lastIndex;
				while( currentSize < 40 )
				{
					lastIndex = prev.length() - currentSize;
					int look = prev.lastIndexOf( '/', lastIndex-1 );
					currentSize += lastIndex - look;
				}
				prev = "..." + prev.right( lastIndex );
			}
			currentPath->setText( prev );
		}
		else
		{
			selectedPath = false;
			currentPath->setText( "No path selected" );
		}

	});

	QPushButton * renderButt = new QPushButton( "Render" );

	formLay->addWidget( renderButt );
	layout()->addWidget( form );

	QWidget * currentRenderWidg = new QWidget;
	currentRenderWidg->setLayout( new QVBoxLayout );
	currentRenderWidg->layout()->setAlignment(Qt::AlignTop);
	currentRenderWidg->layout()->addWidget( new QLabel( "Current Render Job" ) );

	QWidget *currentStats = new QWidget;
	QFormLayout *currentFormStats = new QFormLayout;
	currentStats->setLayout( currentFormStats );
	QLabel *actionStatus = new QLabel, *frameIndicator = new QLabel, *currentTimeIndicator = new QLabel, *estimatedTimeLeft = new QLabel;
	QProgressBar *progress = new QProgressBar;
	currentFormStats->addWidget( actionStatus );
	currentFormStats->addRow( "Frame", frameIndicator );
	currentFormStats->addRow( "Time Elapsed", currentTimeIndicator );
	currentFormStats->addRow( "Est. Remaining", estimatedTimeLeft );
	currentFormStats->addWidget( progress );
	progress->setMinimum( 0 );
	progress->setMaximum( 100 );

	actionString = "Waiting for Job";
	isRendering = false;

	currentRenderWidg->layout()->addWidget( currentStats );

	layout()->addWidget( currentRenderWidg );

	connect( &qtimer, &QTimer::timeout, [this, form, renderButt, actionStatus, frameIndicator, currentTimeIndicator, estimatedTimeLeft, progress](){
		float dt = dtTimer.interval();

		actionStatus->setText( actionString );
		frameIndicator->setText( frameString );
		currentTimeIndicator->setText( timeString );
		estimatedTimeLeft->setText( estString );
		progress->setValue( (int)showProgress );
		renderButt->setEnabled( !isRendering && selectedPath );

		QTabWidget * widg = (QTabWidget*)parentWidget()->parentWidget();
		form->setEnabled( !isRendering );
		widg->setTabEnabled( 0, !isRendering );
		widg->setTabEnabled( 1, !isRendering );
		widg->setTabEnabled( 2, !isRendering );
	});
	qtimer.start(10);

	connect( renderButt, &QPushButton::clicked, [this, widthInput, heightInput, fpsInput ](){

		std::thread worker( [this, widthInput, heightInput, fpsInput ](){
			if( (*this->scene)->structures.size() > 0 )
			{
				actionString = "Calculating Images";
				QString workingString = path;
				QString prepend = workingString.left( workingString.lastIndexOf( '.' ) );
				isRendering = true;

				int width = widthInput->value();
				int height = heightInput->value();

				float fps = fpsInput->value();
				float interv = 1/fps;

				float beginTime = 0;
				float endTime = 0;

				for( unsigned int i = 0; i < (*this->scene)->variables.size(); i++ )
				{
					if( (*this->scene)->variables[i].animation.size() > 0 )
						endTime = std::max( endTime, (*this->scene)->variables[i].animation[ (*this->scene)->variables[i].animation.size() - 1 ].time );
				}

				if( endTime > interv )
				{
					try
					{
						NewRayMarchCL * march = new NewRayMarchCL;
						march->initialize();
						march->load( "Assets/temp/testAll.cl" );

						Timer tim;
						tim.interval();

						float actualTime = 0;

						int frame;
						for( frame = 0; frame * interv < endTime; frame++ )
						{
							float time = frame * interv;

							actualTime += tim.interval();
							frameString = tr( std::to_string( frame + 1 ).c_str() ) + " of " + tr( std::to_string( int(endTime/interv + 1) ).c_str() );
							timeString = tr( std::to_string( (int)(actualTime) ).c_str() ) + "s";
							estString = (frame == 0) ? "N/A" : QString::number( int((actualTime/frame)*(endTime/interv-frame+1)) ) + "s";
							showProgress = 100 * float(frame) / int(endTime/interv + 1);

							//std::cout << "Calculating Frame " << frame << " of " <<  int(endTime/interv) << std::endl;

							*march->camera = (*this->scene)->calcCamera( time );
							(*march->camera).aspect = float(width)/height;

							for( unsigned int i = 1; i < (*this->scene)->variables.size(); i++ )
							{
								march->setData( i, (*this->scene)->variables[i].calcValue( 0, time ) );
							}

							march->launch( glm::vec2( width, height ) );

							while( !march->isComplete() )
							{
								std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
							}

							//std::cout << "Drawing Frame " << frame << " of " <<  int(endTime/interv) << std::endl;

							QImage img( width, height, QImage::Format_ARGB32 );

							img.fill( QColor( 0, 0, 0 ) );

							for( int cell = 0; cell < march->calculatedAmount && cell < width * height; cell++ )
							{
								glm::vec2 solvedSize = march->subdivideSizes[std::max( march->calculatedAmount/2, cell )];
								glm::vec2 solvedPosition = march->subdividePositions[cell];
								int posXInt = std::ceil( solvedPosition.x * width - 0.5 );
								int posYInt = std::ceil( solvedPosition.y * height - 0.5 );

								glm::vec4 pix = 255.0f * march->getPixel(cell);

								for( int drawX = 0; drawX < solvedSize.x * width && posXInt + drawX < width; drawX++ )
								{
									for( int drawY = 0; drawY < solvedSize.y * height && posYInt + drawY < height; drawY++ )
									{
										img.setPixel(drawX+posXInt, drawY+posYInt, qRgb( (int)pix.x, (int)pix.y, (int)pix.z ) );
									} 

								}
							}

							//std::cout << "Saving Frame " << frame << " of " <<  int(endTime/interv) << std::endl;

							img.save( prepend + "FRAME" + QString::number( frame + 100000 ) + ".jpg", 0, 100 );
						}
						frame--;
		

						//std::cout << "Writing Video" << std::endl;

						cv::VideoCapture capt;
						cv::VideoWriter writ;
		
						capt.open( ( prepend + "FRAME1%05d.jpg" ).toStdString().c_str() );
						writ.open( workingString.toStdString().c_str(), CV_FOURCC( 'M', 'J', 'P', 'G' ), fps, cv::Size(width, height) );
		
						cv::Mat ferry;
						std::cout << writ.isOpened();

						if( writ.isOpened() )
						{
							actionString = "Writing Images to Video";

							int fCount = 0;
							while( capt.read( ferry ) && fCount <= frame )
							{
								actualTime += tim.interval();
								frameString = tr( std::to_string( fCount + 1 ).c_str() ) + " of " + tr( std::to_string( frame + 1 ).c_str() );
								timeString = tr( std::to_string( (int)(actualTime) ).c_str() ) + "s";
								estString = "N/A";
								showProgress = 100 * float(fCount) / int(frame + 1);

								//std::cout << "Writing Frame " << fCount << std::endl;
								writ.write( ferry );
								fCount++;
							}

							//std::cout << "Writing Closed" << std::endl;

							capt.release();
							writ.release();
							//CvVideoWriter * writer = new CvVideoWriter();
						}

						actionString = "Done -- Waiting for New Job";
						frameString = "All " + tr( std::to_string( frame + 1 ).c_str() ) + " finished";
						showProgress = 100;
					}
					catch( char * exception )
					{
						exception;
						actionString = "An error has occurred -- diagnose in scene editor";
					}
				}
				else
				{
					actionString = "An error has occurred -- Animation does not exist or is too short";
				}
				isRendering = false;
			}
			else
			{
				actionString = "Empty Scene -- cannot render";
			}
		});
		worker.detach();
		
	});
}