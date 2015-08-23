#include "ScriptArtist.h"

#include "Operator.h"
#include "Function.h"

#include "ValueNode.h"
#include "VariableNode.h"
#include "Block.h"
#include "StructureNode.h"
#include "FunctionNode.h"
#include "OperatorNode.h"
#include "EvaluationNode.h"
#include "EmptyNode.h"

#include "QtGui\qimage.h"
#include "QtGui\qpainter.h"
#include <cmath>

int NODE_BUBBLE_WIDTH = 72, NODE_BUBBLE_HEIGHT = 32;
	
//ScriptNode displayTree;


void drawThickLine( QPainter* g, int x1, int y1, int x2, int y2, int radius )
{
	for( int x = -radius; x < radius; x++ )
	{
		for( int y = -radius; y < radius; y++ )
		{
			if( x*x + y*y <= radius*radius )
				g->drawLine( x1, y1, x2, y2 );
		}
	}
}

void drawCenterredText( std::string text, QPainter* g, int x, int y )
{
	// Find the size of string s in font f in the current Graphics context g.
	//FontMetrics fm   = g.getFontMetrics(g.getFont());
	//java.awt.geom.Rectangle2D rect = fm.getStringBounds(text, g);

	int textHeight = 20;//(int)(rect.getHeight()); 
	int textWidth  = 60;//(int)(rect.getWidth());

	g->drawText( x - textWidth/2, y - textHeight/2, textWidth,textHeight, 0, QString( text.c_str() ) );
	//g.drawString(text, x - textWidth/2, y - textHeight/2 + fm.getAscent());  // Draw the string.
}

template< class NewType >
bool instanceOf( ScriptNode * old )
{
	NewType* v = dynamic_cast<NewType*>(old);
	return v!=0;
}

std::string getText( ScriptNode * mathNode )
{
	std::string result = "";
	if ( instanceOf<ValueNode>(mathNode) )
	{
		result = "V: " + ((ValueNode*)mathNode)->evaluate().toString();
	}
	else
	if ( instanceOf<Block>(mathNode) )
	{
		result = "BLOCK";
	}
	else
	if ( instanceOf<FunctionNode>(mathNode) )
	{
		result = ((FunctionNode*)mathNode)->getFunction()->getIdentifier()+"(...)";
	}
	else
	if ( instanceOf<VariableNode>(mathNode) )
	{
		result = "X: " + ((VariableNode*)mathNode)->getIdentifier();
	}
	else
	if ( instanceOf<StructureNode>(mathNode) )
	{
		result = "S: " + ((StructureNode*)mathNode)->getStructure()->getIdentifier();
	}
	else
	if ( instanceOf<EvaluationNode>(mathNode) )
	{
		result = "(EVAL)";
	}
	else
	if ( instanceOf<OperatorNode>(mathNode) )
	{
		result = "O: " + ((OperatorNode*)mathNode)->getOperator()->getSymbol();
	}
	else
	if ( instanceOf<EmptyNode>(mathNode) )
	{
		result = "EMPTY";
	}
	return result;
}

void getRowStep( ScriptNode * node, std::vector<ScriptNode*> * nodeList, int targetIndex, int currentIndex )
{
	if (currentIndex == targetIndex)
	{
		nodeList->push_back( node );
	}
	else
	{
		for( unsigned int i = 0; i < node->getChildren()->size(); i++ )
		{
			getRowStep( (*node->getChildren())[i], nodeList, targetIndex, currentIndex+1 );
		}
	}
}

std::vector<ScriptNode*> getRow( ScriptNode * tree, int index )
{
	std::vector<ScriptNode*> result;
	getRowStep( tree, &result, index, 0 );
	return result;
}
	
int getRowsStep( ScriptNode * node, int total )
{
	int largestTotal = total;
		
	for( unsigned int i = 0; i < node->getChildren()->size(); i++ )
	{
		ScriptNode * child = (*node->getChildren())[i];
		largestTotal = std::max( largestTotal, getRowsStep( child, total + 1 ) );
	}
		
	return largestTotal;
}

int getRows( ScriptNode * tree )
{
	return getRowsStep( tree, 1 );
}

std::vector<std::vector<ScriptNode*>> getNodesBiArray( ScriptNode * tree )
{
	std::vector<std::vector<ScriptNode*>> result;
	int rows = getRows( tree );

	for( int i = 0; i < rows; i++ )
	{
		result.push_back( getRow( tree, i ) );
	}
	return result;
}


QImage* ScriptArtist::display( ScriptNode * script )
{
	QImage * result = new QImage( 512, 512, QImage::Format_ARGB32 );
	QPainter g( result );
	int count = 0;

	std::vector<std::vector<ScriptNode*>> nodes = getNodesBiArray( script );
	double ypadding = result->height()/(nodes.size()+1.0);
	for( unsigned int i = 0; i < nodes.size(); i++ )
	{
		int lineCount = 0;
		double xpadding = result->width()/(nodes[i].size()+1.0);
		for( unsigned int j = 0; j < nodes[i].size(); j++ )
		{
			int bubbleX = std::floor((j+1)*xpadding); //round
			int bubbleY = std::floor((i+1)*ypadding); //round
					
			g.setPen( QColor( 0,0,0 ) );
			for( unsigned int k = 0; k < nodes[i][j]->getChildren()->size(); k++ )
			{
				lineCount++;
				double nextXPadding =  result->width()/(nodes[i+1].size()+1.0);
				int nextX = std::floor(lineCount*nextXPadding); //round
				int nextY = std::floor((i+2)*ypadding); //round
				count++;
				g.setPen( QColor( 0,50,0 ) );
				//g.setColor( new Color( Color.HSBtoRGB( (count / 1.34f ) % 1, 1, 0.75f ) ) );
				drawThickLine( &g, bubbleX, bubbleY, nextX, nextY, 4 );
			}
			//g.setFont( new Font( "arial", Font.PLAIN, 12 ) );
			g.setPen( QColor( 255,255,255 ) );
			g.fillRect( bubbleX-NODE_BUBBLE_WIDTH/2, bubbleY-NODE_BUBBLE_HEIGHT/2, NODE_BUBBLE_WIDTH, NODE_BUBBLE_HEIGHT, QColor( 255,255,255 ) );
			g.setPen( QColor( 0,0,0 ) );
			g.drawRect( bubbleX-NODE_BUBBLE_WIDTH/2, bubbleY-NODE_BUBBLE_HEIGHT/2, NODE_BUBBLE_WIDTH, NODE_BUBBLE_HEIGHT );
			drawCenterredText( getText( nodes[i][j] ), &g, bubbleX, bubbleY );
		}
	}

	return result;
}