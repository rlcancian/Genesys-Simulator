%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include <locale>
# include <list>
#include <iostream>
# include "Genesys++-driver.h"
# include "GenesysParser.h"
# include "obj_t.h"
# include "../kernel/util/Util.h"
# include "../kernel/util/List.h"
# include "../kernel/simulator/Attribute.h"
# include "../kernel/simulator/Counter.h"
# include "../kernel/simulator/ModelDataDefinition.h"
# include "../kernel/simulator/StatisticsCollector.h"
/**begin_Includes_plugins**/
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
/**end_Includes_plugins**/


// Work around an incompatibility in flex (at least versions
// 2.5.31 through 2.5.33): it generates code that does
// not conform to C89.  See Debian bug 333231
// <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.
# undef yywrap
# define yywrap() 1

// The location of the current token.
static yy::location loc;
%}
%option noyywrap nounput batch noinput

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
	//std::cout << yyleng << std::endl;
%}

I     ~[A-Za-z]+
DD     [0-9]+([eE][-]?[0-9]+)?
RR     [0-9]+[.][0-9]+([eE][-]?[0-9]+)?
L      [A-Za-z0-9_.]+

%%

%{
  // Code run each time yylex is called.
  loc.step ();
%}


[0][xX][aA-fF0-9]+  {
	//Hexadecimal number
	//Will not fail because of regex
	//std::string text("Found Hexadecimal: ");
	//text += yytext;
	//driver.getModel()->getTraceManager()->trace(TraceManager::Level::L9_mostDetailed, text);
	char* endPtr = nullptr;
	long long value = std::strtoll(yytext, &endPtr, 16);
	return yy::genesyspp_parser::make_NUMH(obj_t(static_cast<double>(value), std::string("Hexadecimal")),loc);
      }

{RR}  {
       //Float number
       //Will not fail because of regex
       //std::string text("Found Float: ");
       //text += yytext;
       //driver.getModel()->getTraceManager()->trace(TraceManager::Level::L9_mostDetailed, text);
       return yy::genesyspp_parser::make_NUMD(obj_t(atof(yytext),std::string("Float")), loc);
     }

{DD}  {
       //Decimal number
       //Will not fail because of regex
       //std::string text("Found Decimal: ");
       //text += yytext;
       //driver.getModel()->getTraceManager()->trace(TraceManager::Level::L9_mostDetailed, text);
       return yy::genesyspp_parser::make_NUMD(obj_t(atof(yytext),std::string("Decimal")), loc);
      }

%{// special symbols %}
[<][=]   { return (yy::genesyspp_parser::make_oLE(obj_t(0, std::string(yytext)), loc));}
[>][=]   { return (yy::genesyspp_parser::make_oGE(obj_t(0, std::string(yytext)), loc));}
[=][=]   { return (yy::genesyspp_parser::make_oEQ(obj_t(0, std::string(yytext)), loc));}
[<][>]   { return (yy::genesyspp_parser::make_oNE(obj_t(0, std::string(yytext)), loc));}
[!][=]   { return (yy::genesyspp_parser::make_oNE(obj_t(0, std::string(yytext)), loc));}
[&][&]   { return (yy::genesyspp_parser::make_oAND(obj_t(0, std::string(yytext)), loc));}
[|][|]   { return (yy::genesyspp_parser::make_oOR(obj_t(0, std::string(yytext)), loc));}
[!]      { return (yy::genesyspp_parser::make_oNOT(obj_t(0, std::string(yytext)), loc));}
[<] {return yy::genesyspp_parser::make_LESS(loc);}
[>] {return yy::genesyspp_parser::make_GREATER(loc);}
[(] {return yy::genesyspp_parser::make_LPAREN(loc);}
[)] {return yy::genesyspp_parser::make_RPAREN(loc);}
[+] {return yy::genesyspp_parser::make_PLUS(loc);}
[-] {return yy::genesyspp_parser::make_MINUS(loc);}
[*] {return yy::genesyspp_parser::make_STAR(loc);}
[\^] {return yy::genesyspp_parser::make_POWER(loc);}
[/] {return yy::genesyspp_parser::make_SLASH(loc);}
[=] {return yy::genesyspp_parser::make_ASSIGN(loc);}
[,] {return yy::genesyspp_parser::make_COMMA(loc);}
[\[] {return yy::genesyspp_parser::make_LBRACKET(loc);}
[\]] {return yy::genesyspp_parser::make_RBRACKET(loc);}

%{// boolean values %}
[tT][rR][uU][eE]      {return yy::genesyspp_parser::make_NUMD(obj_t(1, std::string(yytext)), loc); /* @todo: should it be -1? */} 
[fF][aA][lL][sS][eE]  {return yy::genesyspp_parser::make_NUMD(obj_t(0, std::string(yytext)), loc);}

%{// algorithmic functions %}
[iI][fF]              {return yy::genesyspp_parser::make_cIF(obj_t(0, std::string(yytext)), loc);}
[eE][lL][sS][eE]      {return yy::genesyspp_parser::make_cELSE(obj_t(0, std::string(yytext)), loc);}
[fF][oO][rR]          {return yy::genesyspp_parser::make_cFOR(obj_t(0, std::string(yytext)), loc);}
[tT][oO]              {return yy::genesyspp_parser::make_cTO(obj_t(0, std::string(yytext)), loc);}
[dD][oO]              {return yy::genesyspp_parser::make_cDO(obj_t(0, std::string(yytext)), loc);}

%{// logic funcions %}
[aA][nN][dD]    {return yy::genesyspp_parser::make_oAND(obj_t(0, std::string(yytext)), loc);}
[oO][rR]        {return yy::genesyspp_parser::make_oOR(obj_t(0, std::string(yytext)), loc);}
[nN][aA][nN][dD]    {return yy::genesyspp_parser::make_oNAND(obj_t(0, std::string(yytext)), loc);}
[xX][oO][rR]        {return yy::genesyspp_parser::make_oXOR(obj_t(0, std::string(yytext)), loc);}
[nN][oO][tT]    {return yy::genesyspp_parser::make_oNOT(obj_t(0, std::string(yytext)), loc);}

%{// trigonometric functions %}
[sS][iI][nN]      {return yy::genesyspp_parser::make_fSIN(obj_t(0, std::string(yytext)), loc);}
[cC][oO][sS]      {return yy::genesyspp_parser::make_fCOS(obj_t(0, std::string(yytext)), loc);}

%{// math funcions %}
[rR][oO][uU][nN][dD]  {return yy::genesyspp_parser::make_fROUND(obj_t(0, std::string(yytext)), loc);}
[mM][oO][dD]          {return yy::genesyspp_parser::make_fMOD(obj_t(0, std::string(yytext)), loc);}
[tT][rR][uU][nN][cC]  {return yy::genesyspp_parser::make_fTRUNC(obj_t(0, std::string(yytext)), loc);}
[fF][rR][aA][cC]      {return yy::genesyspp_parser::make_fFRAC(obj_t(0, std::string(yytext)), loc);}
[eE][xX][pP]          {return yy::genesyspp_parser::make_fEXP(obj_t(0, std::string(yytext)), loc);}
[sS][qQ][rR][tT]      {return yy::genesyspp_parser::make_fSQRT(obj_t(0, std::string(yytext)), loc);}
[lL][oO][gG]          {return yy::genesyspp_parser::make_fLOG(obj_t(0, std::string(yytext)), loc);}
[lL][nN]              {return yy::genesyspp_parser::make_fLN(obj_t(0, std::string(yytext)), loc);}
[mM][iI][nN]		  {return yy::genesyspp_parser::make_mathMIN(obj_t(0, std::string(yytext)), loc);}
[mM][aA][xX]          {return yy::genesyspp_parser::make_mathMAX(obj_t(0, std::string(yytext)), loc);}

%{// string functions %}
[vV][aA][lL]          {return yy::genesyspp_parser::make_fVAL(obj_t(0, std::string(yytext)), loc);}
[eE][vV][aA][lL]      {return yy::genesyspp_parser::make_fEVAL(obj_t(0, std::string(yytext)), loc);}
[lL][eE][nN][gG]      {return yy::genesyspp_parser::make_fLENG(obj_t(0, std::string(yytext)), loc);}

%{// probability distributions %}
[rR][nN][dD]	  {return yy::genesyspp_parser::make_fRND1(obj_t(0, std::string(yytext)), loc);}
[eE][xX][pP][oO]  {return yy::genesyspp_parser::make_fEXPO(obj_t(0, std::string(yytext)), loc);}
[nN][oO][rR][mM]  {return yy::genesyspp_parser::make_fNORM(obj_t(0, std::string(yytext)), loc); /*!< norm(average,stddev): Generates a normal distribution random value */}
[uU][nN][iI][fF]  {return yy::genesyspp_parser::make_fUNIF(obj_t(0, std::string(yytext)), loc);}
[wW][eE][iI][bB]  {return yy::genesyspp_parser::make_fWEIB(obj_t(0, std::string(yytext)), loc);}
[lL][oO][gG][nN]  {return yy::genesyspp_parser::make_fLOGN(obj_t(0, std::string(yytext)), loc);}
[gG][aA][mM][mM]  {return yy::genesyspp_parser::make_fGAMM(obj_t(0, std::string(yytext)), loc);}
[eE][rR][lL][aA]  {return yy::genesyspp_parser::make_fERLA(obj_t(0, std::string(yytext)), loc);}
[tT][rR][iI][aA]  {return yy::genesyspp_parser::make_fTRIA(obj_t(0, std::string(yytext)), loc);}
[bB][eE][tT][aA]  {return yy::genesyspp_parser::make_fBETA(obj_t(0, std::string(yytext)), loc);}
[dD][iI][sS][cC]  {return yy::genesyspp_parser::make_fDISC(obj_t(0, std::string(yytext)), loc);}

%{// simulation infos %}
[tT][nN][oO][wW]          {return yy::genesyspp_parser::make_fTNOW(obj_t(0, std::string(yytext)), loc);}
[tT][fF][iI][nN]          {return yy::genesyspp_parser::make_fTFIN(obj_t(0, std::string(yytext)), loc);}
[mM][aA][xX][rR][eE][pP]  {return yy::genesyspp_parser::make_fMAXREP(obj_t(0, std::string(yytext)), loc);}
[nN][uU][mM][rR][eE][pP]  {return yy::genesyspp_parser::make_fNUMREP(obj_t(0, std::string(yytext)), loc);}
[iI][dD][eE][nN][tT]      {return yy::genesyspp_parser::make_fIDENT(obj_t(0, std::string(yytext)), loc);}
[eE][nN][tT][iI][tT][iI][eE][sS][wW][iI][pP]   {return yy::genesyspp_parser::make_simulEntitiesWIP(obj_t(0, std::string(yytext)), loc);}

%{// Kernel elements, Counters and Statistic Collectors %}
[tT][aA][vV][gG]          {return yy::genesyspp_parser::make_fTAVG(obj_t(0, std::string(yytext)), loc);}
[cC][oO][uU][nN][tT]      {return yy::genesyspp_parser::make_fCOUNT(obj_t(0, std::string(yytext)), loc);}


%{/****begin_Lexical_plugins****/%}

%{/**begin_Lexical:Resource**/%}
[nN][rR]                                        {return yy::genesyspp_parser::make_fNR(obj_t(0, std::string(yytext)), loc);}
[mM][rR]                                        {return yy::genesyspp_parser::make_fMR(obj_t(0, std::string(yytext)), loc);}
[iI][rR][fF]                                    {return yy::genesyspp_parser::make_fIRF(obj_t(0, std::string(yytext)), loc);}
[sS][tT][aA][tT][eE]                            {return yy::genesyspp_parser::make_fSTATE(obj_t(0, std::string(yytext)), loc);}
[sS][eE][tT][sS][uU][mM]                        {return yy::genesyspp_parser::make_fSETSUM(obj_t(0, std::string(yytext)), loc);}
[rR][eE][sS][uU][tT][iI][lL]                    {return yy::genesyspp_parser::make_fRESUTIL(obj_t(0, std::string(yytext)), loc);}
[rR][eE][sS][sS][eE][iI][zZ][eE][sS]            {return yy::genesyspp_parser::make_fRESSEIZES(obj_t(0, std::string(yytext)), loc);}
[iI][dD][lL][eE][_][rR][eE][sS]                 {return yy::genesyspp_parser::make_NUMD(obj_t(-1, std::string(yytext)), loc);}
[bB][uU][sS][yY][_][rR][eE][sS]                 {return yy::genesyspp_parser::make_NUMD(obj_t(-2, std::string(yytext)), loc);}
[iI][nN][aA][cC][tT][iI][vV][eE][_][rR][eE][sS] {return yy::genesyspp_parser::make_NUMD(obj_t(-3, std::string(yytext)), loc);}
[fF][aA][iI][lL][eE][dD][_][rR][eE][sS]         {return yy::genesyspp_parser::make_NUMD(obj_t(-4, std::string(yytext)), loc);}
%{/**end_Lexical:Resource**/%}

%{/**begin_Lexical:Queue**/%}
[nN][qQ]                             {return yy::genesyspp_parser::make_fNQ(obj_t(0, std::string(yytext)), loc);}
[lL][aA][sS][tT][iI][nN][qQ]         {return yy::genesyspp_parser::make_fLASTINQ(obj_t(0, std::string(yytext)), loc);}
[fF][iI][rR][sS][tT][iI][nN][qQ]     {return yy::genesyspp_parser::make_fFIRSTINQ(obj_t(0, std::string(yytext)), loc);}
[sS][aA][qQ][uU][eE]                 {return yy::genesyspp_parser::make_fSAQUE(obj_t(0, std::string(yytext)), loc);}
[aA][qQ][uU][eE]                     {return yy::genesyspp_parser::make_fAQUE(obj_t(0, std::string(yytext)), loc);}
[eE][nN][tT][aA][tT][rR][aA][nN][kK] {return yy::genesyspp_parser::make_fENTATRANK(obj_t(0, std::string(yytext)), loc);}
%{/**end_Lexical:Queue**/%}

%{/**begin_Lexical:Set**/%}
[nN][uU][mM][sS][eE][tT]             {return yy::genesyspp_parser::make_fNUMSET(obj_t(0, std::string(yytext)), loc);}
%{/**end_Lexical:Set**/%}

%{/**begin_Lexical:EntityGroup**/%}
[nN][uU][mM][gG][rR]                 {return yy::genesyspp_parser::make_fNUMGR(obj_t(0, std::string(yytext)), loc);}
[aA][tT][rR][gG][rR]                 {return yy::genesyspp_parser::make_fATRGR(obj_t(0, std::string(yytext)), loc);}
%{/**end_Lexical:EntityGroup**/%}

%{/****end_Lexical_plugins****/%}


[ \t\n]        ;

{L}   {
        ModelDataDefinition* datadef; 

		//std::cout << "Verificando o que é o Literal \""+std::string(yytext)+"\"\n";
		//std::cout << driver.getModel()->getInfos()->getName() << std::endl;
        // check if it is an ATTRIBUTE (and return the attribute ID (and not the value!)
		datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), std::string(yytext));
        if (datadef != nullptr) {
			//std::cout << "Found ATTRIBUTE " << std::string(yytext) << std::endl;
			driver.addRefered({Util::TypeOf<Attribute>(),std::string(yytext)});
            return yy::genesyspp_parser::make_ATRIB(obj_t(0, Util::TypeOf<Attribute>(), datadef->getId()),loc);
        }


        //check CSTAT
        datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<StatisticsCollector>(), std::string(yytext));
        if (datadef != nullptr) { 
			//std::cout << "Found CSTAT" << std::string(yytext) << std::endl;
			driver.addRefered({Util::TypeOf<StatisticsCollector>(),std::string(yytext)});
            return yy::genesyspp_parser::make_CSTAT(obj_t(0, Util::TypeOf<StatisticsCollector>(), datadef->getId()),loc);
        }

        //check COUNTER
        datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Counter>(), std::string(yytext));
        if (datadef != nullptr) { 
			//std::cout << "Found COUNTER" << std::string(yytext) << std::endl;
			driver.addRefered({Util::TypeOf<Counter>(),std::string(yytext)});
			return yy::genesyspp_parser::make_COUNTER(obj_t(0, Util::TypeOf<Counter>(), datadef->getId()),loc);
        }

/****begin_LexicalLiterals_plugins****/

/**begin_LexicalLiterals:Variable**/
        // check VARIABLE
        datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), std::string(yytext));
        if (datadef != nullptr) { // it is a variable
            Variable* var = static_cast<Variable*>(datadef);
            //double variableID = var->getId();// ->getValue(); // var->getId()
			//std::cout << "Found VARIABLE " << var->getName() <<" ID " << var->getId() << std::endl;
			driver.addRefered({Util::TypeOf<Variable>(),std::string(yytext)});
			return yy::genesyspp_parser::make_VARI(obj_t(0, Util::TypeOf<Variable>(), var->getId()),loc);
        }
/**end_LexicalLiterals:Variable**/

/**begin_LexicalLiterals:Formula**/
        // check FORMULA
        datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), std::string(yytext));
        if (datadef != nullptr) { // it is a FORMULA
            Formula* form = static_cast<Formula*>(datadef);
            // return only formula ID. NEVER should call getValue in LL
			//std::cout << "Found FORMULA " << form->getName() <<" ID " << form->getId() << std::endl;
			driver.addRefered({Util::TypeOf<Formula>(),std::string(yytext)});
			return yy::genesyspp_parser::make_FORM(obj_t(0, Util::TypeOf<Formula>(), form->getId()),loc);
        }
/**end_LexicalLiterals:Formula**/

/**begin_LexicalLiterals:Queue**/
        // check QUEUE
		//List<ModelDataDefinition*>* queues =  driver.getModel()->getDataManager()->getDataDefinitionList(Util::TypeOf<Queue>());
		//std::cout << "Há " << queues->size() << " filas" << std::endl;
		//for (ModelDataDefinition* dd: *queues->list()) {
		//	std::cout << " queue: " << dd->getName() << std::endl;
		//}
        datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), std::string(yytext));
        if (datadef != nullptr) { 
			//std::cout << "Found QUEUE " << datadef->getName() <<" ID " << datadef->getId() << std::endl;
			driver.addRefered({Util::TypeOf<Queue>(),std::string(yytext)});
			return yy::genesyspp_parser::make_QUEUE(obj_t(0, Util::TypeOf<Queue>(), datadef->getId()),loc);
        }
/**end_LexicalLiterals:Queue**/

/**begin_LexicalLiterals:Resource**/
        // check RESOURCE
        datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), std::string(yytext));
        if (datadef != nullptr) { 
			//std::cout << "Found RESOURCE " << datadef->getName() <<" ID " << datadef->getId() << std::endl;
			driver.addRefered({Util::TypeOf<Resource>(),std::string(yytext)});
			return yy::genesyspp_parser::make_RESOURCE(obj_t(0, Util::TypeOf<Resource>(), datadef->getId()),loc);
        }
/**end_LexicalLiterals:Resource**/

/**begin_LexicalLiterals:Set**/
        //check SET
        datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(), std::string(yytext));
        if (datadef != nullptr) { 
			//std::cout << "Found SET " << datadef->getName() <<" ID " << datadef->getId() << std::endl;
			driver.addRefered({Util::TypeOf<Set>(),std::string(yytext)});
			return yy::genesyspp_parser::make_SET(obj_t(0, Util::TypeOf<Set>(), datadef->getId()),loc);
        }
/**end_LexicalLiterals:Set**/

/****end_LexicalLiterals_plugins****/

	// If no one before has identified this literal, then it is an ILLEGAL (not found, unknown) literal 
        //Case not found retturns a illegal token
		//datadef = driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(), std::string(yytext));
		//std::cout << "NOT FOUND " << std::string(yytext) << std::endl;
        return yy::genesyspp_parser::make_ILLEGAL(obj_t(0, std::string("Illegal")), loc);
      }

.       {return yy::genesyspp_parser::make_ILLEGAL(obj_t(1, std::string("Illegal")), loc);}

<<EOF>> {return yy::genesyspp_parser::make_END(loc);}


%%

void
genesyspp_driver::scan_begin_file ()
{
  //yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      error ("cannot open " + file + ": " + strerror(errno));
      exit (EXIT_FAILURE);
    }
}

void genesyspp_driver::scan_begin_str ()
{
  //yy_flex_debug = trace_scanning;
  if(!str_to_parse.empty()){    
    yy_scan_string (str_to_parse.c_str()); //maybe throw exception on else
  }else{
    std::string str("0");
    yy_scan_string (str.c_str()); //maybe throw exception on else
  }
}



void
genesyspp_driver::scan_end_file ()
{
  fclose (yyin);
}

void
genesyspp_driver::scan_end_str ()
{
  yy_delete_buffer(YY_CURRENT_BUFFER);
}
