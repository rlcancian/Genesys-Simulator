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
%type <obj_t> arithmetic
%type <obj_t> logical
%type <obj_t> relacional
%type <obj_t> command
%type <obj_t> commandIF
%type <obj_t> commandFOR
%type <obj_t> function
%type <obj_t> number
%type <obj_t> attribute
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

%left oAND oOR;
%left oNOT;
%left oLE oGE oEQ oNE LESS GREATER LBRACKET cELSE;
%left MINUS PLUS;
%left STAR SLASH;
%precedence NEG;
%left fROUND fMOD fTRUNC fFRAC fLOG fLN fSQRT;

//%printer { yyoutput << $$; } <*>; //prints when something
%%

input: 
      expression    { driver.setResult($1.valor);}
//    | error '\n'        { yyerrok; }
    ;

expression:
      number                           {$$.valor = $1.valor;}
    | function                         {$$.valor = $1.valor;}
    | command                          {$$.valor = $1.valor;}
    | assigment                       {$$.valor = $1.valor;}
	| arithmetic                       {$$.valor = $1.valor;}
    | logical                           {$$.valor = $1.valor;}
    | relacional                       {$$.valor = $1.valor;}
	| LPAREN expression RPAREN          {$$.valor = $2.valor;}
    | attribute                         {$$.valor = $1.valor;}
    | illegal                           {$$.valor = -1;}

/****begin_Expression_plugins****/

	/**begin_Expression:Variable**/
		| variable                         {$$.valor = $1.valor;}
	/**end_Expression:Variable**/

	/**begin_Expression:Formula**/
		| formula                          {$$.valor = $1.valor;}
	/**end_Expression:Formula**/

/****end_Expression_plugins****/
    ;

number:
     NUMD     { $$.valor = $1.valor;}
    | NUMH    { $$.valor = $1.valor;}
    ;

arithmetic:
     expression PLUS expression      { $$.valor = $1.valor + $3.valor;}
    | expression MINUS expression    { $$.valor = $1.valor - $3.valor;}
    | expression SLASH expression    { $$.valor = $1.valor / $3.valor;}
    | expression STAR expression     { $$.valor = $1.valor * $3.valor;}
    | expression POWER expression    { $$.valor = pow($1.valor,$3.valor);}
    | MINUS expression %prec NEG     { $$.valor = -$2.valor;}


	| mathMIN LPAREN expression "," expression RPAREN   {std::cout <<"MIN(" << $3.valor << "," << $5.valor <<")"<< std::endl;
														 $$.valor = std::min($3.valor,$5.valor);}
	| mathMAX LPAREN expression "," expression RPAREN   { $$.valor = std::max($3.valor,$5.valor);}
    ;

logical:
      expression oAND expression    { $$.valor = (int) $1.valor && (int) $3.valor;}
    | expression oOR  expression    { $$.valor = (int) $1.valor || (int) $3.valor;}
    | expression oNAND expression   { $$.valor = !((int) $1.valor && (int) $3.valor);}
    | expression oXOR  expression   { $$.valor = (!(int) $1.valor && (int) $3.valor) || ((int) $1.valor && !(int) $3.valor);}
    | oNOT expression               { $$.valor = !(int) $2.valor;}
	;

relacional:
      expression LESS  expression        { $$.valor = $1.valor < $3.valor ? 1 : 0;}
    | expression GREATER expression      { $$.valor = $1.valor > $3.valor ? 1 : 0;}
    | expression oLE  expression         { $$.valor = $1.valor <= $3.valor ? 1 : 0;}
    | expression oGE  expression         { $$.valor = $1.valor >= $3.valor ? 1 : 0;}
    | expression oEQ  expression         { $$.valor = $1.valor == $3.valor ? 1 : 0;}
    | expression oNE  expression         { $$.valor = $1.valor != $3.valor ? 1 : 0;}
    ;

command:
      commandIF	    { $$.valor = $1.valor; }
    | commandFOR    { $$.valor = $1.valor; }
    ;

commandIF:
      cIF expression expression cELSE expression   { $$.valor = $2.valor != 0 ? $3.valor : $5.valor; }
    | cIF expression expression                   { $$.valor = $2.valor != 0 ? $3.valor : 0;}
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
    | fTAVG  "(" CSTAT ")"     {
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
    ;

probFunction:
	  fRND1					     { $$.valor = driver.getSampler()->sampleUniform(0.0,1.0);}
	| fEXPO  "(" expression ")"  { $$.valor = driver.getSampler()->sampleExponential($3.valor);}
	| fNORM  "(" expression "," expression ")"  { $$.valor = driver.getSampler()->sampleNormal($3.valor,$5.valor);}
	| fUNIF  "(" expression "," expression ")"  { $$.valor = driver.getSampler()->sampleUniform($3.valor,$5.valor);}
	| fWEIB  "(" expression "," expression ")"  { $$.valor = driver.getSampler()->sampleWeibull($3.valor,$5.valor);}
	| fLOGN  "(" expression "," expression ")"  { $$.valor = driver.getSampler()->sampleLogNormal($3.valor,$5.valor);}
	| fGAMM  "(" expression "," expression ")"  { $$.valor = driver.getSampler()->sampleGamma($3.valor,$5.valor);}
	| fERLA  "(" expression "," expression ")"  { $$.valor = driver.getSampler()->sampleErlang($3.valor,$5.valor);}
	| fTRIA  "(" expression "," expression "," expression ")"   { $$.valor = driver.getSampler()->sampleTriangular($3.valor,$5.valor,$7.valor);}
	| fBETA  "(" expression "," expression "," expression "," expression ")"  { $$.valor = driver.getSampler()->sampleBeta($3.valor,$5.valor,$7.valor,$9.valor);}
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
		if(driver.getThrowsException()){
			if($1.valor == 0){
			  throw std::string("Literal nao encontrado");
			}else if($1.valor == 1){
			  throw std::string("Caracter invalido encontrado");
			}
		} else {
			if($1.valor == 0){
			  driver.setErrorMessage(std::string("Literal nao encontrado"));
			}else if($1.valor == 1){
				driver.setErrorMessage(std::string("Caracter invalido encontrado"));
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
