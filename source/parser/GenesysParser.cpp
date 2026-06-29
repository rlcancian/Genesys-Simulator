// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "GenesysParser.h"


// Unqualified %code blocks.
#line 63 "bisonparser.yy"

# include "Genesys++-driver.h"
# include "SemanticResolver.h"
# include <exception>

namespace {

std::string buildProbError(const std::string& functionName, const std::string& details) {
	return std::string("Error evaluating ") + functionName + ": " + details;
}

void failProbFunction(genesyspp_driver& driver, const std::string& message) {
	driver.setErrorMessage(message);
	driver.setResult(-1);
}

std::string parserIndexPart(double value) {
	return SparseValueStore::makeIndexKey(std::vector<unsigned int>{static_cast<unsigned int>(value)});
}

std::string appendParserIndex(const std::string& currentKey, double value) {
	return SparseValueStore::appendIndexKeyFromDouble(currentKey, value);
}

SemanticResolverResult resolveGenericFunction(genesyspp_driver& driver, const std::string& functionName, const std::vector<double>& arguments) {
	SemanticResolver resolver(driver.getFunctionRegistry());
	SemanticResolverResult result = resolver.resolveFunction(functionName, arguments);
	if (!result.success) {
		driver.setErrorMessage(result.errorMessage);
		driver.setResult(-1);
	}
	return result;
}

}


#line 84 "../GenesysParser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 176 "../GenesysParser.cpp"

  /// Build a parser object.
  genesyspp_parser::genesyspp_parser (genesyspp_driver& driver_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      driver (driver_yyarg)
  {}

  genesyspp_parser::~genesyspp_parser ()
  {}

  genesyspp_parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
  genesyspp_parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  genesyspp_parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  genesyspp_parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  genesyspp_parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  genesyspp_parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  genesyspp_parser::symbol_kind_type
  genesyspp_parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  genesyspp_parser::stack_symbol_type::stack_symbol_type ()
  {}

  genesyspp_parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_NUMD: // NUMD
      case symbol_kind::S_NUMH: // NUMH
      case symbol_kind::S_CTEZERO: // CTEZERO
      case symbol_kind::S_oLE: // oLE
      case symbol_kind::S_oGE: // oGE
      case symbol_kind::S_oEQ: // oEQ
      case symbol_kind::S_oNE: // oNE
      case symbol_kind::S_oAND: // oAND
      case symbol_kind::S_oOR: // oOR
      case symbol_kind::S_oNAND: // oNAND
      case symbol_kind::S_oXOR: // oXOR
      case symbol_kind::S_oNOT: // oNOT
      case symbol_kind::S_fSIN: // fSIN
      case symbol_kind::S_fCOS: // fCOS
      case symbol_kind::S_fROUND: // fROUND
      case symbol_kind::S_fMOD: // fMOD
      case symbol_kind::S_fTRUNC: // fTRUNC
      case symbol_kind::S_fFRAC: // fFRAC
      case symbol_kind::S_fEXP: // fEXP
      case symbol_kind::S_fSQRT: // fSQRT
      case symbol_kind::S_fLOG: // fLOG
      case symbol_kind::S_fLN: // fLN
      case symbol_kind::S_mathMIN: // mathMIN
      case symbol_kind::S_mathMAX: // mathMAX
      case symbol_kind::S_fVAL: // fVAL
      case symbol_kind::S_fEVAL: // fEVAL
      case symbol_kind::S_fLENG: // fLENG
      case symbol_kind::S_fRND1: // fRND1
      case symbol_kind::S_fEXPO: // fEXPO
      case symbol_kind::S_fNORM: // fNORM
      case symbol_kind::S_fUNIF: // fUNIF
      case symbol_kind::S_fWEIB: // fWEIB
      case symbol_kind::S_fLOGN: // fLOGN
      case symbol_kind::S_fGAMM: // fGAMM
      case symbol_kind::S_fERLA: // fERLA
      case symbol_kind::S_fTRIA: // fTRIA
      case symbol_kind::S_fBETA: // fBETA
      case symbol_kind::S_fDISC: // fDISC
      case symbol_kind::S_fTNOW: // fTNOW
      case symbol_kind::S_fTFIN: // fTFIN
      case symbol_kind::S_fMAXREP: // fMAXREP
      case symbol_kind::S_fNUMREP: // fNUMREP
      case symbol_kind::S_fIDENT: // fIDENT
      case symbol_kind::S_simulEntitiesWIP: // simulEntitiesWIP
      case symbol_kind::S_cIF: // cIF
      case symbol_kind::S_cELSE: // cELSE
      case symbol_kind::S_cFOR: // cFOR
      case symbol_kind::S_cTO: // cTO
      case symbol_kind::S_cDO: // cDO
      case symbol_kind::S_ATRIB: // ATRIB
      case symbol_kind::S_CSTAT: // CSTAT
      case symbol_kind::S_COUNTER: // COUNTER
      case symbol_kind::S_SIMRESP: // SIMRESP
      case symbol_kind::S_SIMCTRL: // SIMCTRL
      case symbol_kind::S_fTAVG: // fTAVG
      case symbol_kind::S_fCOUNT: // fCOUNT
      case symbol_kind::S_ILLEGAL: // ILLEGAL
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_RESOURCE: // RESOURCE
      case symbol_kind::S_fNR: // fNR
      case symbol_kind::S_fMR: // fMR
      case symbol_kind::S_fIRF: // fIRF
      case symbol_kind::S_fRESSEIZES: // fRESSEIZES
      case symbol_kind::S_fSTATE: // fSTATE
      case symbol_kind::S_fSETSUM: // fSETSUM
      case symbol_kind::S_fRESUTIL: // fRESUTIL
      case symbol_kind::S_QUEUE: // QUEUE
      case symbol_kind::S_fNQ: // fNQ
      case symbol_kind::S_fFIRSTINQ: // fFIRSTINQ
      case symbol_kind::S_fLASTINQ: // fLASTINQ
      case symbol_kind::S_fSAQUE: // fSAQUE
      case symbol_kind::S_fAQUE: // fAQUE
      case symbol_kind::S_fENTATRANK: // fENTATRANK
      case symbol_kind::S_SET: // SET
      case symbol_kind::S_fNUMSET: // fNUMSET
      case symbol_kind::S_VARI: // VARI
      case symbol_kind::S_FORM: // FORM
      case symbol_kind::S_fNUMGR: // fNUMGR
      case symbol_kind::S_fATRGR: // fATRGR
      case symbol_kind::S_input: // input
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_logicalOr: // logicalOr
      case symbol_kind::S_logicalXor: // logicalXor
      case symbol_kind::S_logicalAnd: // logicalAnd
      case symbol_kind::S_logicalNot: // logicalNot
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_number: // number
      case symbol_kind::S_command: // command
      case symbol_kind::S_commandIF: // commandIF
      case symbol_kind::S_commandFOR: // commandFOR
      case symbol_kind::S_function: // function
      case symbol_kind::S_kernelFunction: // kernelFunction
      case symbol_kind::S_elementFunction: // elementFunction
      case symbol_kind::S_trigonFunction: // trigonFunction
      case symbol_kind::S_mathFunction: // mathFunction
      case symbol_kind::S_probFunction: // probFunction
      case symbol_kind::S_userFunction: // userFunction
      case symbol_kind::S_listaparm: // listaparm
      case symbol_kind::S_genericFunction: // genericFunction
      case symbol_kind::S_illegal: // illegal
      case symbol_kind::S_attribute: // attribute
      case symbol_kind::S_simulationResponse: // simulationResponse
      case symbol_kind::S_simulationControl: // simulationControl
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        value.YY_MOVE_OR_COPY< obj_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_indexList: // indexList
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_argumentList: // argumentList
        value.YY_MOVE_OR_COPY< std::vector<double> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  genesyspp_parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_NUMD: // NUMD
      case symbol_kind::S_NUMH: // NUMH
      case symbol_kind::S_CTEZERO: // CTEZERO
      case symbol_kind::S_oLE: // oLE
      case symbol_kind::S_oGE: // oGE
      case symbol_kind::S_oEQ: // oEQ
      case symbol_kind::S_oNE: // oNE
      case symbol_kind::S_oAND: // oAND
      case symbol_kind::S_oOR: // oOR
      case symbol_kind::S_oNAND: // oNAND
      case symbol_kind::S_oXOR: // oXOR
      case symbol_kind::S_oNOT: // oNOT
      case symbol_kind::S_fSIN: // fSIN
      case symbol_kind::S_fCOS: // fCOS
      case symbol_kind::S_fROUND: // fROUND
      case symbol_kind::S_fMOD: // fMOD
      case symbol_kind::S_fTRUNC: // fTRUNC
      case symbol_kind::S_fFRAC: // fFRAC
      case symbol_kind::S_fEXP: // fEXP
      case symbol_kind::S_fSQRT: // fSQRT
      case symbol_kind::S_fLOG: // fLOG
      case symbol_kind::S_fLN: // fLN
      case symbol_kind::S_mathMIN: // mathMIN
      case symbol_kind::S_mathMAX: // mathMAX
      case symbol_kind::S_fVAL: // fVAL
      case symbol_kind::S_fEVAL: // fEVAL
      case symbol_kind::S_fLENG: // fLENG
      case symbol_kind::S_fRND1: // fRND1
      case symbol_kind::S_fEXPO: // fEXPO
      case symbol_kind::S_fNORM: // fNORM
      case symbol_kind::S_fUNIF: // fUNIF
      case symbol_kind::S_fWEIB: // fWEIB
      case symbol_kind::S_fLOGN: // fLOGN
      case symbol_kind::S_fGAMM: // fGAMM
      case symbol_kind::S_fERLA: // fERLA
      case symbol_kind::S_fTRIA: // fTRIA
      case symbol_kind::S_fBETA: // fBETA
      case symbol_kind::S_fDISC: // fDISC
      case symbol_kind::S_fTNOW: // fTNOW
      case symbol_kind::S_fTFIN: // fTFIN
      case symbol_kind::S_fMAXREP: // fMAXREP
      case symbol_kind::S_fNUMREP: // fNUMREP
      case symbol_kind::S_fIDENT: // fIDENT
      case symbol_kind::S_simulEntitiesWIP: // simulEntitiesWIP
      case symbol_kind::S_cIF: // cIF
      case symbol_kind::S_cELSE: // cELSE
      case symbol_kind::S_cFOR: // cFOR
      case symbol_kind::S_cTO: // cTO
      case symbol_kind::S_cDO: // cDO
      case symbol_kind::S_ATRIB: // ATRIB
      case symbol_kind::S_CSTAT: // CSTAT
      case symbol_kind::S_COUNTER: // COUNTER
      case symbol_kind::S_SIMRESP: // SIMRESP
      case symbol_kind::S_SIMCTRL: // SIMCTRL
      case symbol_kind::S_fTAVG: // fTAVG
      case symbol_kind::S_fCOUNT: // fCOUNT
      case symbol_kind::S_ILLEGAL: // ILLEGAL
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_RESOURCE: // RESOURCE
      case symbol_kind::S_fNR: // fNR
      case symbol_kind::S_fMR: // fMR
      case symbol_kind::S_fIRF: // fIRF
      case symbol_kind::S_fRESSEIZES: // fRESSEIZES
      case symbol_kind::S_fSTATE: // fSTATE
      case symbol_kind::S_fSETSUM: // fSETSUM
      case symbol_kind::S_fRESUTIL: // fRESUTIL
      case symbol_kind::S_QUEUE: // QUEUE
      case symbol_kind::S_fNQ: // fNQ
      case symbol_kind::S_fFIRSTINQ: // fFIRSTINQ
      case symbol_kind::S_fLASTINQ: // fLASTINQ
      case symbol_kind::S_fSAQUE: // fSAQUE
      case symbol_kind::S_fAQUE: // fAQUE
      case symbol_kind::S_fENTATRANK: // fENTATRANK
      case symbol_kind::S_SET: // SET
      case symbol_kind::S_fNUMSET: // fNUMSET
      case symbol_kind::S_VARI: // VARI
      case symbol_kind::S_FORM: // FORM
      case symbol_kind::S_fNUMGR: // fNUMGR
      case symbol_kind::S_fATRGR: // fATRGR
      case symbol_kind::S_input: // input
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_logicalOr: // logicalOr
      case symbol_kind::S_logicalXor: // logicalXor
      case symbol_kind::S_logicalAnd: // logicalAnd
      case symbol_kind::S_logicalNot: // logicalNot
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_number: // number
      case symbol_kind::S_command: // command
      case symbol_kind::S_commandIF: // commandIF
      case symbol_kind::S_commandFOR: // commandFOR
      case symbol_kind::S_function: // function
      case symbol_kind::S_kernelFunction: // kernelFunction
      case symbol_kind::S_elementFunction: // elementFunction
      case symbol_kind::S_trigonFunction: // trigonFunction
      case symbol_kind::S_mathFunction: // mathFunction
      case symbol_kind::S_probFunction: // probFunction
      case symbol_kind::S_userFunction: // userFunction
      case symbol_kind::S_listaparm: // listaparm
      case symbol_kind::S_genericFunction: // genericFunction
      case symbol_kind::S_illegal: // illegal
      case symbol_kind::S_attribute: // attribute
      case symbol_kind::S_simulationResponse: // simulationResponse
      case symbol_kind::S_simulationControl: // simulationControl
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        value.move< obj_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_indexList: // indexList
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_argumentList: // argumentList
        value.move< std::vector<double> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  genesyspp_parser::stack_symbol_type&
  genesyspp_parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_NUMD: // NUMD
      case symbol_kind::S_NUMH: // NUMH
      case symbol_kind::S_CTEZERO: // CTEZERO
      case symbol_kind::S_oLE: // oLE
      case symbol_kind::S_oGE: // oGE
      case symbol_kind::S_oEQ: // oEQ
      case symbol_kind::S_oNE: // oNE
      case symbol_kind::S_oAND: // oAND
      case symbol_kind::S_oOR: // oOR
      case symbol_kind::S_oNAND: // oNAND
      case symbol_kind::S_oXOR: // oXOR
      case symbol_kind::S_oNOT: // oNOT
      case symbol_kind::S_fSIN: // fSIN
      case symbol_kind::S_fCOS: // fCOS
      case symbol_kind::S_fROUND: // fROUND
      case symbol_kind::S_fMOD: // fMOD
      case symbol_kind::S_fTRUNC: // fTRUNC
      case symbol_kind::S_fFRAC: // fFRAC
      case symbol_kind::S_fEXP: // fEXP
      case symbol_kind::S_fSQRT: // fSQRT
      case symbol_kind::S_fLOG: // fLOG
      case symbol_kind::S_fLN: // fLN
      case symbol_kind::S_mathMIN: // mathMIN
      case symbol_kind::S_mathMAX: // mathMAX
      case symbol_kind::S_fVAL: // fVAL
      case symbol_kind::S_fEVAL: // fEVAL
      case symbol_kind::S_fLENG: // fLENG
      case symbol_kind::S_fRND1: // fRND1
      case symbol_kind::S_fEXPO: // fEXPO
      case symbol_kind::S_fNORM: // fNORM
      case symbol_kind::S_fUNIF: // fUNIF
      case symbol_kind::S_fWEIB: // fWEIB
      case symbol_kind::S_fLOGN: // fLOGN
      case symbol_kind::S_fGAMM: // fGAMM
      case symbol_kind::S_fERLA: // fERLA
      case symbol_kind::S_fTRIA: // fTRIA
      case symbol_kind::S_fBETA: // fBETA
      case symbol_kind::S_fDISC: // fDISC
      case symbol_kind::S_fTNOW: // fTNOW
      case symbol_kind::S_fTFIN: // fTFIN
      case symbol_kind::S_fMAXREP: // fMAXREP
      case symbol_kind::S_fNUMREP: // fNUMREP
      case symbol_kind::S_fIDENT: // fIDENT
      case symbol_kind::S_simulEntitiesWIP: // simulEntitiesWIP
      case symbol_kind::S_cIF: // cIF
      case symbol_kind::S_cELSE: // cELSE
      case symbol_kind::S_cFOR: // cFOR
      case symbol_kind::S_cTO: // cTO
      case symbol_kind::S_cDO: // cDO
      case symbol_kind::S_ATRIB: // ATRIB
      case symbol_kind::S_CSTAT: // CSTAT
      case symbol_kind::S_COUNTER: // COUNTER
      case symbol_kind::S_SIMRESP: // SIMRESP
      case symbol_kind::S_SIMCTRL: // SIMCTRL
      case symbol_kind::S_fTAVG: // fTAVG
      case symbol_kind::S_fCOUNT: // fCOUNT
      case symbol_kind::S_ILLEGAL: // ILLEGAL
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_RESOURCE: // RESOURCE
      case symbol_kind::S_fNR: // fNR
      case symbol_kind::S_fMR: // fMR
      case symbol_kind::S_fIRF: // fIRF
      case symbol_kind::S_fRESSEIZES: // fRESSEIZES
      case symbol_kind::S_fSTATE: // fSTATE
      case symbol_kind::S_fSETSUM: // fSETSUM
      case symbol_kind::S_fRESUTIL: // fRESUTIL
      case symbol_kind::S_QUEUE: // QUEUE
      case symbol_kind::S_fNQ: // fNQ
      case symbol_kind::S_fFIRSTINQ: // fFIRSTINQ
      case symbol_kind::S_fLASTINQ: // fLASTINQ
      case symbol_kind::S_fSAQUE: // fSAQUE
      case symbol_kind::S_fAQUE: // fAQUE
      case symbol_kind::S_fENTATRANK: // fENTATRANK
      case symbol_kind::S_SET: // SET
      case symbol_kind::S_fNUMSET: // fNUMSET
      case symbol_kind::S_VARI: // VARI
      case symbol_kind::S_FORM: // FORM
      case symbol_kind::S_fNUMGR: // fNUMGR
      case symbol_kind::S_fATRGR: // fATRGR
      case symbol_kind::S_input: // input
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_logicalOr: // logicalOr
      case symbol_kind::S_logicalXor: // logicalXor
      case symbol_kind::S_logicalAnd: // logicalAnd
      case symbol_kind::S_logicalNot: // logicalNot
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_number: // number
      case symbol_kind::S_command: // command
      case symbol_kind::S_commandIF: // commandIF
      case symbol_kind::S_commandFOR: // commandFOR
      case symbol_kind::S_function: // function
      case symbol_kind::S_kernelFunction: // kernelFunction
      case symbol_kind::S_elementFunction: // elementFunction
      case symbol_kind::S_trigonFunction: // trigonFunction
      case symbol_kind::S_mathFunction: // mathFunction
      case symbol_kind::S_probFunction: // probFunction
      case symbol_kind::S_userFunction: // userFunction
      case symbol_kind::S_listaparm: // listaparm
      case symbol_kind::S_genericFunction: // genericFunction
      case symbol_kind::S_illegal: // illegal
      case symbol_kind::S_attribute: // attribute
      case symbol_kind::S_simulationResponse: // simulationResponse
      case symbol_kind::S_simulationControl: // simulationControl
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        value.copy< obj_t > (that.value);
        break;

      case symbol_kind::S_indexList: // indexList
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_argumentList: // argumentList
        value.copy< std::vector<double> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  genesyspp_parser::stack_symbol_type&
  genesyspp_parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_NUMD: // NUMD
      case symbol_kind::S_NUMH: // NUMH
      case symbol_kind::S_CTEZERO: // CTEZERO
      case symbol_kind::S_oLE: // oLE
      case symbol_kind::S_oGE: // oGE
      case symbol_kind::S_oEQ: // oEQ
      case symbol_kind::S_oNE: // oNE
      case symbol_kind::S_oAND: // oAND
      case symbol_kind::S_oOR: // oOR
      case symbol_kind::S_oNAND: // oNAND
      case symbol_kind::S_oXOR: // oXOR
      case symbol_kind::S_oNOT: // oNOT
      case symbol_kind::S_fSIN: // fSIN
      case symbol_kind::S_fCOS: // fCOS
      case symbol_kind::S_fROUND: // fROUND
      case symbol_kind::S_fMOD: // fMOD
      case symbol_kind::S_fTRUNC: // fTRUNC
      case symbol_kind::S_fFRAC: // fFRAC
      case symbol_kind::S_fEXP: // fEXP
      case symbol_kind::S_fSQRT: // fSQRT
      case symbol_kind::S_fLOG: // fLOG
      case symbol_kind::S_fLN: // fLN
      case symbol_kind::S_mathMIN: // mathMIN
      case symbol_kind::S_mathMAX: // mathMAX
      case symbol_kind::S_fVAL: // fVAL
      case symbol_kind::S_fEVAL: // fEVAL
      case symbol_kind::S_fLENG: // fLENG
      case symbol_kind::S_fRND1: // fRND1
      case symbol_kind::S_fEXPO: // fEXPO
      case symbol_kind::S_fNORM: // fNORM
      case symbol_kind::S_fUNIF: // fUNIF
      case symbol_kind::S_fWEIB: // fWEIB
      case symbol_kind::S_fLOGN: // fLOGN
      case symbol_kind::S_fGAMM: // fGAMM
      case symbol_kind::S_fERLA: // fERLA
      case symbol_kind::S_fTRIA: // fTRIA
      case symbol_kind::S_fBETA: // fBETA
      case symbol_kind::S_fDISC: // fDISC
      case symbol_kind::S_fTNOW: // fTNOW
      case symbol_kind::S_fTFIN: // fTFIN
      case symbol_kind::S_fMAXREP: // fMAXREP
      case symbol_kind::S_fNUMREP: // fNUMREP
      case symbol_kind::S_fIDENT: // fIDENT
      case symbol_kind::S_simulEntitiesWIP: // simulEntitiesWIP
      case symbol_kind::S_cIF: // cIF
      case symbol_kind::S_cELSE: // cELSE
      case symbol_kind::S_cFOR: // cFOR
      case symbol_kind::S_cTO: // cTO
      case symbol_kind::S_cDO: // cDO
      case symbol_kind::S_ATRIB: // ATRIB
      case symbol_kind::S_CSTAT: // CSTAT
      case symbol_kind::S_COUNTER: // COUNTER
      case symbol_kind::S_SIMRESP: // SIMRESP
      case symbol_kind::S_SIMCTRL: // SIMCTRL
      case symbol_kind::S_fTAVG: // fTAVG
      case symbol_kind::S_fCOUNT: // fCOUNT
      case symbol_kind::S_ILLEGAL: // ILLEGAL
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_RESOURCE: // RESOURCE
      case symbol_kind::S_fNR: // fNR
      case symbol_kind::S_fMR: // fMR
      case symbol_kind::S_fIRF: // fIRF
      case symbol_kind::S_fRESSEIZES: // fRESSEIZES
      case symbol_kind::S_fSTATE: // fSTATE
      case symbol_kind::S_fSETSUM: // fSETSUM
      case symbol_kind::S_fRESUTIL: // fRESUTIL
      case symbol_kind::S_QUEUE: // QUEUE
      case symbol_kind::S_fNQ: // fNQ
      case symbol_kind::S_fFIRSTINQ: // fFIRSTINQ
      case symbol_kind::S_fLASTINQ: // fLASTINQ
      case symbol_kind::S_fSAQUE: // fSAQUE
      case symbol_kind::S_fAQUE: // fAQUE
      case symbol_kind::S_fENTATRANK: // fENTATRANK
      case symbol_kind::S_SET: // SET
      case symbol_kind::S_fNUMSET: // fNUMSET
      case symbol_kind::S_VARI: // VARI
      case symbol_kind::S_FORM: // FORM
      case symbol_kind::S_fNUMGR: // fNUMGR
      case symbol_kind::S_fATRGR: // fATRGR
      case symbol_kind::S_input: // input
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_logicalOr: // logicalOr
      case symbol_kind::S_logicalXor: // logicalXor
      case symbol_kind::S_logicalAnd: // logicalAnd
      case symbol_kind::S_logicalNot: // logicalNot
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_number: // number
      case symbol_kind::S_command: // command
      case symbol_kind::S_commandIF: // commandIF
      case symbol_kind::S_commandFOR: // commandFOR
      case symbol_kind::S_function: // function
      case symbol_kind::S_kernelFunction: // kernelFunction
      case symbol_kind::S_elementFunction: // elementFunction
      case symbol_kind::S_trigonFunction: // trigonFunction
      case symbol_kind::S_mathFunction: // mathFunction
      case symbol_kind::S_probFunction: // probFunction
      case symbol_kind::S_userFunction: // userFunction
      case symbol_kind::S_listaparm: // listaparm
      case symbol_kind::S_genericFunction: // genericFunction
      case symbol_kind::S_illegal: // illegal
      case symbol_kind::S_attribute: // attribute
      case symbol_kind::S_simulationResponse: // simulationResponse
      case symbol_kind::S_simulationControl: // simulationControl
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        value.move< obj_t > (that.value);
        break;

      case symbol_kind::S_indexList: // indexList
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_argumentList: // argumentList
        value.move< std::vector<double> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  genesyspp_parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  genesyspp_parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  genesyspp_parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  genesyspp_parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  genesyspp_parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  genesyspp_parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  genesyspp_parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  genesyspp_parser::debug_level_type
  genesyspp_parser::debug_level () const
  {
    return yydebug_;
  }

  void
  genesyspp_parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  genesyspp_parser::state_type
  genesyspp_parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  genesyspp_parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  genesyspp_parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  genesyspp_parser::operator() ()
  {
    return parse ();
  }

  int
  genesyspp_parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
#line 56 "bisonparser.yy"
{
  // Initialize the initial location.
  //@$.begin.filename = @$.end.filename = &driver.getFile();
}

#line 934 "../GenesysParser.cpp"


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (driver));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_NUMD: // NUMD
      case symbol_kind::S_NUMH: // NUMH
      case symbol_kind::S_CTEZERO: // CTEZERO
      case symbol_kind::S_oLE: // oLE
      case symbol_kind::S_oGE: // oGE
      case symbol_kind::S_oEQ: // oEQ
      case symbol_kind::S_oNE: // oNE
      case symbol_kind::S_oAND: // oAND
      case symbol_kind::S_oOR: // oOR
      case symbol_kind::S_oNAND: // oNAND
      case symbol_kind::S_oXOR: // oXOR
      case symbol_kind::S_oNOT: // oNOT
      case symbol_kind::S_fSIN: // fSIN
      case symbol_kind::S_fCOS: // fCOS
      case symbol_kind::S_fROUND: // fROUND
      case symbol_kind::S_fMOD: // fMOD
      case symbol_kind::S_fTRUNC: // fTRUNC
      case symbol_kind::S_fFRAC: // fFRAC
      case symbol_kind::S_fEXP: // fEXP
      case symbol_kind::S_fSQRT: // fSQRT
      case symbol_kind::S_fLOG: // fLOG
      case symbol_kind::S_fLN: // fLN
      case symbol_kind::S_mathMIN: // mathMIN
      case symbol_kind::S_mathMAX: // mathMAX
      case symbol_kind::S_fVAL: // fVAL
      case symbol_kind::S_fEVAL: // fEVAL
      case symbol_kind::S_fLENG: // fLENG
      case symbol_kind::S_fRND1: // fRND1
      case symbol_kind::S_fEXPO: // fEXPO
      case symbol_kind::S_fNORM: // fNORM
      case symbol_kind::S_fUNIF: // fUNIF
      case symbol_kind::S_fWEIB: // fWEIB
      case symbol_kind::S_fLOGN: // fLOGN
      case symbol_kind::S_fGAMM: // fGAMM
      case symbol_kind::S_fERLA: // fERLA
      case symbol_kind::S_fTRIA: // fTRIA
      case symbol_kind::S_fBETA: // fBETA
      case symbol_kind::S_fDISC: // fDISC
      case symbol_kind::S_fTNOW: // fTNOW
      case symbol_kind::S_fTFIN: // fTFIN
      case symbol_kind::S_fMAXREP: // fMAXREP
      case symbol_kind::S_fNUMREP: // fNUMREP
      case symbol_kind::S_fIDENT: // fIDENT
      case symbol_kind::S_simulEntitiesWIP: // simulEntitiesWIP
      case symbol_kind::S_cIF: // cIF
      case symbol_kind::S_cELSE: // cELSE
      case symbol_kind::S_cFOR: // cFOR
      case symbol_kind::S_cTO: // cTO
      case symbol_kind::S_cDO: // cDO
      case symbol_kind::S_ATRIB: // ATRIB
      case symbol_kind::S_CSTAT: // CSTAT
      case symbol_kind::S_COUNTER: // COUNTER
      case symbol_kind::S_SIMRESP: // SIMRESP
      case symbol_kind::S_SIMCTRL: // SIMCTRL
      case symbol_kind::S_fTAVG: // fTAVG
      case symbol_kind::S_fCOUNT: // fCOUNT
      case symbol_kind::S_ILLEGAL: // ILLEGAL
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_RESOURCE: // RESOURCE
      case symbol_kind::S_fNR: // fNR
      case symbol_kind::S_fMR: // fMR
      case symbol_kind::S_fIRF: // fIRF
      case symbol_kind::S_fRESSEIZES: // fRESSEIZES
      case symbol_kind::S_fSTATE: // fSTATE
      case symbol_kind::S_fSETSUM: // fSETSUM
      case symbol_kind::S_fRESUTIL: // fRESUTIL
      case symbol_kind::S_QUEUE: // QUEUE
      case symbol_kind::S_fNQ: // fNQ
      case symbol_kind::S_fFIRSTINQ: // fFIRSTINQ
      case symbol_kind::S_fLASTINQ: // fLASTINQ
      case symbol_kind::S_fSAQUE: // fSAQUE
      case symbol_kind::S_fAQUE: // fAQUE
      case symbol_kind::S_fENTATRANK: // fENTATRANK
      case symbol_kind::S_SET: // SET
      case symbol_kind::S_fNUMSET: // fNUMSET
      case symbol_kind::S_VARI: // VARI
      case symbol_kind::S_FORM: // FORM
      case symbol_kind::S_fNUMGR: // fNUMGR
      case symbol_kind::S_fATRGR: // fATRGR
      case symbol_kind::S_input: // input
      case symbol_kind::S_expression: // expression
      case symbol_kind::S_logicalOr: // logicalOr
      case symbol_kind::S_logicalXor: // logicalXor
      case symbol_kind::S_logicalAnd: // logicalAnd
      case symbol_kind::S_logicalNot: // logicalNot
      case symbol_kind::S_relational: // relational
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_number: // number
      case symbol_kind::S_command: // command
      case symbol_kind::S_commandIF: // commandIF
      case symbol_kind::S_commandFOR: // commandFOR
      case symbol_kind::S_function: // function
      case symbol_kind::S_kernelFunction: // kernelFunction
      case symbol_kind::S_elementFunction: // elementFunction
      case symbol_kind::S_trigonFunction: // trigonFunction
      case symbol_kind::S_mathFunction: // mathFunction
      case symbol_kind::S_probFunction: // probFunction
      case symbol_kind::S_userFunction: // userFunction
      case symbol_kind::S_listaparm: // listaparm
      case symbol_kind::S_genericFunction: // genericFunction
      case symbol_kind::S_illegal: // illegal
      case symbol_kind::S_attribute: // attribute
      case symbol_kind::S_simulationResponse: // simulationResponse
      case symbol_kind::S_simulationControl: // simulationControl
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        yylhs.value.emplace< obj_t > ();
        break;

      case symbol_kind::S_indexList: // indexList
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_argumentList: // argumentList
        yylhs.value.emplace< std::vector<double> > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // input: expression
#line 291 "bisonparser.yy"
                    { driver.setResult(yystack_[0].value.as < obj_t > ().valor);}
#line 1195 "../GenesysParser.cpp"
    break;

  case 3: // expression: assigment
#line 296 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1201 "../GenesysParser.cpp"
    break;

  case 4: // expression: command
#line 297 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1207 "../GenesysParser.cpp"
    break;

  case 5: // expression: logicalOr
#line 298 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1213 "../GenesysParser.cpp"
    break;

  case 6: // expression: illegal
#line 299 "bisonparser.yy"
                                        {yylhs.value.as < obj_t > ().valor = -1;}
#line 1219 "../GenesysParser.cpp"
    break;

  case 7: // logicalOr: logicalOr oOR logicalXor
#line 303 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (int)yystack_[2].value.as < obj_t > ().valor || (int)yystack_[0].value.as < obj_t > ().valor; }
#line 1225 "../GenesysParser.cpp"
    break;

  case 8: // logicalOr: logicalXor
#line 304 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1231 "../GenesysParser.cpp"
    break;

  case 9: // logicalXor: logicalXor oXOR logicalAnd
#line 308 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (!(int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor) || ((int)yystack_[2].value.as < obj_t > ().valor && !(int)yystack_[0].value.as < obj_t > ().valor); }
#line 1237 "../GenesysParser.cpp"
    break;

  case 10: // logicalXor: logicalAnd
#line 309 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1243 "../GenesysParser.cpp"
    break;

  case 11: // logicalAnd: logicalAnd oAND logicalNot
#line 313 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor; }
#line 1249 "../GenesysParser.cpp"
    break;

  case 12: // logicalAnd: logicalAnd oNAND logicalNot
#line 314 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = !((int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor); }
#line 1255 "../GenesysParser.cpp"
    break;

  case 13: // logicalAnd: logicalNot
#line 315 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1261 "../GenesysParser.cpp"
    break;

  case 14: // logicalNot: oNOT logicalNot
#line 319 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = !(int)yystack_[0].value.as < obj_t > ().valor; }
#line 1267 "../GenesysParser.cpp"
    break;

  case 15: // logicalNot: relational
#line 320 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1273 "../GenesysParser.cpp"
    break;

  case 16: // relational: relational "<" additive
#line 324 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor < yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1279 "../GenesysParser.cpp"
    break;

  case 17: // relational: relational ">" additive
#line 325 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor > yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1285 "../GenesysParser.cpp"
    break;

  case 18: // relational: relational oLE additive
#line 326 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor <= yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1291 "../GenesysParser.cpp"
    break;

  case 19: // relational: relational oGE additive
#line 327 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor >= yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1297 "../GenesysParser.cpp"
    break;

  case 20: // relational: relational oEQ additive
#line 328 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor == yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1303 "../GenesysParser.cpp"
    break;

  case 21: // relational: relational oNE additive
#line 329 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor != yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1309 "../GenesysParser.cpp"
    break;

  case 22: // relational: additive
#line 330 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1315 "../GenesysParser.cpp"
    break;

  case 23: // additive: additive "+" multiplicative
#line 334 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor + yystack_[0].value.as < obj_t > ().valor; }
#line 1321 "../GenesysParser.cpp"
    break;

  case 24: // additive: additive "-" multiplicative
#line 335 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor - yystack_[0].value.as < obj_t > ().valor; }
#line 1327 "../GenesysParser.cpp"
    break;

  case 25: // additive: multiplicative
#line 336 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1333 "../GenesysParser.cpp"
    break;

  case 26: // multiplicative: multiplicative "*" power
#line 340 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor * yystack_[0].value.as < obj_t > ().valor; }
#line 1339 "../GenesysParser.cpp"
    break;

  case 27: // multiplicative: multiplicative "/" power
#line 341 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor / yystack_[0].value.as < obj_t > ().valor; }
#line 1345 "../GenesysParser.cpp"
    break;

  case 28: // multiplicative: power
#line 342 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1351 "../GenesysParser.cpp"
    break;

  case 29: // power: unary "^" power
#line 346 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = pow(yystack_[2].value.as < obj_t > ().valor, yystack_[0].value.as < obj_t > ().valor); }
#line 1357 "../GenesysParser.cpp"
    break;

  case 30: // power: unary
#line 347 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1363 "../GenesysParser.cpp"
    break;

  case 31: // unary: "-" unary
#line 351 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = -yystack_[0].value.as < obj_t > ().valor; }
#line 1369 "../GenesysParser.cpp"
    break;

  case 32: // unary: "+" unary
#line 352 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = +yystack_[0].value.as < obj_t > ().valor; }
#line 1375 "../GenesysParser.cpp"
    break;

  case 33: // unary: primary
#line 353 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1381 "../GenesysParser.cpp"
    break;

  case 34: // primary: number
#line 357 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1387 "../GenesysParser.cpp"
    break;

  case 35: // primary: function
#line 358 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1393 "../GenesysParser.cpp"
    break;

  case 36: // primary: "(" expression ")"
#line 359 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor;}
#line 1399 "../GenesysParser.cpp"
    break;

  case 37: // primary: attribute
#line 360 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1405 "../GenesysParser.cpp"
    break;

  case 38: // primary: simulationResponse
#line 361 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1411 "../GenesysParser.cpp"
    break;

  case 39: // primary: simulationControl
#line 362 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1417 "../GenesysParser.cpp"
    break;

  case 40: // primary: variable
#line 367 "bisonparser.yy"
                                              {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1423 "../GenesysParser.cpp"
    break;

  case 41: // primary: formula
#line 371 "bisonparser.yy"
                                              {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1429 "../GenesysParser.cpp"
    break;

  case 42: // number: NUMD
#line 378 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1435 "../GenesysParser.cpp"
    break;

  case 43: // number: NUMH
#line 379 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1441 "../GenesysParser.cpp"
    break;

  case 44: // command: commandIF
#line 383 "bisonparser.yy"
                    { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1447 "../GenesysParser.cpp"
    break;

  case 45: // command: commandFOR
#line 384 "bisonparser.yy"
                    { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1453 "../GenesysParser.cpp"
    break;

  case 46: // commandIF: cIF "(" expression "," expression "," expression ")"
#line 388 "bisonparser.yy"
                                                           { yylhs.value.as < obj_t > ().valor = yystack_[5].value.as < obj_t > ().valor != 0 ? yystack_[3].value.as < obj_t > ().valor : yystack_[1].value.as < obj_t > ().valor; }
#line 1459 "../GenesysParser.cpp"
    break;

  case 47: // commandIF: cIF "(" expression "," expression ")"
#line 389 "bisonparser.yy"
                                                            { yylhs.value.as < obj_t > ().valor = yystack_[3].value.as < obj_t > ().valor != 0 ? yystack_[1].value.as < obj_t > ().valor : 0; }
#line 1465 "../GenesysParser.cpp"
    break;

  case 48: // commandFOR: cFOR variable "=" expression cTO expression cDO assigment
#line 394 "bisonparser.yy"
                                                                {yylhs.value.as < obj_t > ().valor = 0; }
#line 1471 "../GenesysParser.cpp"
    break;

  case 49: // commandFOR: cFOR attribute "=" expression cTO expression cDO assigment
#line 395 "bisonparser.yy"
                                                                  {yylhs.value.as < obj_t > ().valor = 0; }
#line 1477 "../GenesysParser.cpp"
    break;

  case 50: // function: mathFunction
#line 399 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1483 "../GenesysParser.cpp"
    break;

  case 51: // function: trigonFunction
#line 400 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1489 "../GenesysParser.cpp"
    break;

  case 52: // function: probFunction
#line 401 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1495 "../GenesysParser.cpp"
    break;

  case 53: // function: kernelFunction
#line 402 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1501 "../GenesysParser.cpp"
    break;

  case 54: // function: elementFunction
#line 403 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1507 "../GenesysParser.cpp"
    break;

  case 55: // function: pluginFunction
#line 404 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1513 "../GenesysParser.cpp"
    break;

  case 56: // function: userFunction
#line 405 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1519 "../GenesysParser.cpp"
    break;

  case 57: // function: genericFunction
#line 406 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1525 "../GenesysParser.cpp"
    break;

  case 58: // kernelFunction: fTNOW
#line 410 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getSimulatedTime();}
#line 1531 "../GenesysParser.cpp"
    break;

  case 59: // kernelFunction: fTFIN
#line 411 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getReplicationLength();}
#line 1537 "../GenesysParser.cpp"
    break;

  case 60: // kernelFunction: fMAXREP
#line 412 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getNumberOfReplications();}
#line 1543 "../GenesysParser.cpp"
    break;

  case 61: // kernelFunction: fNUMREP
#line 413 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getCurrentReplicationNumber();}
#line 1549 "../GenesysParser.cpp"
    break;

  case 62: // kernelFunction: fIDENT
#line 414 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getId();}
#line 1555 "../GenesysParser.cpp"
    break;

  case 63: // kernelFunction: simulEntitiesWIP
#line 415 "bisonparser.yy"
                            { yylhs.value.as < obj_t > ().valor = driver.getModel()->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Entity>());}
#line 1561 "../GenesysParser.cpp"
    break;

  case 64: // elementFunction: fTAVG "(" CSTAT ")"
#line 420 "bisonparser.yy"
                               {
                    StatisticsCollector* cstat = ((StatisticsCollector*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<StatisticsCollector>(), yystack_[1].value.as < obj_t > ().id)));
                    double value = cstat->getStatistics()->average();
                    yylhs.value.as < obj_t > ().valor = value; }
#line 1570 "../GenesysParser.cpp"
    break;

  case 65: // elementFunction: fCOUNT "(" COUNTER ")"
#line 424 "bisonparser.yy"
                                 {
					Counter* counter = ((Counter*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Counter>(), yystack_[1].value.as < obj_t > ().id)));
                    double value = counter->getCountValue();
                    yylhs.value.as < obj_t > ().valor = value; }
#line 1579 "../GenesysParser.cpp"
    break;

  case 66: // trigonFunction: fSIN "(" expression ")"
#line 431 "bisonparser.yy"
                                  { yylhs.value.as < obj_t > ().valor = sin(yystack_[1].value.as < obj_t > ().valor); }
#line 1585 "../GenesysParser.cpp"
    break;

  case 67: // trigonFunction: fCOS "(" expression ")"
#line 432 "bisonparser.yy"
                                  { yylhs.value.as < obj_t > ().valor = cos(yystack_[1].value.as < obj_t > ().valor); }
#line 1591 "../GenesysParser.cpp"
    break;

  case 68: // mathFunction: fROUND "(" expression ")"
#line 436 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = round(yystack_[1].value.as < obj_t > ().valor);}
#line 1597 "../GenesysParser.cpp"
    break;

  case 69: // mathFunction: fFRAC "(" expression ")"
#line 437 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor - (int) yystack_[1].value.as < obj_t > ().valor;}
#line 1603 "../GenesysParser.cpp"
    break;

  case 70: // mathFunction: fTRUNC "(" expression ")"
#line 438 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = trunc(yystack_[1].value.as < obj_t > ().valor);}
#line 1609 "../GenesysParser.cpp"
    break;

  case 71: // mathFunction: fEXP "(" expression ")"
#line 439 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = exp(yystack_[1].value.as < obj_t > ().valor);}
#line 1615 "../GenesysParser.cpp"
    break;

  case 72: // mathFunction: fSQRT "(" expression ")"
#line 440 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = sqrt(yystack_[1].value.as < obj_t > ().valor);}
#line 1621 "../GenesysParser.cpp"
    break;

  case 73: // mathFunction: fLOG "(" expression ")"
#line 441 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = log10(yystack_[1].value.as < obj_t > ().valor);}
#line 1627 "../GenesysParser.cpp"
    break;

  case 74: // mathFunction: fLN "(" expression ")"
#line 442 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = log(yystack_[1].value.as < obj_t > ().valor);}
#line 1633 "../GenesysParser.cpp"
    break;

  case 75: // mathFunction: fMOD "(" expression "," expression ")"
#line 443 "bisonparser.yy"
                                               { yylhs.value.as < obj_t > ().valor = (int) yystack_[3].value.as < obj_t > ().valor % (int) yystack_[1].value.as < obj_t > ().valor; }
#line 1639 "../GenesysParser.cpp"
    break;

  case 76: // mathFunction: mathMIN "(" expression "," expression ")"
#line 444 "bisonparser.yy"
                                                { yylhs.value.as < obj_t > ().valor = std::min(yystack_[3].value.as < obj_t > ().valor, yystack_[1].value.as < obj_t > ().valor); }
#line 1645 "../GenesysParser.cpp"
    break;

  case 77: // mathFunction: mathMAX "(" expression "," expression ")"
#line 445 "bisonparser.yy"
                                                { yylhs.value.as < obj_t > ().valor = std::max(yystack_[3].value.as < obj_t > ().valor, yystack_[1].value.as < obj_t > ().valor); }
#line 1651 "../GenesysParser.cpp"
    break;

  case 78: // probFunction: fRND1
#line 449 "bisonparser.yy"
                                                     {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleUniform(0.0,1.0); }
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
#line 1675 "../GenesysParser.cpp"
    break;

  case 79: // probFunction: fEXPO "(" expression ")"
#line 468 "bisonparser.yy"
                                     {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleExponential(yystack_[1].value.as < obj_t > ().valor); }
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
#line 1699 "../GenesysParser.cpp"
    break;

  case 80: // probFunction: fNORM "(" expression "," expression ")"
#line 487 "bisonparser.yy"
                                                    {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleNormal(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor); }
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
#line 1723 "../GenesysParser.cpp"
    break;

  case 81: // probFunction: fUNIF "(" expression "," expression ")"
#line 506 "bisonparser.yy"
                                                    {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleUniform(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor); }
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
#line 1747 "../GenesysParser.cpp"
    break;

  case 82: // probFunction: fWEIB "(" expression "," expression ")"
#line 525 "bisonparser.yy"
                                                    {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleWeibull(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor); }
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
#line 1771 "../GenesysParser.cpp"
    break;

  case 83: // probFunction: fLOGN "(" expression "," expression ")"
#line 544 "bisonparser.yy"
                                                    {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleLogNormal(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor); }
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
#line 1795 "../GenesysParser.cpp"
    break;

  case 84: // probFunction: fGAMM "(" expression "," expression ")"
#line 563 "bisonparser.yy"
                                                    {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleGamma(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor); }
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
#line 1819 "../GenesysParser.cpp"
    break;

  case 85: // probFunction: fERLA "(" expression "," expression ")"
#line 582 "bisonparser.yy"
                                                    {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleErlang(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor); }
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
#line 1843 "../GenesysParser.cpp"
    break;

  case 86: // probFunction: fTRIA "(" expression "," expression "," expression ")"
#line 601 "bisonparser.yy"
                                                                    {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleTriangular(yystack_[5].value.as < obj_t > ().valor,yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor); }
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
#line 1867 "../GenesysParser.cpp"
    break;

  case 87: // probFunction: fBETA "(" expression "," expression "," expression "," expression ")"
#line 620 "bisonparser.yy"
                                                                                  {
		try { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleBeta(yystack_[7].value.as < obj_t > ().valor,yystack_[5].value.as < obj_t > ().valor,yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor); }
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
#line 1891 "../GenesysParser.cpp"
    break;

  case 88: // probFunction: fDISC "(" listaparm ")"
#line 639 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleDiscrete(0,0); /*@TODO: NOT IMPLEMENTED YET*/ }
#line 1897 "../GenesysParser.cpp"
    break;

  case 89: // userFunction: "USER" "(" expression ")"
#line 645 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor; }
#line 1903 "../GenesysParser.cpp"
    break;

  case 90: // listaparm: listaparm "," expression "," expression
#line 650 "bisonparser.yy"
                                                 {/*@TODO: NOT IMPLEMENTED YET*/}
#line 1909 "../GenesysParser.cpp"
    break;

  case 91: // listaparm: expression "," expression
#line 651 "bisonparser.yy"
                                                 {/*@TODO: NOT IMPLEMENTED YET*/}
#line 1915 "../GenesysParser.cpp"
    break;

  case 92: // genericFunction: IDENTIFIER "(" ")"
#line 655 "bisonparser.yy"
                         {
		SemanticResolverResult result = resolveGenericFunction(driver, yystack_[2].value.as < obj_t > ().tipo, {});
		if (!result.success) {
			if (driver.getThrowsException()) throw std::string(result.errorMessage);
			YYERROR;
		}
		yylhs.value.as < obj_t > ().valor = result.value;
	}
#line 1928 "../GenesysParser.cpp"
    break;

  case 93: // genericFunction: IDENTIFIER "(" argumentList ")"
#line 663 "bisonparser.yy"
                                      {
		SemanticResolverResult result = resolveGenericFunction(driver, yystack_[3].value.as < obj_t > ().tipo, yystack_[1].value.as < std::vector<double> > ());
		if (!result.success) {
			if (driver.getThrowsException()) throw std::string(result.errorMessage);
			YYERROR;
		}
		yylhs.value.as < obj_t > ().valor = result.value;
	}
#line 1941 "../GenesysParser.cpp"
    break;

  case 94: // argumentList: expression
#line 674 "bisonparser.yy"
                                                  { yylhs.value.as < std::vector<double> > () = std::vector<double>{yystack_[0].value.as < obj_t > ().valor}; }
#line 1947 "../GenesysParser.cpp"
    break;

  case 95: // argumentList: argumentList "," expression
#line 675 "bisonparser.yy"
                                                  { yystack_[2].value.as < std::vector<double> > ().push_back(yystack_[0].value.as < obj_t > ().valor); yylhs.value.as < std::vector<double> > () = yystack_[2].value.as < std::vector<double> > (); }
#line 1953 "../GenesysParser.cpp"
    break;

  case 96: // indexList: expression
#line 679 "bisonparser.yy"
                                                  { yylhs.value.as < std::string > () = parserIndexPart(yystack_[0].value.as < obj_t > ().valor); }
#line 1959 "../GenesysParser.cpp"
    break;

  case 97: // indexList: indexList "," expression
#line 680 "bisonparser.yy"
                                                  { yylhs.value.as < std::string > () = appendParserIndex(yystack_[2].value.as < std::string > (), yystack_[0].value.as < obj_t > ().valor); }
#line 1965 "../GenesysParser.cpp"
    break;

  case 98: // illegal: ILLEGAL
#line 685 "bisonparser.yy"
                          {
		driver.setResult(-1);
		std::string lexema = yystack_[0].value.as < obj_t > ().tipo;
		bool hasLexema = !lexema.empty();
		std::string literalMsg = hasLexema ? std::string("Literal nao encontrado: \"") + lexema + "\"" : std::string("Literal nao encontrado");
		std::string caracterMsg = hasLexema ? std::string("Caracter invalido encontrado: \"") + lexema + "\"" : std::string("Caracter invalido encontrado");
		if(driver.getThrowsException()){
			if(yystack_[0].value.as < obj_t > ().valor == 0){
			  throw literalMsg;
			}else if(yystack_[0].value.as < obj_t > ().valor == 1){
			  throw caracterMsg;
			}
		} else {
			if(yystack_[0].value.as < obj_t > ().valor == 0){
			  driver.setErrorMessage(literalMsg);
			}else if(yystack_[0].value.as < obj_t > ().valor == 1){
				driver.setErrorMessage(caracterMsg);
			}
		}
	}
#line 1990 "../GenesysParser.cpp"
    break;

  case 99: // illegal: IDENTIFIER
#line 705 "bisonparser.yy"
                          {
		driver.setResult(-1);
		std::string lexema = yystack_[0].value.as < obj_t > ().tipo;
		std::string literalMsg = lexema.empty() ? std::string("Literal nao encontrado") : std::string("Literal nao encontrado: \"") + lexema + "\"";
		if(driver.getThrowsException()){
			throw literalMsg;
		} else {
			driver.setErrorMessage(literalMsg);
		}
	}
#line 2005 "../GenesysParser.cpp"
    break;

  case 100: // attribute: ATRIB
#line 720 "bisonparser.yy"
                   {  
		double attributeValue = 0.0;
		//std::cout << "Tentando..." << std::endl;
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[0].value.as < obj_t > ().id);
		}
		//std::cout << "Passei" << std::endl;
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 2020 "../GenesysParser.cpp"
    break;

  case 101: // attribute: ATRIB "[" indexList "]"
#line 730 "bisonparser.yy"
                                             {
		double attributeValue = 0.0;
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[3].value.as < obj_t > ().id, yystack_[1].value.as < std::string > ());
		}
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 2033 "../GenesysParser.cpp"
    break;

  case 102: // simulationResponse: SIMRESP
#line 741 "bisonparser.yy"
                   {
		yylhs.value.as < obj_t > ().valor = driver.getSimulationResponseValueAsDouble(yystack_[0].value.as < obj_t > ().tipo);
	}
#line 2041 "../GenesysParser.cpp"
    break;

  case 103: // simulationControl: SIMCTRL
#line 747 "bisonparser.yy"
                   {
		yylhs.value.as < obj_t > ().valor = driver.getSimulationControlValueAsDouble(yystack_[0].value.as < obj_t > ().tipo);
	}
#line 2049 "../GenesysParser.cpp"
    break;

  case 104: // variable: VARI
#line 755 "bisonparser.yy"
                            {yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[0].value.as < obj_t > ().id)))->getValue();}
#line 2055 "../GenesysParser.cpp"
    break;

  case 105: // variable: VARI "[" indexList "]"
#line 756 "bisonparser.yy"
                                                                            {
					yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[3].value.as < obj_t > ().id)))->getValue(yystack_[1].value.as < std::string > ()); }
#line 2062 "../GenesysParser.cpp"
    break;

  case 106: // formula: FORM
#line 763 "bisonparser.yy"
                                    { 
					std::string index = "";
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[0].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 2074 "../GenesysParser.cpp"
    break;

  case 107: // formula: FORM "[" expression "]"
#line 770 "bisonparser.yy"
                                                                    {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[3].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 2086 "../GenesysParser.cpp"
    break;

  case 108: // formula: FORM "[" expression "," expression "]"
#line 777 "bisonparser.yy"
                                                                                   {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor)) +","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[5].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 2098 "../GenesysParser.cpp"
    break;

  case 109: // formula: FORM "[" expression "," expression "," expression "]"
#line 784 "bisonparser.yy"
                                                                                                  {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor)) +","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[7].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 2110 "../GenesysParser.cpp"
    break;

  case 110: // assigment: ATRIB "=" expression
#line 796 "bisonparser.yy"
                                                { 
					// @TODO: getCurrentEvent()->getEntity() may be nullptr if simulation hasn't started yet
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[2].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2119 "../GenesysParser.cpp"
    break;

  case 111: // assigment: ATRIB "[" indexList "]" "=" expression
#line 800 "bisonparser.yy"
                                                                                         {
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[5].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor, yystack_[3].value.as < std::string > ());
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2127 "../GenesysParser.cpp"
    break;

  case 112: // assigment: VARI "=" expression
#line 805 "bisonparser.yy"
                                                                {
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[2].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; 
					}
#line 2136 "../GenesysParser.cpp"
    break;

  case 113: // assigment: VARI "[" indexList "]" "=" expression
#line 809 "bisonparser.yy"
                                                                                        {
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[5].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor, yystack_[3].value.as < std::string > ());
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2144 "../GenesysParser.cpp"
    break;

  case 114: // pluginFunction: CTEZERO
#line 819 "bisonparser.yy"
                                                     { yylhs.value.as < obj_t > ().valor = 0; }
#line 2150 "../GenesysParser.cpp"
    break;

  case 115: // pluginFunction: fNQ "(" QUEUE ")"
#line 822 "bisonparser.yy"
                                    {   //std::cout << "Queue ID: " << $3.id << ", Size: " << ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->size() << std::endl; 
                                        yylhs.value.as < obj_t > ().valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->size();}
#line 2157 "../GenesysParser.cpp"
    break;

  case 116: // pluginFunction: fLASTINQ "(" QUEUE ")"
#line 824 "bisonparser.yy"
                                    {/*For now does nothing because need acces to list of QUEUE, or at least the last element*/ }
#line 2163 "../GenesysParser.cpp"
    break;

  case 117: // pluginFunction: fFIRSTINQ "(" QUEUE ")"
#line 825 "bisonparser.yy"
                                    { 
                if (((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->size() > 0){
                    //id da 1a entidade da fila, talvez pegar nome
                    yylhs.value.as < obj_t > ().valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->first()->getEntity()->getId();
                }else{
                    yylhs.value.as < obj_t > ().valor = 0;
                } }
#line 2175 "../GenesysParser.cpp"
    break;

  case 118: // pluginFunction: fSAQUE "(" QUEUE "," ATRIB ")"
#line 832 "bisonparser.yy"
                                       {   
                //Util::identification queueID = $3.id;
                Util::identification attrID = yystack_[1].value.as < obj_t > ().id;
                double sum = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[3].value.as < obj_t > ().id)))->sumAttributesFromWaiting(attrID);
                yylhs.value.as < obj_t > ().valor = sum; }
#line 2185 "../GenesysParser.cpp"
    break;

  case 119: // pluginFunction: fAQUE "(" QUEUE "," NUMD "," ATRIB ")"
#line 837 "bisonparser.yy"
                                             {
                //Util::identification queueID = $3.id;
                Util::identification attrID = yystack_[1].value.as < obj_t > ().id;
                double value = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[5].value.as < obj_t > ().id)))->getAttributeFromWaitingRank(yystack_[3].value.as < obj_t > ().valor-1, attrID); // rank starts on 0 in genesys
                yylhs.value.as < obj_t > ().valor = value; }
#line 2195 "../GenesysParser.cpp"
    break;

  case 120: // pluginFunction: fMR "(" RESOURCE ")"
#line 845 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getCapacity();}
#line 2201 "../GenesysParser.cpp"
    break;

  case 121: // pluginFunction: fNR "(" RESOURCE ")"
#line 846 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getNumberBusy();}
#line 2207 "../GenesysParser.cpp"
    break;

  case 122: // pluginFunction: fRESSEIZES "(" RESOURCE ")"
#line 847 "bisonparser.yy"
                                         { /*\TODO: For now does nothing because needs get Seizes, check with teacher*/}
#line 2213 "../GenesysParser.cpp"
    break;

  case 123: // pluginFunction: fSTATE "(" RESOURCE ")"
#line 848 "bisonparser.yy"
                                         {  yylhs.value.as < obj_t > ().valor = static_cast<int>(((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getResourceState()); }
#line 2219 "../GenesysParser.cpp"
    break;

  case 124: // pluginFunction: fIRF "(" RESOURCE ")"
#line 849 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getResourceState() == Resource::ResourceState::FAILED ? 1 : 0; }
#line 2225 "../GenesysParser.cpp"
    break;

  case 125: // pluginFunction: fSETSUM "(" SET ")"
#line 850 "bisonparser.yy"
                              {
                unsigned int count=0;
                Resource* res;
                List<ModelDataDefinition*>* setList = ((Set*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(),yystack_[1].value.as < obj_t > ().id))->getElementSet(); 
                for (std::list<ModelDataDefinition*>::iterator it = setList->list()->begin(); it!=setList->list()->end(); it++) {
                    res = dynamic_cast<Resource*>(*it);
                    if (res != nullptr) {
                        if (res->getResourceState()==Resource::ResourceState::BUSY) {
                            count++;
                        }
                    }
                }
                yylhs.value.as < obj_t > ().valor = count; }
#line 2243 "../GenesysParser.cpp"
    break;

  case 126: // pluginFunction: fNUMSET "(" SET ")"
#line 866 "bisonparser.yy"
                                { yylhs.value.as < obj_t > ().valor = ((Set*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(),yystack_[1].value.as < obj_t > ().id))->getElementSet()->size(); }
#line 2249 "../GenesysParser.cpp"
    break;


#line 2253 "../GenesysParser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  genesyspp_parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  genesyspp_parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  genesyspp_parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // genesyspp_parser::context.
  genesyspp_parser::context::context (const genesyspp_parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  genesyspp_parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
  genesyspp_parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  genesyspp_parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short genesyspp_parser::yypact_ninf_ = -263;

  const signed char genesyspp_parser::yytable_ninf_ = -1;

  const short
  genesyspp_parser::yypact_[] =
  {
     410,  -263,  -263,  -263,   495,   -64,   -37,   -32,   -28,   -26,
     -10,    -9,    -8,    -7,    -6,    -5,    -4,  -263,    -1,     1,
       2,     4,     5,     8,     9,    11,    12,    13,  -263,  -263,
    -263,  -263,  -263,  -263,    14,   -45,   -70,  -263,  -263,    38,
      39,  -263,    40,    42,    43,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,   -67,   -68,   410,   580,   580,
      55,    79,  -263,    69,    72,     3,  -263,    -3,   -22,   -35,
    -263,   -77,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,    58,    40,    59,  -263,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     -11,    56,   410,   410,    85,    90,   325,    31,    78,    89,
      91,    92,    70,    82,    86,    87,    88,    93,    83,   410,
     410,   410,    71,  -263,  -263,   410,  -263,   495,   495,   495,
     495,   580,   580,   580,   580,   580,   580,   580,   580,   580,
     580,   580,   410,   410,    75,    77,    80,    67,    81,    84,
      94,    95,    96,    97,    74,    76,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   -74,   108,   410,   410,
    -263,   -66,  -263,   109,   120,  -263,  -263,   -73,   121,   122,
     123,   124,   127,   128,   129,   130,   131,   125,   126,   132,
     -58,  -263,   -56,  -263,   133,    72,     3,  -263,  -263,   -22,
     -22,   -22,   -22,   -22,   -22,   -35,   -35,  -263,  -263,  -263,
     -55,   -54,  -263,  -263,  -263,   410,  -263,  -263,  -263,  -263,
    -263,  -263,   410,   410,  -263,   410,   410,   410,   410,   410,
     410,   410,   410,   410,  -263,   410,   410,   115,   116,   134,
     410,  -263,  -263,  -263,   410,  -263,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,   117,   168,  -263,   135,  -263,   410,
    -263,  -263,  -263,   138,   139,   140,   141,   142,   143,   146,
     147,   148,   144,   145,  -263,   150,   -72,   410,   410,   410,
    -263,  -263,   149,   160,   410,   -53,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,  -263,   410,   410,   410,  -263,   410,
     136,   166,  -263,  -263,   181,  -263,  -263,   410,   151,   163,
    -263,   153,   -44,   -44,   154,   155,  -263,   410,  -263,   -49,
     -47,  -263,  -263,  -263,  -263,   158,   410,   410,  -263,   -43,
     -42,   134,   135
  };

  const signed char
  genesyspp_parser::yydefact_[] =
  {
       0,    42,    43,   114,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,    61,    62,    63,     0,     0,   100,   102,   103,     0,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   106,     0,     0,     0,
       0,     0,     2,     5,     8,    10,    13,    15,    22,    25,
      28,    30,    33,    34,     4,    44,    45,    35,    53,    54,
      51,    50,    52,    56,    57,     6,    37,    38,    39,    40,
      41,     3,    55,   100,     0,   104,    14,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    32,    31,     0,     1,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      96,     0,   110,     0,     0,    92,    94,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   112,     0,    36,     0,     7,     9,    11,    12,    18,
      19,    20,    21,    16,    17,    23,    24,    26,    27,    29,
       0,     0,    66,    67,    68,     0,    70,    69,    71,    72,
      73,    74,     0,     0,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    88,     0,     0,     0,     0,   101,
       0,    64,    65,    93,     0,   121,   120,   124,   122,   123,
     125,   115,   117,   116,     0,     0,   126,   105,   107,     0,
      89,   101,   105,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    91,     0,     0,     0,     0,     0,
      97,    95,     0,     0,     0,     0,    75,    76,    77,    80,
      81,    82,    83,    84,    85,     0,     0,     0,    47,     0,
       0,     0,   111,   118,     0,   113,   108,     0,     0,     0,
      90,     0,     0,     0,     0,     0,    86,     0,    46,     0,
       0,    49,    48,   119,   109,     0,     0,     0,    87,     0,
       0,     0,     0
  };

  const short
  genesyspp_parser::yypgoto_[] =
  {
    -263,  -263,     0,  -263,    25,    26,    -2,  -263,   -93,   -91,
    -112,    10,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,  -263,  -263,  -263,  -138,  -263,   156,
    -263,  -263,   183,  -263,  -262,  -263
  };

  const unsigned char
  genesyspp_parser::yydefgoto_[] =
  {
       0,    61,   190,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,   186,    84,   197,   191,    85,    86,
      87,    88,    89,    90,    91,    92
  };

  const short
  genesyspp_parser::yytable_[] =
  {
      62,   210,    96,   151,   152,   153,   154,    93,   339,   254,
     263,   318,   161,   149,   122,   150,   141,   139,    97,   259,
     255,   264,   319,   123,   230,   231,   140,   277,   260,   278,
     281,   282,   326,    95,   340,   346,   260,   347,   279,   260,
     260,   327,   351,   352,   123,    98,   140,   227,   228,   229,
      99,   260,   260,   159,   100,   160,   101,   142,   219,   220,
     221,   222,   223,   224,   157,   158,   225,   226,   143,   144,
     341,   342,   102,   103,   104,   105,   106,   107,   108,   146,
     147,   109,   188,   110,   111,   148,   112,   113,   155,   156,
     114,   115,   198,   116,   117,   118,   119,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   187,
     124,   125,   126,   192,   127,   128,   196,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   145,   193,   199,
     211,   212,   162,   163,   194,   214,   203,   217,   218,   189,
     200,   204,   201,   202,   213,   205,   206,   207,   232,   209,
     233,   235,   208,   234,   236,   297,   298,   237,   242,   302,
     243,   303,   215,     0,   216,     0,     0,   238,   239,   240,
     241,   244,     0,     0,     0,     0,     0,   332,   257,   258,
       0,   120,   261,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   256,   262,   265,   266,   267,   268,   349,   350,
     269,   270,   271,   272,   273,   276,   280,   333,   121,   274,
     275,   306,   307,   308,   309,   310,   311,   299,   304,   312,
     313,   314,   323,   334,   336,   283,   338,   343,   315,   316,
     344,   348,   284,   285,   317,   286,   287,   288,   289,   290,
     291,   292,   293,   294,   324,   295,   296,   337,     0,     0,
     300,     0,     0,     0,   301,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   305,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   320,   321,   322,
       0,     0,     0,     0,   325,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   328,   329,   330,     0,   331,
       0,     0,     0,     0,     0,     0,     0,   335,     1,     2,
       3,     0,     0,     0,     0,     0,     0,   345,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,     0,    35,     0,     0,    36,     0,     0,
      37,    38,    39,    40,    41,    42,     0,    43,    44,    45,
      46,    47,    48,     0,     0,    49,    50,    51,    52,    53,
       0,     0,    54,    55,    56,     0,     0,    57,   195,     0,
       0,    58,    59,     1,     2,     3,     0,     0,     0,     0,
      60,     0,     0,     0,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,     0,    35,
       0,     0,    36,     0,     0,    37,    38,    39,    40,    41,
      42,     0,    43,    44,    45,    46,    47,    48,     0,     0,
      49,    50,    51,    52,    53,     0,     0,    54,    55,    56,
       0,     0,    57,     0,     0,     0,    58,    59,     1,     2,
       3,     0,     0,     0,     0,    60,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,     0,     0,     0,     0,     0,    93,     0,     0,
      37,    38,    39,    40,     0,    94,     0,    43,    44,    45,
      46,    47,    48,     0,     0,    49,    50,    51,    52,    53,
       0,     0,    54,    95,    56,     0,     0,    57,     0,     0,
       0,    58,    59,     1,     2,     3,     0,     0,     0,     0,
      60,     0,     0,     0,     0,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,     0,     0,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,     0,     0,     0,
       0,     0,    93,     0,     0,    37,    38,    39,    40,     0,
      94,     0,    43,    44,    45,    46,    47,    48,     0,     0,
      49,    50,    51,    52,    53,     0,     0,    54,    95,    56,
       0,     0,    57,     0,     0,     0,    58,    59,     0,     0,
       0,     0,     0,     0,     0,    60
  };

  const short
  genesyspp_parser::yycheck_[] =
  {
       0,   139,     4,     6,     7,     8,     9,    52,    52,    83,
      83,    83,    89,    10,    84,    12,    84,    84,    82,    85,
      94,    94,    94,    93,   162,   163,    93,    85,    94,    85,
      85,    85,    85,    78,    78,    84,    94,    84,    94,    94,
      94,    94,    85,    85,    93,    82,    93,   159,   160,   161,
      82,    94,    94,    88,    82,    90,    82,    57,   151,   152,
     153,   154,   155,   156,    86,    87,   157,   158,    58,    59,
     332,   333,    82,    82,    82,    82,    82,    82,    82,     0,
      11,    82,    93,    82,    82,    13,    82,    82,    91,    92,
      82,    82,    61,    82,    82,    82,    82,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
      82,    82,    82,   123,    82,    82,   126,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    53,    61,
     140,   141,    84,    84,    54,   145,    76,   149,   150,    93,
      61,    69,    61,    61,    83,    69,    69,    69,    83,    76,
      83,    94,    69,    83,    83,    50,    50,    83,    94,    52,
      94,     3,   147,    -1,   148,    -1,    -1,    83,    83,    83,
      83,    83,    -1,    -1,    -1,    -1,    -1,    51,   188,   189,
      -1,    35,    83,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    83,    83,    83,    83,    83,   346,   347,
      83,    83,    83,    83,    83,    83,    83,    51,    35,    94,
      94,    83,    83,    83,    83,    83,    83,    93,    93,    83,
      83,    83,    83,    52,    83,   235,    83,    83,    94,    94,
      85,    83,   242,   243,    94,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    94,   255,   256,    94,    -1,    -1,
     260,    -1,    -1,    -1,   264,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   279,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   297,   298,   299,
      -1,    -1,    -1,    -1,   304,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   315,   316,   317,    -1,   319,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   327,     3,     4,
       5,    -1,    -1,    -1,    -1,    -1,    -1,   337,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    -1,    -1,
      55,    56,    57,    58,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    77,    78,    79,    -1,    -1,    82,    83,    -1,
      -1,    86,    87,     3,     4,     5,    -1,    -1,    -1,    -1,
      95,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    52,    -1,    -1,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    65,    66,    67,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    77,    78,    79,
      -1,    -1,    82,    -1,    -1,    -1,    86,    87,     3,     4,
       5,    -1,    -1,    -1,    -1,    95,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      55,    56,    57,    58,    -1,    60,    -1,    62,    63,    64,
      65,    66,    67,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    77,    78,    79,    -1,    -1,    82,    -1,    -1,
      -1,    86,    87,     3,     4,     5,    -1,    -1,    -1,    -1,
      95,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    55,    56,    57,    58,    -1,
      60,    -1,    62,    63,    64,    65,    66,    67,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    77,    78,    79,
      -1,    -1,    82,    -1,    -1,    -1,    86,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    95
  };

  const unsigned char
  genesyspp_parser::yystos_[] =
  {
       0,     3,     4,     5,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    49,    52,    55,    56,    57,
      58,    59,    60,    62,    63,    64,    65,    66,    67,    70,
      71,    72,    73,    74,    77,    78,    79,    82,    86,    87,
      95,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   121,   124,   125,   126,   127,   128,
     129,   130,   131,    52,    60,    78,   102,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
     125,   128,    84,    93,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    84,
      93,    84,    98,   107,   107,    82,     0,    11,    13,    10,
      12,     6,     7,     8,     9,    91,    92,    86,    87,    88,
      90,    89,    84,    84,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,   120,    98,    93,    93,
      98,   123,    98,    53,    54,    83,    98,   122,    61,    61,
      61,    61,    61,    76,    69,    69,    69,    69,    69,    76,
     123,    98,    98,    83,    98,   100,   101,   102,   102,   104,
     104,   104,   104,   104,   104,   105,   105,   106,   106,   106,
     123,   123,    83,    83,    83,    94,    83,    83,    83,    83,
      83,    83,    94,    94,    83,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    83,    94,    94,    98,    98,    85,
      94,    83,    83,    83,    94,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    94,    94,    83,    85,    85,    94,
      83,    85,    85,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    50,    50,    93,
      98,    98,    52,     3,    93,    98,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    94,    94,    94,    83,    94,
      98,    98,    98,    83,    94,    98,    85,    94,    98,    98,
      98,    98,    51,    51,    52,    98,    83,    94,    83,    52,
      78,   130,   130,    83,    85,    98,    84,    84,    83,   123,
     123,    85,    85
  };

  const unsigned char
  genesyspp_parser::yyr1_[] =
  {
       0,    96,    97,    98,    98,    98,    98,    99,    99,   100,
     100,   101,   101,   101,   102,   102,   103,   103,   103,   103,
     103,   103,   103,   104,   104,   104,   105,   105,   105,   106,
     106,   107,   107,   107,   108,   108,   108,   108,   108,   108,
     108,   108,   109,   109,   110,   110,   111,   111,   112,   112,
     113,   113,   113,   113,   113,   113,   113,   113,   114,   114,
     114,   114,   114,   114,   115,   115,   116,   116,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   119,
     120,   120,   121,   121,   122,   122,   123,   123,   124,   124,
     125,   125,   126,   127,   128,   128,   129,   129,   129,   129,
     130,   130,   130,   130,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131
  };

  const signed char
  genesyspp_parser::yyr2_[] =
  {
       0,     2,     1,     1,     1,     1,     1,     3,     1,     3,
       1,     3,     3,     1,     2,     1,     3,     3,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     1,     3,
       1,     2,     2,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     8,     6,     8,     8,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     6,     6,     6,     1,     4,
       6,     6,     6,     6,     6,     6,     8,    10,     4,     4,
       5,     3,     3,     4,     1,     3,     1,     3,     1,     1,
       1,     4,     1,     1,     1,     4,     1,     4,     6,     8,
       3,     6,     3,     6,     1,     4,     4,     4,     6,     8,
       4,     4,     4,     4,     4,     4,     4
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const genesyspp_parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "NUMD", "NUMH",
  "CTEZERO", "oLE", "oGE", "oEQ", "oNE", "oAND", "oOR", "oNAND", "oXOR",
  "oNOT", "fSIN", "fCOS", "fROUND", "fMOD", "fTRUNC", "fFRAC", "fEXP",
  "fSQRT", "fLOG", "fLN", "mathMIN", "mathMAX", "fVAL", "fEVAL", "fLENG",
  "fRND1", "fEXPO", "fNORM", "fUNIF", "fWEIB", "fLOGN", "fGAMM", "fERLA",
  "fTRIA", "fBETA", "fDISC", "fTNOW", "fTFIN", "fMAXREP", "fNUMREP",
  "fIDENT", "simulEntitiesWIP", "cIF", "cELSE", "cFOR", "cTO", "cDO",
  "ATRIB", "CSTAT", "COUNTER", "SIMRESP", "SIMCTRL", "fTAVG", "fCOUNT",
  "ILLEGAL", "IDENTIFIER", "RESOURCE", "fNR", "fMR", "fIRF", "fRESSEIZES",
  "fSTATE", "fSETSUM", "fRESUTIL", "QUEUE", "fNQ", "fFIRSTINQ", "fLASTINQ",
  "fSAQUE", "fAQUE", "fENTATRANK", "SET", "fNUMSET", "VARI", "FORM",
  "fNUMGR", "fATRGR", "\"(\"", "\")\"", "\"[\"", "\"]\"", "\"+\"", "\"-\"",
  "\"*\"", "\"^\"", "\"/\"", "\"<\"", "\">\"", "\"=\"", "\",\"",
  "\"USER\"", "$accept", "input", "expression", "logicalOr", "logicalXor",
  "logicalAnd", "logicalNot", "relational", "additive", "multiplicative",
  "power", "unary", "primary", "number", "command", "commandIF",
  "commandFOR", "function", "kernelFunction", "elementFunction",
  "trigonFunction", "mathFunction", "probFunction", "userFunction",
  "listaparm", "genericFunction", "argumentList", "indexList", "illegal",
  "attribute", "simulationResponse", "simulationControl", "variable",
  "formula", "assigment", "pluginFunction", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  genesyspp_parser::yyrline_[] =
  {
       0,   291,   291,   296,   297,   298,   299,   303,   304,   308,
     309,   313,   314,   315,   319,   320,   324,   325,   326,   327,
     328,   329,   330,   334,   335,   336,   340,   341,   342,   346,
     347,   351,   352,   353,   357,   358,   359,   360,   361,   362,
     367,   371,   378,   379,   383,   384,   388,   389,   394,   395,
     399,   400,   401,   402,   403,   404,   405,   406,   410,   411,
     412,   413,   414,   415,   420,   424,   431,   432,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   445,   449,   468,
     487,   506,   525,   544,   563,   582,   601,   620,   639,   645,
     650,   651,   655,   663,   674,   675,   679,   680,   685,   705,
     720,   730,   741,   747,   755,   756,   763,   770,   777,   784,
     796,   800,   805,   809,   819,   822,   824,   825,   832,   837,
     845,   846,   847,   848,   849,   850,   866
  };

  void
  genesyspp_parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  genesyspp_parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // yy
#line 3012 "../GenesysParser.cpp"

#line 873 "bisonparser.yy"

void
yy::genesyspp_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
