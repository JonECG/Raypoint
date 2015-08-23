#include "ScriptParser.h"

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

ScriptParser::ScriptParser()
{
}

void ScriptParser::registerOperator( Operator * op )
{
	operators[ op->getSymbol() ] = op;
}
void ScriptParser::registerStructure( Structure* structure )
{
	structures[ structure->getIdentifier() ] = structure;
}
void ScriptParser::registerFunction( Function * funct )
{
	functions[ funct->getIdentifier() ] = funct;
}

Block * ScriptParser::parse( std::string function )
{
	return parseBlock( function, 0, function.length() );
}
Block * ScriptParser::parseBlock( std::string block, int begin, int end )
{
	//System.out.println( "Block: |" + block.substring( begin, end ) + "|" );
	Block * b = new Block();
		
	int lastIndex = begin, index = begin;
		
	int parends = 0;
		
	while( index <= end )
	{
		char current = 0;
		if( index < end )
		{
			current = block.at( index );
				
			if( current == '(' )
				parends++;
			if( current == ')' )
				parends--;
				
			if( parends < 0 )
				throw "Unexpected symbol ')'";
		}
			
		if( parends == 0 )
		{
		
			if( index == end || current == ';' /* || current == '\n' || current == '\r' */ ) //End of line
			{
				if( index != lastIndex ) //Only if there is substantial work
				{
					b->addChild( parseLine( block, lastIndex, index, nullptr ) );
				}
				lastIndex = index+1;
			}
				
			if( current == '{' ) //Start of an inner scope
			{
				int scopes = 1;
				int beginBlockInd = index;
				index++;
					
				while( index < end && scopes != 0 )
				{
					if( block.at( index ) == '}' )
						scopes--;
					if( block.at( index ) == '{' )
						scopes++;
					index++;
				}
				if( scopes != 0 )
				{
					throw "Unexpected end of block";
				}
				Block * after = parseBlock( block, beginBlockInd + 1, index - 1 );
				if( beginBlockInd != lastIndex )
					b->addChild( parseLine( block, lastIndex, beginBlockInd, after ) );
				else
					b->addChild( after );
				lastIndex = index;
			}
			
		}
			
		index++;
	}
		
	if( parends > 0 )
		throw "Matching parend not found";
		
	return b;
}

int getCharacterClass( char c )
{
	if ( ( c >= 'A' && c <= 'z' ) || ( c == '_' ) )
		return 1;
	if ( c >= '0' && c <= '9' )
		return 2;
	if ( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
		return 4;
	return 0;
}

bool isValidIdentifier( std::string containing, int begin, int end )
{
	if( end-begin == 0 )
		return false;
		
	int index = begin;
		
	bool result = true;
		
	while( index < end && result )
	{
		char c = containing.at( index );
			
		result = ( c >= 'A' && c <= 'z' ) || ( c == '_' );//|| ( index != begin && c >= '0' && c <= '9' );
		index++;
	}
		
	return result;
}

bool isValidNumber( std::string containing, int begin, int end )
{
	std::string sub = containing.substr( begin, end - begin );
	try
	{
		std::stof( sub );
	}
	catch( std::invalid_argument )
	{
		return false;
	}
	
	return true;
}

ScriptNode * ScriptParser::parseLine( std::string line )
{
	return parseLine( line, 0, line.size(), nullptr );
}

ScriptNode * ScriptParser::parseLine( std::string line, int begin, int end, ScriptNode* procedingExpression )
{
	//System.out.println( "\tLine: |" + line.substring( begin, end ) + "|" );
	std::vector<ScriptNode*> tokens;
	//ArrayList<ScriptNode> tokens = new ArrayList<ScriptNode>();
		
	int index = begin, lastIndex = begin;
		
	int followType = -1;
		
	while( index <= end )
	{
		int currentType = 0;
		char current = 0;
			
		if( index < end ) //Janky so that it looks again when reaching the end
		{
			current = line.at( index );
			currentType = getCharacterClass( current );
			//System.out.println( currentType );
		}
			
		if( index == end || current == ' ' || ( followType != -1 && currentType != followType ) ) //Separation
		{
			//System.out.println( "Buzz: " + (index - begin) );

			bool wasMade = false;
			if( index > lastIndex ) //Only if there is substantial work
			{
				std::string s = line.substr( lastIndex, index-lastIndex );
					
				//System.out.println(s);
				if( operators.find( s ) != operators.end() ) //see if it was an operator
				{
					tokens.push_back( new OperatorNode( operators[ s ] ) );
					wasMade = true;
				}
				else
				if( isValidIdentifier( line, lastIndex, index ) ) //Can be a structure, function, or variable
				{
					wasMade = true;
						
					if( structures.find( s ) != structures.end() ) //Is directive structure
					{
							
						while( index < end && line.at( index ) != '(' )
						{
							index++;
						}
						int parends = 1;
						int beginInd = index + 1;
						while( index < end && parends != 0 )
						{
							if( line.at( index ) == '(' )
								parends++;
							if( line.at( index ) == ')' )
								parends--;
							index++;
						}
						//System.out.println( "inner: " + line.substring( beginInd, index - 2 ));
						Block * inner = parseBlock( line, beginInd, index - 2 );
							
						ScriptNode * after = ( ( procedingExpression == nullptr ) ? parseLine( line, index, end, procedingExpression ) : procedingExpression );
						StructureNode * st = new StructureNode( structures[ s ] );
						st->setInner( inner );
						st->addChild( after );
						tokens.push_back( st );
						index = end;
					}
					else
					if( functions.find( s ) != functions.end() ) //Is function
					{
						while( index < end && line.at( index ) != '(' )
						{
							index++;
						}

						int parends = 1;
						int beginInd = index + 1;
						index++;

						FunctionNode * func = new FunctionNode( functions[ s ] );

						while( index < end && parends != 0 )
						{
							if( line.at( index ) == '(' )
								parends++;
							if( line.at( index ) == ')' )
								parends--;

							if( line.at( index ) == ',' && parends == 1 ) //comma separated params
							{
								func->addChild( parseLine( line, beginInd, index, nullptr ) );
								beginInd = index + 1;
							}

							index++;
						}
						//System.out.println( "inner: " + line.substring( beginInd, index - 2 ));
						func->addChild( parseLine( line, beginInd, index - 1, nullptr ) );
						index--;

						tokens.push_back( func );
					}
					else //is a variable
					{
						tokens.push_back( new VariableNode( s ) );
					}
				}
				else //Maybe a number
				if( isValidNumber( line, lastIndex, index ) )
				{
					//System.out.println( "ADDED NUM" );
					tokens.push_back( new ValueNode( std::stof( s ) ) );
					wasMade = true;
				}
			}
			if( wasMade )
				lastIndex = index;
			else
			{
				if( current == '(' )
				{
					//System.out.println( "found" );
					int parends = 1;
					index++;
					int beginInd = index;
						
					while( index < end && parends != 0 )
					{
						if( line.at( index ) == '(' )
							parends++;
						if( line.at( index ) == ')' )
							parends--;
						index++;
					}
						
					EvaluationNode * eval = new EvaluationNode();
					eval->addChild( parseLine( line, beginInd, index - 1, nullptr ) );
					tokens.push_back( eval );
					//followType = -1;
					index--;
					//lastIndex = index+1;
				}
				lastIndex = index;
			}
		}
			
		followType = currentType;
		index++;
	}
		
		
	ScriptNode ** tokensArray = reinterpret_cast<ScriptNode**>( malloc( sizeof(ScriptNode*) * tokens.size() ) );
	for( unsigned int i = 0; i < tokens.size(); i++ )
	{
		tokensArray[i] = tokens[i];
	}
	//System.out.println( line.substring( begin, end ) + " ~~ " + ( (procedingExpression==null)? "noexp" : "exp" ) );
	return parseTokensRec( tokensArray, 0, tokens.size() );
}
ScriptNode * ScriptParser::parseTokensRec( ScriptNode ** tokens, int begin, int end )
{
	//System.out.println( "\t\tTokens found: " + ( end - begin ) );
	if( end - begin == 1 )
		return tokens[begin];
	if( end - begin == 0 )
		return new EmptyNode();//throw new RuntimeException( "Weird, no tokens left" );
		
	int lowestPriority = tokens[begin]->getPriority();
	int lowestIndex = begin;
		
	int index = begin+1;
	while( index < end )
	{
		if( tokens[index]->getPriority() < lowestPriority )
		{
			lowestPriority = tokens[index]->getPriority();
			lowestIndex = index;
		}
		index++;
	}
		
	tokens[lowestIndex]->addChild( parseTokensRec( tokens, begin, lowestIndex ) );
	tokens[lowestIndex]->addChild( parseTokensRec( tokens, lowestIndex + 1, end ) );
	return tokens[lowestIndex];
}

#include "Structure.h"
#include <functional>
ScriptParser * ScriptParser::makeRegular()
{
	ScriptParser * parser = new ScriptParser();

		//postfix	expr++ expr--
	//parser->registerOperator( new Operator( "++", 110 ) );
	//parser->registerOperator( new Operator( "--", 110 ) );
		
		//unary	++expr --expr +expr -expr ~ !
		//parser->registerOperator( new Operator( "++", 100 ) );
		//parser->registerOperator( new Operator( "--", 100 ) );
		//parser->registerOperator( new Operator( "+", 100 ) );
		//parser->registerOperator( new Operator( "-", 100 ) );
		//parser->registerOperator( new Operator( "~", 100 ) );
	//parser->registerOperator( new Operator( "!", 100 ) );
		
		//multiplicative	* / %
		parser->registerOperator( new Operator( "*", 90 ) );
		parser->registerOperator( new Operator( "/", 90 ) );
	//parser->registerOperator( new Operator( "%", 90 ) );
		
		//additive	+ -
		parser->registerOperator( new Operator( "+", 80 ) );
		parser->registerOperator( new Operator( "-", 80 ) );
		
		//shift	<< >> >>>
		//parser->registerOperator( new Operator( "<<", 70 ) );
		//parser->registerOperator( new Operator( ">>", 70 ) );
		//parser->registerOperator( new Operator( ">>>", 70 ) );
		
		//relational	< > <= >= instanceof
		parser->registerOperator( new Operator( "<", 60 ) );
		parser->registerOperator( new Operator( ">", 60 ) );
		parser->registerOperator( new Operator( "<=", 60 ) );
		parser->registerOperator( new Operator( ">=", 60 ) );
		//parser->registerOperator( new Operator( "typeof", 60 ) );
		
		//equality	== !=
		parser->registerOperator( new Operator( "==", 50 ) );
		parser->registerOperator( new Operator( "!=", 50 ) );
		
		//bitwise AND	&
		//parser->registerOperator( new Operator( "&", 40 ) );
		
		//bitwise exclusive OR	^
		//parser->registerOperator( new Operator( "^", 40 ) );
		
		//bitwise inclusive OR	|
		//parser->registerOperator( new Operator( "|", 40 ) );
		
		//logical AND	&&
		parser->registerOperator( new Operator( "&&", 30 ) );
		
		//logical OR	||
		parser->registerOperator( new Operator( "||", 30 ) );
		
		//ternary	? :
		parser->registerOperator( new Operator( "?", 20 ) );
		parser->registerOperator( new Operator( ":", 20 ) );
		
		//assignment	= += -= *= /= %= &= ^= |= <<= >>= >>>=
		parser->registerOperator( new Operator( "=", 10 ) );
		parser->registerOperator( new Operator( "+=", 10 ) );
		parser->registerOperator( new Operator( "-=", 10 ) );
		parser->registerOperator( new Operator( "*=", 10 ) );
		parser->registerOperator( new Operator( "/=", 10 ) );
		parser->registerOperator( new Operator( "%=", 10 ) );
		//parser->registerOperator( new Operator( "&=", 10 ) );
		//parser->registerOperator( new Operator( "^=", 10 ) );
		//parser->registerOperator( new Operator( "|=", 10 ) );
		//parser->registerOperator( new Operator( "<<=", 10 ) );
		//parser->registerOperator( new Operator( ">>=", 10 ) );
		//parser->registerOperator( new Operator( ">>>=", 10 ) );
		
		//return
		parser->registerOperator( new Operator( "return", 0 ) );

/////////////////////  STRUCTURES  ////////////////////////////

		parser->registerStructure( new Structure( "if", []( StructureNode* stru ){ return Value(); }, []( StructureNode* stru ){ return false; } ) );
		parser->registerStructure( new Structure( "for", []( StructureNode* stru ){ return Value(); }, []( StructureNode* stru ){ return false; } ) );
		parser->registerStructure( new Structure( "while", []( StructureNode* stru ){ return Value(); }, []( StructureNode* stru ){ return false; } ) );

		parser->registerFunction( new Function( "fabs" ) );
		parser->registerFunction( new Function( "sqrt" ) );
		parser->registerFunction( new Function( "min" ) );
		parser->registerFunction( new Function( "max" ) );
		parser->registerFunction( new Function( "pow" ) );
		parser->registerFunction( new Function( "fmod" ) );
		parser->registerFunction( new Function( "clamp" ) );

	return parser;
}

template< class NewType >
bool instanceOf( ScriptNode * old )
{
	NewType* v = dynamic_cast<NewType*>(old);
	return v!=0;
}

#include <iostream>

std::string ScriptParser::textOf( ScriptNode * node, int indents, bool curly )
{
	std::string tabs = "";
	for( int i = 0; i < indents; i++ )
	{
		tabs += "\t";
	}
		
	std::string result = "";
		
	if( instanceOf<Block>(node) )
	{
		if( curly )
			result = tabs+"{\n";
		for( unsigned int i = 0; i < node->children.size(); i++ )
		{
			if( curly )
				result += tabs + "\t" + textOf( node->children[ i ], indents+1, true ) + ";\n";
			else
			{
				result += textOf( node->children[ i ], indents+1, true );
				if( i != node->children.size()-1 )
					result +=  "; ";
			}
		}
		if( curly )
			result += tabs + "}\n";
	}

	if( instanceOf<FunctionNode>(node) )
	{
		result = ((FunctionNode*)node)->getFunction()->getIdentifier() + "(";

		for( unsigned int i = 0; i < node->children.size(); i++ )
		{
			result += textOf( node->children[ i ], indents+1, false );

			if( i < node->children.size() - 1 )
				result += ",";
		}
		result += ")";
	}
		
	if( instanceOf<OperatorNode>(node) )
	{
		result = textOf( node->children[ 0 ], indents, true ) + " " + ((OperatorNode*)node)->getOperator()->getSymbol() + " " + textOf( node->children[ 1 ], indents, true );
	}
		
	if( instanceOf<VariableNode>(node) )
	{
		bool found = false;
			
		VariablePacket vp;

		Block * scope = node->findClosestScope();
			
		while( !found && scope != nullptr )
		{
			found = scope->variablesInScope.find( ((VariableNode*)node)->getIdentifier() ) != scope->variablesInScope.end();
			if( found )
				vp = scope->variablesInScope[ ((VariableNode*)node)->getIdentifier() ];
			//val = scope->variablesInScope[ ((VariableNode*)node)->getIdentifier() ];
			scope = ( scope->parent == nullptr ) ? nullptr : scope->parent->findClosestScope();
		}
			
		if( !found )
		{
			result = "frac " + ((VariableNode*)node)->getIdentifier();
			
			std::cout << "before: " << node->findClosestScope()->variablesInScope.size();
			node->findClosestScope()->variablesInScope[ ((VariableNode*)node)->getIdentifier() ] = VariablePacket( Value(0) );
			std::cout << "after: " << node->findClosestScope()->variablesInScope.size() << std::endl;
		}
		else
		{
			if( vp.isReferenceType() )
				result = "(*"+((VariableNode*)node)->getIdentifier() + ")";
			else
				result = ((VariableNode*)node)->getIdentifier();
		}
	}
		
	if( instanceOf<ValueNode>(node) )
	{
		result = ((ValueNode*)node)->evaluate().toString();
	}
		
	if( instanceOf<EvaluationNode>(node) )
	{
		result = "(" + textOf( node->children[ 0 ], indents, true ) + ")";
	}
		
	if( instanceOf<StructureNode>(node) )
	{
		std::string inner = textOf( ((StructureNode*)node)->getInner(), indents, false );
		std::string block = textOf( node->children[ 0 ], indents + 1, true );
		result = ((StructureNode*)node)->getStructure()->getIdentifier() + "(" + inner + ")\n" + tabs + "{\n" + block + "}";
	}
		
	return result;
}

