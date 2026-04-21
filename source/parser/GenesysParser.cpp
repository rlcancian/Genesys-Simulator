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
#line 62 "parserBisonFlex/bisonparser.yy"

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

std::string parserIndexPart(double value) {
	return SparseValueStore::makeIndexKey(std::vector<unsigned int>{static_cast<unsigned int>(value)});
}

std::string appendParserIndex(const std::string& currentKey, double value) {
	return SparseValueStore::appendIndexKeyFromDouble(currentKey, value);
}

}


#line 73 "GenesysParser.cpp"


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
#line 165 "GenesysParser.cpp"

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
#line 55 "parserBisonFlex/bisonparser.yy"
{
  // Initialize the initial location.
  //@$.begin.filename = @$.end.filename = &driver.getFile();
}

#line 899 "GenesysParser.cpp"


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
#line 276 "parserBisonFlex/bisonparser.yy"
                    { driver.setResult(yystack_[0].value.as < obj_t > ().valor);}
#line 1154 "GenesysParser.cpp"
    break;

  case 3: // expression: assigment
#line 281 "parserBisonFlex/bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1160 "GenesysParser.cpp"
    break;

  case 4: // expression: command
#line 282 "parserBisonFlex/bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1166 "GenesysParser.cpp"
    break;

  case 5: // expression: logicalOr
#line 283 "parserBisonFlex/bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1172 "GenesysParser.cpp"
    break;

  case 6: // expression: illegal
#line 284 "parserBisonFlex/bisonparser.yy"
                                        {yylhs.value.as < obj_t > ().valor = -1;}
#line 1178 "GenesysParser.cpp"
    break;

  case 7: // logicalOr: logicalOr oOR logicalXor
#line 288 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (int)yystack_[2].value.as < obj_t > ().valor || (int)yystack_[0].value.as < obj_t > ().valor; }
#line 1184 "GenesysParser.cpp"
    break;

  case 8: // logicalOr: logicalXor
#line 289 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1190 "GenesysParser.cpp"
    break;

  case 9: // logicalXor: logicalXor oXOR logicalAnd
#line 293 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (!(int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor) || ((int)yystack_[2].value.as < obj_t > ().valor && !(int)yystack_[0].value.as < obj_t > ().valor); }
#line 1196 "GenesysParser.cpp"
    break;

  case 10: // logicalXor: logicalAnd
#line 294 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1202 "GenesysParser.cpp"
    break;

  case 11: // logicalAnd: logicalAnd oAND logicalNot
#line 298 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor; }
#line 1208 "GenesysParser.cpp"
    break;

  case 12: // logicalAnd: logicalAnd oNAND logicalNot
#line 299 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = !((int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor); }
#line 1214 "GenesysParser.cpp"
    break;

  case 13: // logicalAnd: logicalNot
#line 300 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1220 "GenesysParser.cpp"
    break;

  case 14: // logicalNot: oNOT logicalNot
#line 304 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = !(int)yystack_[0].value.as < obj_t > ().valor; }
#line 1226 "GenesysParser.cpp"
    break;

  case 15: // logicalNot: relational
#line 305 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1232 "GenesysParser.cpp"
    break;

  case 16: // relational: relational "<" additive
#line 309 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor < yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1238 "GenesysParser.cpp"
    break;

  case 17: // relational: relational ">" additive
#line 310 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor > yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1244 "GenesysParser.cpp"
    break;

  case 18: // relational: relational oLE additive
#line 311 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor <= yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1250 "GenesysParser.cpp"
    break;

  case 19: // relational: relational oGE additive
#line 312 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor >= yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1256 "GenesysParser.cpp"
    break;

  case 20: // relational: relational oEQ additive
#line 313 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor == yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1262 "GenesysParser.cpp"
    break;

  case 21: // relational: relational oNE additive
#line 314 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor != yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1268 "GenesysParser.cpp"
    break;

  case 22: // relational: additive
#line 315 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1274 "GenesysParser.cpp"
    break;

  case 23: // additive: additive "+" multiplicative
#line 319 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor + yystack_[0].value.as < obj_t > ().valor; }
#line 1280 "GenesysParser.cpp"
    break;

  case 24: // additive: additive "-" multiplicative
#line 320 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor - yystack_[0].value.as < obj_t > ().valor; }
#line 1286 "GenesysParser.cpp"
    break;

  case 25: // additive: multiplicative
#line 321 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1292 "GenesysParser.cpp"
    break;

  case 26: // multiplicative: multiplicative "*" power
#line 325 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor * yystack_[0].value.as < obj_t > ().valor; }
#line 1298 "GenesysParser.cpp"
    break;

  case 27: // multiplicative: multiplicative "/" power
#line 326 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor / yystack_[0].value.as < obj_t > ().valor; }
#line 1304 "GenesysParser.cpp"
    break;

  case 28: // multiplicative: power
#line 327 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1310 "GenesysParser.cpp"
    break;

  case 29: // power: unary "^" power
#line 331 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = pow(yystack_[2].value.as < obj_t > ().valor, yystack_[0].value.as < obj_t > ().valor); }
#line 1316 "GenesysParser.cpp"
    break;

  case 30: // power: unary
#line 332 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1322 "GenesysParser.cpp"
    break;

  case 31: // unary: "-" unary
#line 336 "parserBisonFlex/bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = -yystack_[0].value.as < obj_t > ().valor; }
#line 1328 "GenesysParser.cpp"
    break;

  case 32: // unary: "+" unary
#line 337 "parserBisonFlex/bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = +yystack_[0].value.as < obj_t > ().valor; }
#line 1334 "GenesysParser.cpp"
    break;

  case 33: // unary: primary
#line 338 "parserBisonFlex/bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1340 "GenesysParser.cpp"
    break;

  case 34: // primary: number
#line 342 "parserBisonFlex/bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1346 "GenesysParser.cpp"
    break;

  case 35: // primary: function
#line 343 "parserBisonFlex/bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1352 "GenesysParser.cpp"
    break;

  case 36: // primary: "(" expression ")"
#line 344 "parserBisonFlex/bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor;}
#line 1358 "GenesysParser.cpp"
    break;

  case 37: // primary: attribute
#line 345 "parserBisonFlex/bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1364 "GenesysParser.cpp"
    break;

  case 38: // primary: simulationResponse
#line 346 "parserBisonFlex/bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1370 "GenesysParser.cpp"
    break;

  case 39: // primary: simulationControl
#line 347 "parserBisonFlex/bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1376 "GenesysParser.cpp"
    break;

  case 40: // primary: variable
#line 352 "parserBisonFlex/bisonparser.yy"
                                              {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1382 "GenesysParser.cpp"
    break;

  case 41: // primary: formula
#line 356 "parserBisonFlex/bisonparser.yy"
                                              {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1388 "GenesysParser.cpp"
    break;

  case 42: // number: NUMD
#line 363 "parserBisonFlex/bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1394 "GenesysParser.cpp"
    break;

  case 43: // number: NUMH
#line 364 "parserBisonFlex/bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1400 "GenesysParser.cpp"
    break;

  case 44: // command: commandIF
#line 368 "parserBisonFlex/bisonparser.yy"
                    { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1406 "GenesysParser.cpp"
    break;

  case 45: // command: commandFOR
#line 369 "parserBisonFlex/bisonparser.yy"
                    { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1412 "GenesysParser.cpp"
    break;

  case 46: // commandIF: cIF "(" expression "," expression "," expression ")"
#line 373 "parserBisonFlex/bisonparser.yy"
                                                           { yylhs.value.as < obj_t > ().valor = yystack_[5].value.as < obj_t > ().valor != 0 ? yystack_[3].value.as < obj_t > ().valor : yystack_[1].value.as < obj_t > ().valor; }
#line 1418 "GenesysParser.cpp"
    break;

  case 47: // commandIF: cIF "(" expression "," expression ")"
#line 374 "parserBisonFlex/bisonparser.yy"
                                                            { yylhs.value.as < obj_t > ().valor = yystack_[3].value.as < obj_t > ().valor != 0 ? yystack_[1].value.as < obj_t > ().valor : 0; }
#line 1424 "GenesysParser.cpp"
    break;

  case 48: // commandFOR: cFOR variable "=" expression cTO expression cDO assigment
#line 379 "parserBisonFlex/bisonparser.yy"
                                                                {yylhs.value.as < obj_t > ().valor = 0; }
#line 1430 "GenesysParser.cpp"
    break;

  case 49: // commandFOR: cFOR attribute "=" expression cTO expression cDO assigment
#line 380 "parserBisonFlex/bisonparser.yy"
                                                                  {yylhs.value.as < obj_t > ().valor = 0; }
#line 1436 "GenesysParser.cpp"
    break;

  case 50: // function: mathFunction
#line 384 "parserBisonFlex/bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1442 "GenesysParser.cpp"
    break;

  case 51: // function: trigonFunction
#line 385 "parserBisonFlex/bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1448 "GenesysParser.cpp"
    break;

  case 52: // function: probFunction
#line 386 "parserBisonFlex/bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1454 "GenesysParser.cpp"
    break;

  case 53: // function: kernelFunction
#line 387 "parserBisonFlex/bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1460 "GenesysParser.cpp"
    break;

  case 54: // function: elementFunction
#line 388 "parserBisonFlex/bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1466 "GenesysParser.cpp"
    break;

  case 55: // function: pluginFunction
#line 389 "parserBisonFlex/bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1472 "GenesysParser.cpp"
    break;

  case 56: // function: userFunction
#line 390 "parserBisonFlex/bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1478 "GenesysParser.cpp"
    break;

  case 57: // kernelFunction: fTNOW
#line 394 "parserBisonFlex/bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getSimulatedTime();}
#line 1484 "GenesysParser.cpp"
    break;

  case 58: // kernelFunction: fTFIN
#line 395 "parserBisonFlex/bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getReplicationLength();}
#line 1490 "GenesysParser.cpp"
    break;

  case 59: // kernelFunction: fMAXREP
#line 396 "parserBisonFlex/bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getNumberOfReplications();}
#line 1496 "GenesysParser.cpp"
    break;

  case 60: // kernelFunction: fNUMREP
#line 397 "parserBisonFlex/bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getCurrentReplicationNumber();}
#line 1502 "GenesysParser.cpp"
    break;

  case 61: // kernelFunction: fIDENT
#line 398 "parserBisonFlex/bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getId();}
#line 1508 "GenesysParser.cpp"
    break;

  case 62: // kernelFunction: simulEntitiesWIP
#line 399 "parserBisonFlex/bisonparser.yy"
                            { yylhs.value.as < obj_t > ().valor = driver.getModel()->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Entity>());}
#line 1514 "GenesysParser.cpp"
    break;

  case 63: // elementFunction: fTAVG "(" CSTAT ")"
#line 404 "parserBisonFlex/bisonparser.yy"
                               {
                    StatisticsCollector* cstat = ((StatisticsCollector*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<StatisticsCollector>(), yystack_[1].value.as < obj_t > ().id)));
                    double value = cstat->getStatistics()->average();
                    yylhs.value.as < obj_t > ().valor = value; }
#line 1523 "GenesysParser.cpp"
    break;

  case 64: // elementFunction: fCOUNT "(" COUNTER ")"
#line 408 "parserBisonFlex/bisonparser.yy"
                                 {
					Counter* counter = ((Counter*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Counter>(), yystack_[1].value.as < obj_t > ().id)));
                    double value = counter->getCountValue();
                    yylhs.value.as < obj_t > ().valor = value; }
#line 1532 "GenesysParser.cpp"
    break;

  case 65: // trigonFunction: fSIN "(" expression ")"
#line 415 "parserBisonFlex/bisonparser.yy"
                                  { yylhs.value.as < obj_t > ().valor = sin(yystack_[1].value.as < obj_t > ().valor); }
#line 1538 "GenesysParser.cpp"
    break;

  case 66: // trigonFunction: fCOS "(" expression ")"
#line 416 "parserBisonFlex/bisonparser.yy"
                                  { yylhs.value.as < obj_t > ().valor = cos(yystack_[1].value.as < obj_t > ().valor); }
#line 1544 "GenesysParser.cpp"
    break;

  case 67: // mathFunction: fROUND "(" expression ")"
#line 420 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = round(yystack_[1].value.as < obj_t > ().valor);}
#line 1550 "GenesysParser.cpp"
    break;

  case 68: // mathFunction: fFRAC "(" expression ")"
#line 421 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor - (int) yystack_[1].value.as < obj_t > ().valor;}
#line 1556 "GenesysParser.cpp"
    break;

  case 69: // mathFunction: fTRUNC "(" expression ")"
#line 422 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = trunc(yystack_[1].value.as < obj_t > ().valor);}
#line 1562 "GenesysParser.cpp"
    break;

  case 70: // mathFunction: fEXP "(" expression ")"
#line 423 "parserBisonFlex/bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = exp(yystack_[1].value.as < obj_t > ().valor);}
#line 1568 "GenesysParser.cpp"
    break;

  case 71: // mathFunction: fSQRT "(" expression ")"
#line 424 "parserBisonFlex/bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = sqrt(yystack_[1].value.as < obj_t > ().valor);}
#line 1574 "GenesysParser.cpp"
    break;

  case 72: // mathFunction: fLOG "(" expression ")"
#line 425 "parserBisonFlex/bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = log10(yystack_[1].value.as < obj_t > ().valor);}
#line 1580 "GenesysParser.cpp"
    break;

  case 73: // mathFunction: fLN "(" expression ")"
#line 426 "parserBisonFlex/bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = log(yystack_[1].value.as < obj_t > ().valor);}
#line 1586 "GenesysParser.cpp"
    break;

  case 74: // mathFunction: fMOD "(" expression "," expression ")"
#line 427 "parserBisonFlex/bisonparser.yy"
                                               { yylhs.value.as < obj_t > ().valor = (int) yystack_[3].value.as < obj_t > ().valor % (int) yystack_[1].value.as < obj_t > ().valor; }
#line 1592 "GenesysParser.cpp"
    break;

  case 75: // mathFunction: mathMIN "(" expression "," expression ")"
#line 428 "parserBisonFlex/bisonparser.yy"
                                                { yylhs.value.as < obj_t > ().valor = std::min(yystack_[3].value.as < obj_t > ().valor, yystack_[1].value.as < obj_t > ().valor); }
#line 1598 "GenesysParser.cpp"
    break;

  case 76: // mathFunction: mathMAX "(" expression "," expression ")"
#line 429 "parserBisonFlex/bisonparser.yy"
                                                { yylhs.value.as < obj_t > ().valor = std::max(yystack_[3].value.as < obj_t > ().valor, yystack_[1].value.as < obj_t > ().valor); }
#line 1604 "GenesysParser.cpp"
    break;

  case 77: // probFunction: fRND1
#line 433 "parserBisonFlex/bisonparser.yy"
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
#line 1628 "GenesysParser.cpp"
    break;

  case 78: // probFunction: fEXPO "(" expression ")"
#line 452 "parserBisonFlex/bisonparser.yy"
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
#line 1652 "GenesysParser.cpp"
    break;

  case 79: // probFunction: fNORM "(" expression "," expression ")"
#line 471 "parserBisonFlex/bisonparser.yy"
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
#line 1676 "GenesysParser.cpp"
    break;

  case 80: // probFunction: fUNIF "(" expression "," expression ")"
#line 490 "parserBisonFlex/bisonparser.yy"
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
#line 1700 "GenesysParser.cpp"
    break;

  case 81: // probFunction: fWEIB "(" expression "," expression ")"
#line 509 "parserBisonFlex/bisonparser.yy"
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
#line 1724 "GenesysParser.cpp"
    break;

  case 82: // probFunction: fLOGN "(" expression "," expression ")"
#line 528 "parserBisonFlex/bisonparser.yy"
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
#line 1748 "GenesysParser.cpp"
    break;

  case 83: // probFunction: fGAMM "(" expression "," expression ")"
#line 547 "parserBisonFlex/bisonparser.yy"
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
#line 1772 "GenesysParser.cpp"
    break;

  case 84: // probFunction: fERLA "(" expression "," expression ")"
#line 566 "parserBisonFlex/bisonparser.yy"
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
#line 1796 "GenesysParser.cpp"
    break;

  case 85: // probFunction: fTRIA "(" expression "," expression "," expression ")"
#line 585 "parserBisonFlex/bisonparser.yy"
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
#line 1820 "GenesysParser.cpp"
    break;

  case 86: // probFunction: fBETA "(" expression "," expression "," expression "," expression ")"
#line 604 "parserBisonFlex/bisonparser.yy"
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
#line 1844 "GenesysParser.cpp"
    break;

  case 87: // probFunction: fDISC "(" listaparm ")"
#line 623 "parserBisonFlex/bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleDiscrete(0,0); /*@TODO: NOT IMPLEMENTED YET*/ }
#line 1850 "GenesysParser.cpp"
    break;

  case 88: // userFunction: "USER" "(" expression ")"
#line 629 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor; }
#line 1856 "GenesysParser.cpp"
    break;

  case 89: // listaparm: listaparm "," expression "," expression
#line 634 "parserBisonFlex/bisonparser.yy"
                                                 {/*@TODO: NOT IMPLEMENTED YET*/}
#line 1862 "GenesysParser.cpp"
    break;

  case 90: // listaparm: expression "," expression
#line 635 "parserBisonFlex/bisonparser.yy"
                                                 {/*@TODO: NOT IMPLEMENTED YET*/}
#line 1868 "GenesysParser.cpp"
    break;

  case 91: // indexList: expression
#line 639 "parserBisonFlex/bisonparser.yy"
                                                  { yylhs.value.as < std::string > () = parserIndexPart(yystack_[0].value.as < obj_t > ().valor); }
#line 1874 "GenesysParser.cpp"
    break;

  case 92: // indexList: indexList "," expression
#line 640 "parserBisonFlex/bisonparser.yy"
                                                  { yylhs.value.as < std::string > () = appendParserIndex(yystack_[2].value.as < std::string > (), yystack_[0].value.as < obj_t > ().valor); }
#line 1880 "GenesysParser.cpp"
    break;

  case 93: // illegal: ILLEGAL
#line 645 "parserBisonFlex/bisonparser.yy"
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
#line 1905 "GenesysParser.cpp"
    break;

  case 94: // attribute: ATRIB
#line 670 "parserBisonFlex/bisonparser.yy"
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
#line 1920 "GenesysParser.cpp"
    break;

  case 95: // attribute: ATRIB "[" indexList "]"
#line 680 "parserBisonFlex/bisonparser.yy"
                                             {
		double attributeValue = 0.0;
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[3].value.as < obj_t > ().id, yystack_[1].value.as < std::string > ());
		}
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 1933 "GenesysParser.cpp"
    break;

  case 96: // simulationResponse: SIMRESP
#line 691 "parserBisonFlex/bisonparser.yy"
                   {
		yylhs.value.as < obj_t > ().valor = driver.getSimulationResponseValueAsDouble(yystack_[0].value.as < obj_t > ().tipo);
	}
#line 1941 "GenesysParser.cpp"
    break;

  case 97: // simulationControl: SIMCTRL
#line 697 "parserBisonFlex/bisonparser.yy"
                   {
		yylhs.value.as < obj_t > ().valor = driver.getSimulationControlValueAsDouble(yystack_[0].value.as < obj_t > ().tipo);
	}
#line 1949 "GenesysParser.cpp"
    break;

  case 98: // variable: VARI
#line 705 "parserBisonFlex/bisonparser.yy"
                            {yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[0].value.as < obj_t > ().id)))->getValue();}
#line 1955 "GenesysParser.cpp"
    break;

  case 99: // variable: VARI "[" indexList "]"
#line 706 "parserBisonFlex/bisonparser.yy"
                                                                            {
					yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[3].value.as < obj_t > ().id)))->getValue(yystack_[1].value.as < std::string > ()); }
#line 1962 "GenesysParser.cpp"
    break;

  case 100: // formula: FORM
#line 713 "parserBisonFlex/bisonparser.yy"
                                    { 
					std::string index = "";
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[0].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1974 "GenesysParser.cpp"
    break;

  case 101: // formula: FORM "[" expression "]"
#line 720 "parserBisonFlex/bisonparser.yy"
                                                                    {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[3].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1986 "GenesysParser.cpp"
    break;

  case 102: // formula: FORM "[" expression "," expression "]"
#line 727 "parserBisonFlex/bisonparser.yy"
                                                                                   {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor)) +","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[5].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1998 "GenesysParser.cpp"
    break;

  case 103: // formula: FORM "[" expression "," expression "," expression "]"
#line 734 "parserBisonFlex/bisonparser.yy"
                                                                                                  {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor)) +","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[7].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 2010 "GenesysParser.cpp"
    break;

  case 104: // assigment: ATRIB "=" expression
#line 746 "parserBisonFlex/bisonparser.yy"
                                                { 
					// @TODO: getCurrentEvent()->getEntity() may be nullptr if simulation hasn't started yet
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[2].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2019 "GenesysParser.cpp"
    break;

  case 105: // assigment: ATRIB "[" indexList "]" "=" expression
#line 750 "parserBisonFlex/bisonparser.yy"
                                                                                         {
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[5].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor, yystack_[3].value.as < std::string > ());
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2027 "GenesysParser.cpp"
    break;

  case 106: // assigment: VARI "=" expression
#line 755 "parserBisonFlex/bisonparser.yy"
                                                                {
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[2].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; 
					}
#line 2036 "GenesysParser.cpp"
    break;

  case 107: // assigment: VARI "[" indexList "]" "=" expression
#line 759 "parserBisonFlex/bisonparser.yy"
                                                                                        {
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[5].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor, yystack_[3].value.as < std::string > ());
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2044 "GenesysParser.cpp"
    break;

  case 108: // pluginFunction: CTEZERO
#line 769 "parserBisonFlex/bisonparser.yy"
                                                     { yylhs.value.as < obj_t > ().valor = 0; }
#line 2050 "GenesysParser.cpp"
    break;

  case 109: // pluginFunction: fNQ "(" QUEUE ")"
#line 772 "parserBisonFlex/bisonparser.yy"
                                    {   //std::cout << "Queue ID: " << $3.id << ", Size: " << ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->size() << std::endl; 
                                        yylhs.value.as < obj_t > ().valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->size();}
#line 2057 "GenesysParser.cpp"
    break;

  case 110: // pluginFunction: fLASTINQ "(" QUEUE ")"
#line 774 "parserBisonFlex/bisonparser.yy"
                                    {/*For now does nothing because need acces to list of QUEUE, or at least the last element*/ }
#line 2063 "GenesysParser.cpp"
    break;

  case 111: // pluginFunction: fFIRSTINQ "(" QUEUE ")"
#line 775 "parserBisonFlex/bisonparser.yy"
                                    { 
                if (((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->size() > 0){
                    //id da 1a entidade da fila, talvez pegar nome
                    yylhs.value.as < obj_t > ().valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->first()->getEntity()->getId();
                }else{
                    yylhs.value.as < obj_t > ().valor = 0;
                } }
#line 2075 "GenesysParser.cpp"
    break;

  case 112: // pluginFunction: fSAQUE "(" QUEUE "," ATRIB ")"
#line 782 "parserBisonFlex/bisonparser.yy"
                                       {   
                //Util::identification queueID = $3.id;
                Util::identification attrID = yystack_[1].value.as < obj_t > ().id;
                double sum = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[3].value.as < obj_t > ().id)))->sumAttributesFromWaiting(attrID);
                yylhs.value.as < obj_t > ().valor = sum; }
#line 2085 "GenesysParser.cpp"
    break;

  case 113: // pluginFunction: fAQUE "(" QUEUE "," NUMD "," ATRIB ")"
#line 787 "parserBisonFlex/bisonparser.yy"
                                             {
                //Util::identification queueID = $3.id;
                Util::identification attrID = yystack_[1].value.as < obj_t > ().id;
                double value = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[5].value.as < obj_t > ().id)))->getAttributeFromWaitingRank(yystack_[3].value.as < obj_t > ().valor-1, attrID); // rank starts on 0 in genesys
                yylhs.value.as < obj_t > ().valor = value; }
#line 2095 "GenesysParser.cpp"
    break;

  case 114: // pluginFunction: fMR "(" RESOURCE ")"
#line 795 "parserBisonFlex/bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getCapacity();}
#line 2101 "GenesysParser.cpp"
    break;

  case 115: // pluginFunction: fNR "(" RESOURCE ")"
#line 796 "parserBisonFlex/bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getNumberBusy();}
#line 2107 "GenesysParser.cpp"
    break;

  case 116: // pluginFunction: fRESSEIZES "(" RESOURCE ")"
#line 797 "parserBisonFlex/bisonparser.yy"
                                         { /*\TODO: For now does nothing because needs get Seizes, check with teacher*/}
#line 2113 "GenesysParser.cpp"
    break;

  case 117: // pluginFunction: fSTATE "(" RESOURCE ")"
#line 798 "parserBisonFlex/bisonparser.yy"
                                         {  yylhs.value.as < obj_t > ().valor = static_cast<int>(((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getResourceState()); }
#line 2119 "GenesysParser.cpp"
    break;

  case 118: // pluginFunction: fIRF "(" RESOURCE ")"
#line 799 "parserBisonFlex/bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getResourceState() == Resource::ResourceState::FAILED ? 1 : 0; }
#line 2125 "GenesysParser.cpp"
    break;

  case 119: // pluginFunction: fSETSUM "(" SET ")"
#line 800 "parserBisonFlex/bisonparser.yy"
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
#line 2143 "GenesysParser.cpp"
    break;

  case 120: // pluginFunction: fNUMSET "(" SET ")"
#line 816 "parserBisonFlex/bisonparser.yy"
                                { yylhs.value.as < obj_t > ().valor = ((Set*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(),yystack_[1].value.as < obj_t > ().id))->getElementSet()->size(); }
#line 2149 "GenesysParser.cpp"
    break;


#line 2153 "GenesysParser.cpp"

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


  const short genesyspp_parser::yypact_ninf_ = -258;

  const signed char genesyspp_parser::yytable_ninf_ = -1;

  const short
  genesyspp_parser::yypact_[] =
  {
     315,  -258,  -258,  -258,   135,   -39,   -36,   -26,   -14,   -13,
     -12,   -11,   -10,    -9,    -8,    -7,    -6,  -258,    -5,    -4,
      -1,     1,     2,     3,     4,     5,     8,     9,  -258,  -258,
    -258,  -258,  -258,  -258,    10,   -45,   -70,  -258,  -258,    11,
      12,  -258,    36,    37,    38,    40,    42,    43,    44,    45,
      46,    47,    48,    49,   -69,    39,   315,   399,   399,    50,
      78,  -258,    25,    66,    41,  -258,    -3,   -74,   -35,  -258,
      54,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,
    -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,
    -258,    51,    52,  -258,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,    53,    55,   315,
     315,    28,    79,    72,    86,    88,   102,   103,    89,   114,
     115,   118,   120,   121,   119,   315,   315,   315,   113,  -258,
    -258,   315,  -258,   135,   135,   135,   135,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   315,   315,
     127,   128,   132,   122,   136,   137,   140,   141,   142,   143,
     124,   133,   145,   138,   139,   144,   154,   157,   158,   159,
     161,   162,   -73,   163,   315,   315,  -258,   -67,  -258,   148,
     151,   152,   175,   176,   177,   178,   179,   180,   181,   182,
     172,   173,   185,   -66,  -258,   -65,  -258,   186,    66,    41,
    -258,  -258,   -74,   -74,   -74,   -74,   -74,   -74,   -35,   -35,
    -258,  -258,  -258,   -55,   -54,  -258,  -258,  -258,   315,  -258,
    -258,  -258,  -258,  -258,  -258,   315,   315,  -258,   315,   315,
     315,   315,   315,   315,   315,   315,   315,  -258,   315,   315,
     219,   221,   183,   315,  -258,  -258,  -258,  -258,  -258,  -258,
    -258,  -258,  -258,  -258,  -258,   220,   270,  -258,   184,  -258,
     315,  -258,  -258,  -258,   192,   195,   196,   197,   198,   199,
     200,   201,   202,   193,   194,  -258,   203,   -72,   315,   315,
     315,  -258,   209,   204,   315,   -53,  -258,  -258,  -258,  -258,
    -258,  -258,  -258,  -258,  -258,   315,   315,   315,  -258,   315,
     234,   241,  -258,  -258,   243,  -258,  -258,   315,   211,   205,
    -258,   217,   -44,   -44,   218,   224,  -258,   315,  -258,   -49,
     -48,  -258,  -258,  -258,  -258,   222,   315,   315,  -258,   -47,
     -43,   183,   184
  };

  const signed char
  genesyspp_parser::yydefact_[] =
  {
       0,    42,    43,   108,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    57,    58,
      59,    60,    61,    62,     0,     0,    94,    96,    97,     0,
       0,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    98,   100,     0,     0,     0,     0,
       0,     2,     5,     8,    10,    13,    15,    22,    25,    28,
      30,    33,    34,     4,    44,    45,    35,    53,    54,    51,
      50,    52,    56,     6,    37,    38,    39,    40,    41,     3,
      55,    94,    98,    14,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    32,
      31,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    91,     0,   104,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   106,     0,    36,     0,     7,     9,
      11,    12,    18,    19,    20,    21,    16,    17,    23,    24,
      26,    27,    29,     0,     0,    65,    66,    67,     0,    69,
      68,    70,    71,    72,    73,     0,     0,    78,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,    95,     0,    63,    64,   115,   114,   118,   116,
     117,   119,   109,   111,   110,     0,     0,   120,    99,   101,
       0,    88,    95,    99,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    74,    75,    76,    79,
      80,    81,    82,    83,    84,     0,     0,     0,    47,     0,
       0,     0,   105,   112,     0,   107,   102,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    85,     0,    46,     0,
       0,    49,    48,   113,   103,     0,     0,     0,    86,     0,
       0,     0,     0
  };

  const short
  genesyspp_parser::yypgoto_[] =
  {
    -258,  -258,     0,  -258,   160,   166,    -2,  -258,   -90,  -138,
    -108,     6,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,
    -258,  -258,  -258,  -258,  -258,  -134,  -258,   266,  -258,  -258,
     267,  -258,  -257,  -258
  };

  const unsigned char
  genesyspp_parser::yydefgoto_[] =
  {
       0,    60,   186,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,   182,   187,    83,    84,    85,    86,
      87,    88,    89,    90
  };

  const short
  genesyspp_parser::yytable_[] =
  {
      61,   203,    93,   147,   148,   149,   150,    91,   329,   247,
     308,   153,   154,   119,   135,   218,   219,   252,   268,   269,
     248,   309,   120,   136,   223,   224,   253,   253,   270,   272,
     273,   316,    92,   330,   336,   337,   143,   341,   253,   253,
     317,   342,    94,   120,   136,    95,   253,   220,   221,   222,
     253,   145,   155,   146,   156,    96,   138,   212,   213,   214,
     215,   216,   217,   139,   140,   331,   332,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   142,   144,
     108,   189,   109,   110,   111,   112,   113,   151,   152,   114,
     115,   116,   121,   122,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   183,   123,   124,   125,
     188,   126,   137,   127,   128,   129,   130,   131,   132,   133,
     134,   141,   191,   190,   158,   159,   204,   205,     1,     2,
       3,   207,   157,   210,   211,   184,   192,   185,   193,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,   194,   195,   196,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,   197,   198,   250,   251,   199,    91,   200,   201,
      37,    38,    39,    40,   202,   206,    42,    43,    44,    45,
      46,    47,   339,   340,    48,    49,    50,    51,    52,   225,
     226,    53,    92,    55,   227,   228,    56,   235,   229,   230,
      57,    58,   231,   232,   233,   234,   236,   237,   274,    59,
     254,   238,   239,   255,   256,   275,   276,   240,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   241,   286,   287,
     242,   243,   244,   291,   245,   246,   249,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   271,   288,
     295,   289,   292,   293,   296,   290,   294,   297,   298,   299,
     300,   301,   302,   303,   304,   322,   305,   306,   310,   311,
     312,   313,   323,   326,   315,   324,   307,   314,   327,   328,
     333,   117,   118,   208,   338,   318,   319,   320,   334,   321,
     209,     0,     0,     0,     0,     0,     0,   325,     1,     2,
       3,     0,     0,     0,     0,     0,     0,   335,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,     0,    35,     0,     0,    36,     0,     0,
      37,    38,    39,    40,    41,     0,    42,    43,    44,    45,
      46,    47,     0,     0,    48,    49,    50,    51,    52,     0,
       0,    53,    54,    55,     0,     0,    56,     0,     0,     0,
      57,    58,     1,     2,     3,     0,     0,     0,     0,    59,
       0,     0,     0,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,     0,     0,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,     0,     0,     0,     0,
       0,    91,     0,     0,    37,    38,    39,    40,     0,     0,
      42,    43,    44,    45,    46,    47,     0,     0,    48,    49,
      50,    51,    52,     0,     0,    53,    92,    55,     0,     0,
      56,     0,     0,     0,    57,    58,     0,     0,     0,     0,
       0,     0,     0,    59
  };

  const short
  genesyspp_parser::yycheck_[] =
  {
       0,   135,     4,     6,     7,     8,     9,    52,    52,    82,
      82,    85,    86,    83,    83,   153,   154,    84,    84,    84,
      93,    93,    92,    92,   158,   159,    93,    93,    93,    84,
      84,    84,    77,    77,    83,    83,    11,    84,    93,    93,
      93,    84,    81,    92,    92,    81,    93,   155,   156,   157,
      93,    10,    87,    12,    89,    81,    56,   147,   148,   149,
     150,   151,   152,    57,    58,   322,   323,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,     0,    13,
      81,    53,    81,    81,    81,    81,    81,    90,    91,    81,
      81,    81,    81,    81,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    81,    81,    81,
     120,    81,    83,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    60,    54,    83,    83,   136,   137,     3,     4,
       5,   141,    88,   145,   146,    92,    60,    92,    60,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    60,    60,    75,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    68,    68,   184,   185,    68,    52,    68,    68,
      55,    56,    57,    58,    75,    82,    61,    62,    63,    64,
      65,    66,   336,   337,    69,    70,    71,    72,    73,    82,
      82,    76,    77,    78,    82,    93,    81,    93,    82,    82,
      85,    86,    82,    82,    82,    82,    93,    82,   228,    94,
      82,    93,    93,    82,    82,   235,   236,    93,   238,   239,
     240,   241,   242,   243,   244,   245,   246,    93,   248,   249,
      93,    93,    93,   253,    93,    93,    93,    82,    82,    82,
      82,    82,    82,    82,    82,    93,    93,    82,    82,    50,
     270,    50,    52,     3,    82,    92,    92,    82,    82,    82,
      82,    82,    82,    82,    82,    51,    93,    93,   288,   289,
     290,    82,    51,    82,   294,    52,    93,    93,    93,    82,
      82,    35,    35,   143,    82,   305,   306,   307,    84,   309,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   317,     3,     4,
       5,    -1,    -1,    -1,    -1,    -1,    -1,   327,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    -1,    -1,
      55,    56,    57,    58,    59,    -1,    61,    62,    63,    64,
      65,    66,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    76,    77,    78,    -1,    -1,    81,    -1,    -1,    -1,
      85,    86,     3,     4,     5,    -1,    -1,    -1,    -1,    94,
      -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    55,    56,    57,    58,    -1,    -1,
      61,    62,    63,    64,    65,    66,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    76,    77,    78,    -1,    -1,
      81,    -1,    -1,    -1,    85,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94
  };

  const unsigned char
  genesyspp_parser::yystos_[] =
  {
       0,     3,     4,     5,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    49,    52,    55,    56,    57,
      58,    59,    61,    62,    63,    64,    65,    66,    69,    70,
      71,    72,    73,    76,    77,    78,    81,    85,    86,    94,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   121,   122,   123,   124,   125,   126,   127,
     128,    52,    77,   101,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,   122,   125,    83,
      92,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    83,    92,    83,    97,   106,
     106,    81,     0,    11,    13,    10,    12,     6,     7,     8,
       9,    90,    91,    85,    86,    87,    89,    88,    83,    83,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,   119,    97,    92,    92,    97,   120,    97,    53,
      54,    60,    60,    60,    60,    60,    75,    68,    68,    68,
      68,    68,    75,   120,    97,    97,    82,    97,    99,   100,
     101,   101,   103,   103,   103,   103,   103,   103,   104,   104,
     105,   105,   105,   120,   120,    82,    82,    82,    93,    82,
      82,    82,    82,    82,    82,    93,    93,    82,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    82,    93,    93,
      97,    97,    84,    93,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    93,    93,    82,    84,    84,
      93,    82,    84,    84,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    50,    50,
      92,    97,    52,     3,    92,    97,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    93,    93,    93,    82,    93,
      97,    97,    97,    82,    93,    97,    84,    93,    97,    97,
      97,    97,    51,    51,    52,    97,    82,    93,    82,    52,
      77,   127,   127,    82,    84,    97,    83,    83,    82,   120,
     120,    84,    84
  };

  const unsigned char
  genesyspp_parser::yyr1_[] =
  {
       0,    95,    96,    97,    97,    97,    97,    98,    98,    99,
      99,   100,   100,   100,   101,   101,   102,   102,   102,   102,
     102,   102,   102,   103,   103,   103,   104,   104,   104,   105,
     105,   106,   106,   106,   107,   107,   107,   107,   107,   107,
     107,   107,   108,   108,   109,   109,   110,   110,   111,   111,
     112,   112,   112,   112,   112,   112,   112,   113,   113,   113,
     113,   113,   113,   114,   114,   115,   115,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   118,   119,
     119,   120,   120,   121,   122,   122,   123,   124,   125,   125,
     126,   126,   126,   126,   127,   127,   127,   127,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128
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
       1,     1,     1,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     6,     6,     6,     1,     4,     6,
       6,     6,     6,     6,     6,     8,    10,     4,     4,     5,
       3,     1,     3,     1,     1,     4,     1,     1,     1,     4,
       1,     4,     6,     8,     3,     6,     3,     6,     1,     4,
       4,     4,     6,     8,     4,     4,     4,     4,     4,     4,
       4
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
  "ILLEGAL", "RESOURCE", "fNR", "fMR", "fIRF", "fRESSEIZES", "fSTATE",
  "fSETSUM", "fRESUTIL", "QUEUE", "fNQ", "fFIRSTINQ", "fLASTINQ", "fSAQUE",
  "fAQUE", "fENTATRANK", "SET", "fNUMSET", "VARI", "FORM", "fNUMGR",
  "fATRGR", "\"(\"", "\")\"", "\"[\"", "\"]\"", "\"+\"", "\"-\"", "\"*\"",
  "\"^\"", "\"/\"", "\"<\"", "\">\"", "\"=\"", "\",\"", "\"USER\"",
  "$accept", "input", "expression", "logicalOr", "logicalXor",
  "logicalAnd", "logicalNot", "relational", "additive", "multiplicative",
  "power", "unary", "primary", "number", "command", "commandIF",
  "commandFOR", "function", "kernelFunction", "elementFunction",
  "trigonFunction", "mathFunction", "probFunction", "userFunction",
  "listaparm", "indexList", "illegal", "attribute", "simulationResponse",
  "simulationControl", "variable", "formula", "assigment",
  "pluginFunction", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  genesyspp_parser::yyrline_[] =
  {
       0,   276,   276,   281,   282,   283,   284,   288,   289,   293,
     294,   298,   299,   300,   304,   305,   309,   310,   311,   312,
     313,   314,   315,   319,   320,   321,   325,   326,   327,   331,
     332,   336,   337,   338,   342,   343,   344,   345,   346,   347,
     352,   356,   363,   364,   368,   369,   373,   374,   379,   380,
     384,   385,   386,   387,   388,   389,   390,   394,   395,   396,
     397,   398,   399,   404,   408,   415,   416,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   433,   452,   471,
     490,   509,   528,   547,   566,   585,   604,   623,   629,   634,
     635,   639,   640,   645,   670,   680,   691,   697,   705,   706,
     713,   720,   727,   734,   746,   750,   755,   759,   769,   772,
     774,   775,   782,   787,   795,   796,   797,   798,   799,   800,
     816
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
#line 2873 "GenesysParser.cpp"

#line 823 "parserBisonFlex/bisonparser.yy"

void
yy::genesyspp_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
