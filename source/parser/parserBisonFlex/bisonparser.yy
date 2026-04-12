%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define api.parser.class {genesyspp_parser} //Name of the parses class
%define api.token.constructor //let that way or change YY_DECL prototype
%define api.value.type variant
%define parse.assert //Checks for constructor and destructor(?)
%code requires
{
	#include <string>
	#include <cmath>
	#include <algorithm>
	#include "obj_t.h"
	#include "../kernel/util/Util.h"
	#include "../kernel/simulator/Attribute.h"
	#include "../kernel/simulator/Counter.h"

	/****begin_Includes_plugins****/

		/**begin_Includes:Variable**/
		#include "../plugins/data/Variable.h"
		/**end_Includes:Variable**/

		/**begin_Includes:Queue**/
		#include "../plugins/data/Queue.h"
		/**end_Includes:Queue**/

		/**begin_Includes:Formula**/
		#include "../plugins/data/Formula.h"
		/**end_Includes:Formula**/

		/**begin_Includes:Resource**/
		#include "../plugins/data/Resource.h"
		/**end_Includes:Resource**/

		/**begin_Includes:Set**/
		#include "../plugins/data/Set.h"
		/**end_Includes:Set**/

	/****end_Includes_plugins****/

#ifdef YYDEBUG
  yydebug = 1;
#endif

	class genesyspp_driver;
}

//%define api.value.type {obj_t} //c++ types for semantic values
 //c++ types for semantic values
// The parsing context.
%param { genesyspp_driver& driver } //aditional params for yylex/yyparse
%locations // allows for more accurate syntax error messages.
%initial-action
{
  // Initialize the initial location.
  //@$.begin.filename = @$.end.filename = &driver.getFile();
};
 //for debugging
%define parse.error verbose //error level to show
%code
{
# include "Genesys++-driver.h"
# include <exception>

namespace {

std::string buildProbError(const std::string& functionName, const std::string& details) {
	return std::string("Error evaluating ") + functionName + ": " + details;
}

void failProbFunction(genesyspp_driver& driver, const std::string& message) {
	driver.setErrorMessage(message);
	driver.setResult(-1);
}

}

}

// numbers
%token <obj_t> NUMD
%token <obj_t> NUMH
%token <obj_t> CTEZERO

// relational operators
%token <obj_t> oLE
%token <obj_t> oGE
%token <obj_t> oEQ
%token <obj_t> oNE

// logic operators
%token <obj_t> oAND
%token <obj_t> oOR
%token <obj_t> oNAND
%token <obj_t> oXOR
%token <obj_t> oNOT

// trigonometric functions
%token <obj_t> fSIN
%token <obj_t> fCOS

// math functions
%token <obj_t> fROUND
%token <obj_t> fMOD
%token <obj_t> fTRUNC
%token <obj_t> fFRAC
%token <obj_t> fEXP
%token <obj_t> fSQRT
%token <obj_t> fLOG
%token <obj_t> fLN
%token <obj_t> mathMIN
%token <obj_t> mathMAX

// string functions
%token <obj_t> fVAL
%token <obj_t> fEVAL
%token <obj_t> fLENG

// probability distributionsformulaValue
%token <obj_t> fRND1
%token <obj_t> fEXPO
%token <obj_t> fNORM
%token <obj_t> fUNIF
%token <obj_t> fWEIB
%token <obj_t> fLOGN
%token <obj_t> fGAMM
%token <obj_t> fERLA
%token <obj_t> fTRIA
%token <obj_t> fBETA
%token <obj_t> fDISC

// simulation infos
%token <obj_t> fTNOW
%token <obj_t> fTFIN
%token <obj_t> fMAXREP
%token <obj_t> fNUMREP
%token <obj_t> fIDENT
%token <obj_t> simulEntitiesWIP

// algoritmic functions
%token <obj_t> cIF
%token <obj_t> cELSE
%token <obj_t> cFOR
%token <obj_t> cTO
%token <obj_t> cDO

// kernel elements
%token <obj_t> ATRIB
%token <obj_t> CSTAT
%token <obj_t> COUNTER
%token <obj_t> SIMRESP
%token <obj_t> SIMCTRL

// kernel elements' functions
%token <obj_t> fTAVG
%token <obj_t> fCOUNT

// not found, wrong, illegal
%token <obj_t> ILLEGAL     /* illegal token */

/****begin_Tokens_plugins****/

	/**begin_Tokens:Resource**/
	%token <obj_t> RESOURCE
	%token <obj_t> fNR
	%token <obj_t> fMR
	%token <obj_t> fIRF
	%token <obj_t> fRESSEIZES
	%token <obj_t> fSTATE
	%token <obj_t> fSETSUM
	%token <obj_t> fRESUTIL
	/**end_Tokens:Resource**/

	/**begin_Tokens:Queue**/
	%token <obj_t> QUEUE
	%token <obj_t> fNQ
	%token <obj_t> fFIRSTINQ
	%token <obj_t> fLASTINQ
	%token <obj_t> fSAQUE
	%token <obj_t> fAQUE
	%token <obj_t> fENTATRANK
	/**end_Tokens:Queue**/

	/**begin_Tokens:Set**/
	%token <obj_t> SET
	%token <obj_t> fNUMSET
	/**end_Tokens:Set**/

	/**begin_Tokens:Variable**/
	%token <obj_t> VARI
	/**end_Tokens:Variable**/

	/**begin_Tokens:Formula**/
	%token <obj_t> FORM
	/**end_Tokens:Formula**/

	/**begin_Tokens:EntityGroup**/
	%token <obj_t> fNUMGR
	%token <obj_t> fATRGR
	/**end_Tokens:EntityGroup**/

/****end_Tokens_plugins****/

%token LPAREN "("
%token RPAREN ")"
%token LBRACKET "["
%token RBRACKET "]"
%token PLUS "+"
%token MINUS "-"
%token STAR "*"
%token POWER "^"
%token SLASH "/"
%token LESS "<"
%token GREATER ">"
%token ASSIGN "="
%token COMMA ","
%token END 0 "end of file" //need to declare, as bison doesnt in especific situation

%type <obj_t> input
%type <obj_t> expression
%type <obj_t> primary
%type <obj_t> unary
%type <obj_t> power
%type <obj_t> multiplicative
%type <obj_t> additive
%type <obj_t> relational
%type <obj_t> logicalNot
%type <obj_t> logicalAnd
%type <obj_t> logicalXor
%type <obj_t> logicalOr
%type <obj_t> command
%type <obj_t> commandIF
%type <obj_t> commandFOR
%type <obj_t> function
%type <obj_t> number
%type <obj_t> attribute
%type <obj_t> simulationResponse
%type <obj_t> simulationControl
%type <obj_t> assigment
%type <obj_t> kernelFunction
%type <obj_t> trigonFunction
%type <obj_t> mathFunction
%type <obj_t> probFunction
%type <obj_t> pluginFunction
%type <obj_t> userFunction
%type <obj_t> elementFunction
%type <obj_t> listaparm
%type <obj_t> illegal

/****begin_TypeObj_plugins****/

	/**begin_TypeObj::Variable**/
	%type <obj_t> variable
	/**end_TypeObj::Variable**/

	/**begin_TypeObj::Formula**/
	%type <obj_t> formula
	/**end_TypeObj::Formula**/

/****end_TypeObj_plugins****/

//%printer { yyoutput << $$; } <*>; //prints when something
%%

input: 
      expression    { driver.setResult($1.valor);}
//    | error '\n'        { yyerrok; }
    ;

expression:
      assigment                        {$$.valor = $1.valor;}
    | command                          {$$.valor = $1.valor;}
    | logicalOr                        {$$.valor = $1.valor;}
    | illegal                           {$$.valor = -1;}
    ;

logicalOr:
      logicalOr oOR logicalXor          { $$.valor = (int)$1.valor || (int)$3.valor; }
    | logicalXor                        { $$.valor = $1.valor; }
    ;

logicalXor:
      logicalXor oXOR logicalAnd        { $$.valor = (!(int)$1.valor && (int)$3.valor) || ((int)$1.valor && !(int)$3.valor); }
    | logicalAnd                        { $$.valor = $1.valor; }
    ;

logicalAnd:
      logicalAnd oAND logicalNot        { $$.valor = (int)$1.valor && (int)$3.valor; }
    | logicalAnd oNAND logicalNot       { $$.valor = !((int)$1.valor && (int)$3.valor); }
    | logicalNot                        { $$.valor = $1.valor; }
    ;

logicalNot:
      oNOT logicalNot                   { $$.valor = !(int)$2.valor; }
    | relational                        { $$.valor = $1.valor; }
    ;

relational:
      relational LESS additive          { $$.valor = $1.valor < $3.valor ? 1 : 0; }
    | relational GREATER additive       { $$.valor = $1.valor > $3.valor ? 1 : 0; }
    | relational oLE additive           { $$.valor = $1.valor <= $3.valor ? 1 : 0; }
    | relational oGE additive           { $$.valor = $1.valor >= $3.valor ? 1 : 0; }
    | relational oEQ additive           { $$.valor = $1.valor == $3.valor ? 1 : 0; }
    | relational oNE additive           { $$.valor = $1.valor != $3.valor ? 1 : 0; }
    | additive                          { $$.valor = $1.valor; }
    ;

additive:
      additive PLUS multiplicative      { $$.valor = $1.valor + $3.valor; }
    | additive MINUS multiplicative     { $$.valor = $1.valor - $3.valor; }
    | multiplicative                    { $$.valor = $1.valor; }
    ;

multiplicative:
      multiplicative STAR power         { $$.valor = $1.valor * $3.valor; }
    | multiplicative SLASH power        { $$.valor = $1.valor / $3.valor; }
    | power                             { $$.valor = $1.valor; }
    ;

power:
      unary POWER power                 { $$.valor = pow($1.valor, $3.valor); }
    | unary                             { $$.valor = $1.valor; }
    ;

unary:
      MINUS unary                        { $$.valor = -$2.valor; }
    | PLUS unary                         { $$.valor = +$2.valor; }
    | primary                            { $$.valor = $1.valor; }
    ;

primary:
      number                             {$$.valor = $1.valor;}
    | function                           {$$.valor = $1.valor;}
    | LPAREN expression RPAREN           {$$.valor = $2.valor;}
    | attribute                          {$$.valor = $1.valor;}
    | simulationResponse                 {$$.valor = $1.valor;}
    | simulationControl                  {$$.valor = $1.valor;}

/****begin_Expression_plugins****/

	/**begin_Expression:Variable**/
	| variable                            {$$.valor = $1.valor;}
	/**end_Expression:Variable**/

	/**begin_Expression:Formula**/
	| formula                             {$$.valor = $1.valor;}
	/**end_Expression:Formula**/

/****end_Expression_plugins****/
    ;

number:
      NUMD                               { $$.valor = $1.valor;}
    | NUMH                               { $$.valor = $1.valor;}
    ;

command:
      commandIF	    { $$.valor = $1.valor; }
    | commandFOR    { $$.valor = $1.valor; }
    ;

commandIF:
      cIF "(" expression "," expression "," expression ")" { $$.valor = $3.valor != 0 ? $5.valor : $7.valor; }
    | cIF "(" expression "," expression ")"                 { $$.valor = $3.valor != 0 ? $5.valor : 0; }
    ;

// \todo: check for function/need, for now will let cout (these should be commands for program, not expression
commandFOR: 
     cFOR variable "=" expression cTO expression cDO assigment  {$$.valor = 0; }
    | cFOR attribute "=" expression cTO expression cDO assigment  {$$.valor = 0; }
    ;

function: 
     mathFunction        { $$.valor = $1.valor; }
    | trigonFunction     { $$.valor = $1.valor; }
    | probFunction       { $$.valor = $1.valor; }
    | kernelFunction     { $$.valor = $1.valor; }
    | elementFunction    { $$.valor = $1.valor; }
    | pluginFunction     { $$.valor = $1.valor; }
    | userFunction       { $$.valor = $1.valor; }
    ;

kernelFunction:
      fTNOW      { $$.valor = driver.getModel()->getSimulation()->getSimulatedTime();}
    | fTFIN      { $$.valor = driver.getModel()->getSimulation()->getReplicationLength();}
    | fMAXREP    { $$.valor = driver.getModel()->getSimulation()->getNumberOfReplications();}
    | fNUMREP    { $$.valor = driver.getModel()->getSimulation()->getCurrentReplicationNumber();}
    | fIDENT     { $$.valor = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getId();}
	| simulEntitiesWIP  { $$.valor = driver.getModel()->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Entity>());}
	;

elementFunction:
    //| CSTAT		 { $$.valor = 0; }
      fTAVG  "(" CSTAT ")"     {
                    StatisticsCollector* cstat = ((StatisticsCollector*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<StatisticsCollector>(), $3.id)));
                    double value = cstat->getStatistics()->average();
                    $$.valor = value; }
	| fCOUNT "(" COUNTER ")" {
					Counter* counter = ((Counter*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Counter>(), $3.id)));
                    double value = counter->getCountValue();
                    $$.valor = value; }
   ;

trigonFunction:
      fSIN   "(" expression ")"   { $$.valor = sin($3.valor); }
    | fCOS   "(" expression ")"   { $$.valor = cos($3.valor); }
    ;

mathFunction:
      fROUND "(" expression ")"		{ $$.valor = round($3.valor);}
    | fFRAC  "(" expression ")"		{ $$.valor = $3.valor - (int) $3.valor;}
    | fTRUNC "(" expression ")"		{ $$.valor = trunc($3.valor);}
    | fEXP "(" expression ")"	    { $$.valor = exp($3.valor);}
    | fSQRT "(" expression ")"	    { $$.valor = sqrt($3.valor);}
    | fLOG "(" expression ")"	    { $$.valor = log10($3.valor);}
    | fLN "(" expression ")"	    { $$.valor = log($3.valor);}
    | fMOD   "(" expression "," expression ")" { $$.valor = (int) $3.valor % (int) $5.valor; }
    | mathMIN "(" expression "," expression ")" { $$.valor = std::min($3.valor, $5.valor); }
    | mathMAX "(" expression "," expression ")" { $$.valor = std::max($3.valor, $5.valor); }
    ;

probFunction:
	  fRND1					     {
		try { $$.valor = driver.getSampler()->sampleUniform(0.0,1.0); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("rnd", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("rnd", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating rnd: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fEXPO  "(" expression ")"  {
		try { $$.valor = driver.getSampler()->sampleExponential($3.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("expo", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("expo", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating expo: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fNORM  "(" expression "," expression ")"  {
		try { $$.valor = driver.getSampler()->sampleNormal($3.valor,$5.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("norm", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("norm", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating norm: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fUNIF  "(" expression "," expression ")"  {
		try { $$.valor = driver.getSampler()->sampleUniform($3.valor,$5.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("unif", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("unif", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating unif: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fWEIB  "(" expression "," expression ")"  {
		try { $$.valor = driver.getSampler()->sampleWeibull($3.valor,$5.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("weib", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("weib", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating weib: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fLOGN  "(" expression "," expression ")"  {
		try { $$.valor = driver.getSampler()->sampleLogNormal($3.valor,$5.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("logn", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("logn", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating logn: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fGAMM  "(" expression "," expression ")"  {
		try { $$.valor = driver.getSampler()->sampleGamma($3.valor,$5.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("gamm", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("gamm", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating gamm: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fERLA  "(" expression "," expression ")"  {
		try { $$.valor = driver.getSampler()->sampleErlang($3.valor,$5.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("erla", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("erla", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating erla: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fTRIA  "(" expression "," expression "," expression ")"   {
		try { $$.valor = driver.getSampler()->sampleTriangular($3.valor,$5.valor,$7.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("tria", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("tria", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating tria: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fBETA  "(" expression "," expression "," expression "," expression ")"  {
		try { $$.valor = driver.getSampler()->sampleBeta($3.valor,$5.valor,$7.valor,$9.valor); }
		catch (const std::exception& e) {
			std::string msg = buildProbError("beta", e.what());
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (const std::string& e) {
			std::string msg = buildProbError("beta", e);
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		} catch (...) {
			std::string msg = "Error evaluating beta: unknown error";
			failProbFunction(driver, msg);
			if (driver.getThrowsException()) throw std::string(msg);
			YYERROR;
		}
	}
	| fDISC  "(" listaparm ")"                  { $$.valor = driver.getSampler()->sampleDiscrete(0,0); /*@TODO: NOT IMPLEMENTED YET*/ }
    ;


//Maybe user defined functions, check if continues on the parser, for now returns the value of expression
userFunction: 
      "USER" "(" expression ")"         { $$.valor = $3.valor; }
    ;

//Probably returns parameters for something, check if continues on the parser, for now does nothing
listaparm: 
      listaparm "," expression "," expression    {/*@TODO: NOT IMPLEMENTED YET*/}
    | expression "," expression                  {/*@TODO: NOT IMPLEMENTED YET*/}
    ;

//If illegal token, verifies if throws exception or set error message
illegal: 
	ILLEGAL           {
		driver.setResult(-1);
		std::string lexema = $1.tipo;
		bool hasLexema = !lexema.empty();
		std::string literalMsg = hasLexema ? std::string("Literal nao encontrado: \"") + lexema + "\"" : std::string("Literal nao encontrado");
		std::string caracterMsg = hasLexema ? std::string("Caracter invalido encontrado: \"") + lexema + "\"" : std::string("Caracter invalido encontrado");
		if(driver.getThrowsException()){
			if($1.valor == 0){
			  throw literalMsg;
			}else if($1.valor == 1){
			  throw caracterMsg;
			}
		} else {
			if($1.valor == 0){
			  driver.setErrorMessage(literalMsg);
			}else if($1.valor == 1){
				driver.setErrorMessage(caracterMsg);
			}
		}
	}
	;


// 20181003  ATRIB now returns the attribute ID not the attribute value anymore. So, now get the attribute value for the current entity
attribute:
	ATRIB      {  
		double attributeValue = 0.0;
		//std::cout << "Tentando..." << std::endl;
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue($1.id);
		}
		//std::cout << "Passei" << std::endl;
		$$.valor = attributeValue; 
	}
	| ATRIB LBRACKET expression RBRACKET  {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>($3.valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue($1.id, index);
		}
		$$.valor = attributeValue; 
	}
	| ATRIB LBRACKET expression "," expression RBRACKET  {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>($3.valor))+","+std::to_string(static_cast<unsigned int>($5.valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue($1.id, index);
		}
		$$.valor = attributeValue; 
	}
	| ATRIB LBRACKET expression "," expression "," expression RBRACKET  {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>($3.valor))+","+std::to_string(static_cast<unsigned int>($5.valor))+","+std::to_string(static_cast<unsigned int>($7.valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue($1.id, index);
		}
		$$.valor = attributeValue; 
	}
	;

simulationResponse:
	SIMRESP    {
		$$.valor = driver.getSimulationResponseValueAsDouble($1.tipo);
	}
	;

simulationControl:
	SIMCTRL    {
		$$.valor = driver.getSimulationControlValueAsDouble($1.tipo);
	}
	;

/****begin_ExpressionProdution_plugins****/

	/**begin_ExpressionProdution:Variable**/
	variable    : VARI  {$$.valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), $1.id)))->getValue();} 
				| VARI LBRACKET expression RBRACKET	    { 
					std::string index = std::to_string(static_cast<unsigned int>($3.valor));
					$$.valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), $1.id)))->getValue(index); }
				| VARI LBRACKET expression "," expression RBRACKET	    { 
					std::string index = std::to_string(static_cast<unsigned int>($3.valor))+","+std::to_string(static_cast<unsigned int>($5.valor)); 
					$$.valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), $1.id)))->getValue(index);}
				| VARI LBRACKET expression "," expression "," expression RBRACKET    { 
					std::string index = std::to_string(static_cast<unsigned int>($3.valor))+","+std::to_string(static_cast<unsigned int>($5.valor))+","+std::to_string(static_cast<unsigned int>($7.valor));
					$$.valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), $1.id)))->getValue(index);}
				;
	/**end_ExpressionProdution:Variable**/

	/**begin_ExpressionProdution:Formula**/
	// \todo: THERE IS A SERIOUS PROBLEM WITH FORMULA: TO EVALUATE THE FORMULA EXPRESSION, PARSER IS REINVOKED, AND THEN IT CRASHES (NO REENTRACE?)
	formula     : FORM	    { 
					std::string index = "";
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), $1.id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					$$.valor = value;}
				| FORM LBRACKET expression RBRACKET {
					std::string index = std::to_string(static_cast<unsigned int>($3.valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), $1.id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					$$.valor = value;}
				| FORM LBRACKET expression "," expression RBRACKET {
					std::string index = std::to_string(static_cast<unsigned int>($3.valor)) +","+std::to_string(static_cast<unsigned int>($5.valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), $1.id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					$$.valor = value;}
				| FORM LBRACKET expression "," expression "," expression RBRACKET {
					std::string index = std::to_string(static_cast<unsigned int>($3.valor)) +","+std::to_string(static_cast<unsigned int>($5.valor))+","+std::to_string(static_cast<unsigned int>($7.valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), $1.id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					$$.valor = value;}
				;
	/**end_ExpressionProdution:Formula**/
	/****end_ExpressionProdution_plugins****/

	//Check if want to set the attribute or variable with expression or just return the expression value, for now just returns expression value
	assigment  : ATRIB ASSIGN expression    { 
					// @TODO: getCurrentEvent()->getEntity() may be nullptr if simulation hasn't started yet
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue($1.id, $3.valor);
					$$.valor = $3.valor; }
				| ATRIB LBRACKET expression RBRACKET ASSIGN expression    { 
					std::string index = std::to_string(static_cast<unsigned int>($3.valor));
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue($1.id, $6.valor, index);
					$$.valor = $6.valor; }
				| ATRIB LBRACKET expression "," expression RBRACKET ASSIGN expression   {
					std::string index = std::to_string(static_cast<unsigned int>($3.valor))+","+std::to_string(static_cast<unsigned int>($5.valor)); 
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue($1.id, $8.valor, index);
					$$.valor = $8.valor;}
				| ATRIB LBRACKET expression "," expression "," expression RBRACKET ASSIGN expression      {
					std::string index = std::to_string(static_cast<unsigned int>($3.valor))+","+std::to_string(static_cast<unsigned int>($5.valor))+","+std::to_string(static_cast<unsigned int>($7.valor));
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue($1.id, $10.valor, index);
					$$.valor = $10.valor; }
	/****begin_Assignment_plugins****/
	/**begin_Assignment:Variable**/
				| VARI ASSIGN expression        {
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), $1.id)))->setValue($3.valor);
					$$.valor = $3.valor; 
					}
				| VARI LBRACKET expression RBRACKET ASSIGN expression    { 
					std::string index = std::to_string(static_cast<unsigned int>($3.valor));
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), $1.id)))->setValue($6.valor, index);
					$$.valor = $6.valor; }
				| VARI LBRACKET expression "," expression RBRACKET ASSIGN expression   {
					std::string index = std::to_string(static_cast<unsigned int>($3.valor))+","+std::to_string(static_cast<unsigned int>($5.valor)); 
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), $1.id)))->setValue($8.valor, index);
					$$.valor = $8.valor; }
				| VARI LBRACKET expression "," expression "," expression RBRACKET ASSIGN expression      {
					std::string index = std::to_string(static_cast<unsigned int>($3.valor))+","+std::to_string(static_cast<unsigned int>($5.valor))+","+std::to_string(static_cast<unsigned int>($7.valor));
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), $1.id)))->setValue($10.valor, index);
					$$.valor = $10.valor; }
	/**end_Assignment:Variable**/

/****end_Assignment_plugins****/
            ;


pluginFunction  : 
      CTEZERO                                        { $$.valor = 0; }
/**begin_FunctionProdution_plugins**/
/**begin_FunctionProdution:Queue**/
    | fNQ       "(" QUEUE ")"	    {   //std::cout << "Queue ID: " << $3.id << ", Size: " << ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->size() << std::endl; 
                                        $$.valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->size();}
    | fLASTINQ  "(" QUEUE ")"       {/*For now does nothing because need acces to list of QUEUE, or at least the last element*/ }
    | fFIRSTINQ "(" QUEUE ")"       { 
                if (((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->size() > 0){
                    //id da 1a entidade da fila, talvez pegar nome
                    $$.valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->first()->getEntity()->getId();
                }else{
                    $$.valor = 0;
                } }
    | fSAQUE "(" QUEUE "," ATRIB ")"   {   
                //Util::identification queueID = $3.id;
                Util::identification attrID = $5.id;
                double sum = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->sumAttributesFromWaiting(attrID);
                $$.valor = sum; }
    | fAQUE "(" QUEUE "," NUMD "," ATRIB ")" {
                //Util::identification queueID = $3.id;
                Util::identification attrID = $7.id;
                double value = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->getAttributeFromWaitingRank($5.valor-1, attrID); // rank starts on 0 in genesys
                $$.valor = value; }
/**end_FunctionProdution:Queue**/

/**begin_FunctionProdution:Resource**/
    | fMR        "(" RESOURCE ")"	{ $$.valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), $3.id))->getCapacity();}
    | fNR        "(" RESOURCE ")"        { $$.valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), $3.id))->getNumberBusy();}
    | fRESSEIZES "(" RESOURCE ")"        { /*\TODO: For now does nothing because needs get Seizes, check with teacher*/}
    | fSTATE     "(" RESOURCE ")"        {  $$.valor = static_cast<int>(((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), $3.id))->getResourceState()); }
    | fIRF       "(" RESOURCE ")"        { $$.valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), $3.id))->getResourceState() == Resource::ResourceState::FAILED ? 1 : 0; }
    | fSETSUM    "(" SET ")"  {
                unsigned int count=0;
                Resource* res;
                List<ModelDataDefinition*>* setList = ((Set*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(),$3.id))->getElementSet(); 
                for (std::list<ModelDataDefinition*>::iterator it = setList->list()->begin(); it!=setList->list()->end(); it++) {
                    res = dynamic_cast<Resource*>(*it);
                    if (res != nullptr) {
                        if (res->getResourceState()==Resource::ResourceState::BUSY) {
                            count++;
                        }
                    }
                }
                $$.valor = count; }
/**end_FunctionProdution:Resource**/

/**begin_FunctionProdution:Set**/
    | fNUMSET    "(" SET ")"	{ $$.valor = ((Set*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(),$3.id))->getElementSet()->size(); }

/**end_FunctionProdution:Set**/
/****end_FunctionProdution_plugins****/
    ;


%%
void
yy::genesyspp_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
