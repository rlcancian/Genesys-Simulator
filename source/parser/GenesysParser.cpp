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
#line 62 "bisonparser.yy"

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


#line 65 "../GenesysParser.cpp"


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
#line 157 "../GenesysParser.cpp"

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
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        value.YY_MOVE_OR_COPY< obj_t > (YY_MOVE (that.value));
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
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        value.move< obj_t > (YY_MOVE (that.value));
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
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        value.copy< obj_t > (that.value);
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
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        value.move< obj_t > (that.value);
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
#line 55 "bisonparser.yy"
{
  // Initialize the initial location.
  //@$.begin.filename = @$.end.filename = &driver.getFile();
}

#line 859 "../GenesysParser.cpp"


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
      case symbol_kind::S_variable: // variable
      case symbol_kind::S_formula: // formula
      case symbol_kind::S_assigment: // assigment
      case symbol_kind::S_pluginFunction: // pluginFunction
        yylhs.value.emplace< obj_t > ();
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
#line 263 "bisonparser.yy"
                    { driver.setResult(yystack_[0].value.as < obj_t > ().valor);}
#line 1106 "../GenesysParser.cpp"
    break;

  case 3: // expression: assigment
#line 268 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1112 "../GenesysParser.cpp"
    break;

  case 4: // expression: command
#line 269 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1118 "../GenesysParser.cpp"
    break;

  case 5: // expression: logicalOr
#line 270 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1124 "../GenesysParser.cpp"
    break;

  case 6: // expression: illegal
#line 271 "bisonparser.yy"
                                        {yylhs.value.as < obj_t > ().valor = -1;}
#line 1130 "../GenesysParser.cpp"
    break;

  case 7: // logicalOr: logicalOr oOR logicalXor
#line 275 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (int)yystack_[2].value.as < obj_t > ().valor || (int)yystack_[0].value.as < obj_t > ().valor; }
#line 1136 "../GenesysParser.cpp"
    break;

  case 8: // logicalOr: logicalXor
#line 276 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1142 "../GenesysParser.cpp"
    break;

  case 9: // logicalXor: logicalXor oXOR logicalAnd
#line 280 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (!(int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor) || ((int)yystack_[2].value.as < obj_t > ().valor && !(int)yystack_[0].value.as < obj_t > ().valor); }
#line 1148 "../GenesysParser.cpp"
    break;

  case 10: // logicalXor: logicalAnd
#line 281 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1154 "../GenesysParser.cpp"
    break;

  case 11: // logicalAnd: logicalAnd oAND logicalNot
#line 285 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = (int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor; }
#line 1160 "../GenesysParser.cpp"
    break;

  case 12: // logicalAnd: logicalAnd oNAND logicalNot
#line 286 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = !((int)yystack_[2].value.as < obj_t > ().valor && (int)yystack_[0].value.as < obj_t > ().valor); }
#line 1166 "../GenesysParser.cpp"
    break;

  case 13: // logicalAnd: logicalNot
#line 287 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1172 "../GenesysParser.cpp"
    break;

  case 14: // logicalNot: oNOT logicalNot
#line 291 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = !(int)yystack_[0].value.as < obj_t > ().valor; }
#line 1178 "../GenesysParser.cpp"
    break;

  case 15: // logicalNot: relational
#line 292 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1184 "../GenesysParser.cpp"
    break;

  case 16: // relational: relational "<" additive
#line 296 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor < yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1190 "../GenesysParser.cpp"
    break;

  case 17: // relational: relational ">" additive
#line 297 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor > yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1196 "../GenesysParser.cpp"
    break;

  case 18: // relational: relational oLE additive
#line 298 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor <= yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1202 "../GenesysParser.cpp"
    break;

  case 19: // relational: relational oGE additive
#line 299 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor >= yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1208 "../GenesysParser.cpp"
    break;

  case 20: // relational: relational oEQ additive
#line 300 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor == yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1214 "../GenesysParser.cpp"
    break;

  case 21: // relational: relational oNE additive
#line 301 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor != yystack_[0].value.as < obj_t > ().valor ? 1 : 0; }
#line 1220 "../GenesysParser.cpp"
    break;

  case 22: // relational: additive
#line 302 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1226 "../GenesysParser.cpp"
    break;

  case 23: // additive: additive "+" multiplicative
#line 306 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor + yystack_[0].value.as < obj_t > ().valor; }
#line 1232 "../GenesysParser.cpp"
    break;

  case 24: // additive: additive "-" multiplicative
#line 307 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor - yystack_[0].value.as < obj_t > ().valor; }
#line 1238 "../GenesysParser.cpp"
    break;

  case 25: // additive: multiplicative
#line 308 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1244 "../GenesysParser.cpp"
    break;

  case 26: // multiplicative: multiplicative "*" power
#line 312 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor * yystack_[0].value.as < obj_t > ().valor; }
#line 1250 "../GenesysParser.cpp"
    break;

  case 27: // multiplicative: multiplicative "/" power
#line 313 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor / yystack_[0].value.as < obj_t > ().valor; }
#line 1256 "../GenesysParser.cpp"
    break;

  case 28: // multiplicative: power
#line 314 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1262 "../GenesysParser.cpp"
    break;

  case 29: // power: unary "^" power
#line 318 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = pow(yystack_[2].value.as < obj_t > ().valor, yystack_[0].value.as < obj_t > ().valor); }
#line 1268 "../GenesysParser.cpp"
    break;

  case 30: // power: unary
#line 319 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1274 "../GenesysParser.cpp"
    break;

  case 31: // unary: "-" unary
#line 323 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = -yystack_[0].value.as < obj_t > ().valor; }
#line 1280 "../GenesysParser.cpp"
    break;

  case 32: // unary: "+" unary
#line 324 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = +yystack_[0].value.as < obj_t > ().valor; }
#line 1286 "../GenesysParser.cpp"
    break;

  case 33: // unary: primary
#line 325 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1292 "../GenesysParser.cpp"
    break;

  case 34: // primary: number
#line 329 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1298 "../GenesysParser.cpp"
    break;

  case 35: // primary: function
#line 330 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1304 "../GenesysParser.cpp"
    break;

  case 36: // primary: "(" expression ")"
#line 331 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor;}
#line 1310 "../GenesysParser.cpp"
    break;

  case 37: // primary: attribute
#line 332 "bisonparser.yy"
                                         {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1316 "../GenesysParser.cpp"
    break;

  case 38: // primary: variable
#line 337 "bisonparser.yy"
                                              {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1322 "../GenesysParser.cpp"
    break;

  case 39: // primary: formula
#line 341 "bisonparser.yy"
                                              {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1328 "../GenesysParser.cpp"
    break;

  case 40: // number: NUMD
#line 348 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1334 "../GenesysParser.cpp"
    break;

  case 41: // number: NUMH
#line 349 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1340 "../GenesysParser.cpp"
    break;

  case 42: // command: commandIF
#line 353 "bisonparser.yy"
                    { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1346 "../GenesysParser.cpp"
    break;

  case 43: // command: commandFOR
#line 354 "bisonparser.yy"
                    { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1352 "../GenesysParser.cpp"
    break;

  case 44: // commandIF: cIF "(" expression "," expression "," expression ")"
#line 358 "bisonparser.yy"
                                                           { yylhs.value.as < obj_t > ().valor = yystack_[5].value.as < obj_t > ().valor != 0 ? yystack_[3].value.as < obj_t > ().valor : yystack_[1].value.as < obj_t > ().valor; }
#line 1358 "../GenesysParser.cpp"
    break;

  case 45: // commandIF: cIF "(" expression "," expression ")"
#line 359 "bisonparser.yy"
                                                            { yylhs.value.as < obj_t > ().valor = yystack_[3].value.as < obj_t > ().valor != 0 ? yystack_[1].value.as < obj_t > ().valor : 0; }
#line 1364 "../GenesysParser.cpp"
    break;

  case 46: // commandFOR: cFOR variable "=" expression cTO expression cDO assigment
#line 364 "bisonparser.yy"
                                                                {yylhs.value.as < obj_t > ().valor = 0; }
#line 1370 "../GenesysParser.cpp"
    break;

  case 47: // commandFOR: cFOR attribute "=" expression cTO expression cDO assigment
#line 365 "bisonparser.yy"
                                                                  {yylhs.value.as < obj_t > ().valor = 0; }
#line 1376 "../GenesysParser.cpp"
    break;

  case 48: // function: mathFunction
#line 369 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1382 "../GenesysParser.cpp"
    break;

  case 49: // function: trigonFunction
#line 370 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1388 "../GenesysParser.cpp"
    break;

  case 50: // function: probFunction
#line 371 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1394 "../GenesysParser.cpp"
    break;

  case 51: // function: kernelFunction
#line 372 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1400 "../GenesysParser.cpp"
    break;

  case 52: // function: elementFunction
#line 373 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1406 "../GenesysParser.cpp"
    break;

  case 53: // function: pluginFunction
#line 374 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1412 "../GenesysParser.cpp"
    break;

  case 54: // function: userFunction
#line 375 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1418 "../GenesysParser.cpp"
    break;

  case 55: // kernelFunction: fTNOW
#line 379 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getSimulatedTime();}
#line 1424 "../GenesysParser.cpp"
    break;

  case 56: // kernelFunction: fTFIN
#line 380 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getReplicationLength();}
#line 1430 "../GenesysParser.cpp"
    break;

  case 57: // kernelFunction: fMAXREP
#line 381 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getNumberOfReplications();}
#line 1436 "../GenesysParser.cpp"
    break;

  case 58: // kernelFunction: fNUMREP
#line 382 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getCurrentReplicationNumber();}
#line 1442 "../GenesysParser.cpp"
    break;

  case 59: // kernelFunction: fIDENT
#line 383 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getId();}
#line 1448 "../GenesysParser.cpp"
    break;

  case 60: // kernelFunction: simulEntitiesWIP
#line 384 "bisonparser.yy"
                            { yylhs.value.as < obj_t > ().valor = driver.getModel()->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Entity>());}
#line 1454 "../GenesysParser.cpp"
    break;

  case 61: // elementFunction: fTAVG "(" CSTAT ")"
#line 389 "bisonparser.yy"
                               {
                    StatisticsCollector* cstat = ((StatisticsCollector*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<StatisticsCollector>(), yystack_[1].value.as < obj_t > ().id)));
                    double value = cstat->getStatistics()->average();
                    yylhs.value.as < obj_t > ().valor = value; }
#line 1463 "../GenesysParser.cpp"
    break;

  case 62: // elementFunction: fCOUNT "(" COUNTER ")"
#line 393 "bisonparser.yy"
                                 {
					Counter* counter = ((Counter*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Counter>(), yystack_[1].value.as < obj_t > ().id)));
                    double value = counter->getCountValue();
                    yylhs.value.as < obj_t > ().valor = value; }
#line 1472 "../GenesysParser.cpp"
    break;

  case 63: // trigonFunction: fSIN "(" expression ")"
#line 400 "bisonparser.yy"
                                  { yylhs.value.as < obj_t > ().valor = sin(yystack_[1].value.as < obj_t > ().valor); }
#line 1478 "../GenesysParser.cpp"
    break;

  case 64: // trigonFunction: fCOS "(" expression ")"
#line 401 "bisonparser.yy"
                                  { yylhs.value.as < obj_t > ().valor = cos(yystack_[1].value.as < obj_t > ().valor); }
#line 1484 "../GenesysParser.cpp"
    break;

  case 65: // mathFunction: fROUND "(" expression ")"
#line 405 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = round(yystack_[1].value.as < obj_t > ().valor);}
#line 1490 "../GenesysParser.cpp"
    break;

  case 66: // mathFunction: fFRAC "(" expression ")"
#line 406 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor - (int) yystack_[1].value.as < obj_t > ().valor;}
#line 1496 "../GenesysParser.cpp"
    break;

  case 67: // mathFunction: fTRUNC "(" expression ")"
#line 407 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = trunc(yystack_[1].value.as < obj_t > ().valor);}
#line 1502 "../GenesysParser.cpp"
    break;

  case 68: // mathFunction: fEXP "(" expression ")"
#line 408 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = exp(yystack_[1].value.as < obj_t > ().valor);}
#line 1508 "../GenesysParser.cpp"
    break;

  case 69: // mathFunction: fSQRT "(" expression ")"
#line 409 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = sqrt(yystack_[1].value.as < obj_t > ().valor);}
#line 1514 "../GenesysParser.cpp"
    break;

  case 70: // mathFunction: fLOG "(" expression ")"
#line 410 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = log10(yystack_[1].value.as < obj_t > ().valor);}
#line 1520 "../GenesysParser.cpp"
    break;

  case 71: // mathFunction: fLN "(" expression ")"
#line 411 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = log(yystack_[1].value.as < obj_t > ().valor);}
#line 1526 "../GenesysParser.cpp"
    break;

  case 72: // mathFunction: fMOD "(" expression "," expression ")"
#line 412 "bisonparser.yy"
                                               { yylhs.value.as < obj_t > ().valor = (int) yystack_[3].value.as < obj_t > ().valor % (int) yystack_[1].value.as < obj_t > ().valor; }
#line 1532 "../GenesysParser.cpp"
    break;

  case 73: // mathFunction: mathMIN "(" expression "," expression ")"
#line 413 "bisonparser.yy"
                                                { yylhs.value.as < obj_t > ().valor = std::min(yystack_[3].value.as < obj_t > ().valor, yystack_[1].value.as < obj_t > ().valor); }
#line 1538 "../GenesysParser.cpp"
    break;

  case 74: // mathFunction: mathMAX "(" expression "," expression ")"
#line 414 "bisonparser.yy"
                                                { yylhs.value.as < obj_t > ().valor = std::max(yystack_[3].value.as < obj_t > ().valor, yystack_[1].value.as < obj_t > ().valor); }
#line 1544 "../GenesysParser.cpp"
    break;

  case 75: // probFunction: fRND1
#line 418 "bisonparser.yy"
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
#line 1568 "../GenesysParser.cpp"
    break;

  case 76: // probFunction: fEXPO "(" expression ")"
#line 437 "bisonparser.yy"
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
#line 1592 "../GenesysParser.cpp"
    break;

  case 77: // probFunction: fNORM "(" expression "," expression ")"
#line 456 "bisonparser.yy"
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
#line 1616 "../GenesysParser.cpp"
    break;

  case 78: // probFunction: fUNIF "(" expression "," expression ")"
#line 475 "bisonparser.yy"
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
#line 1640 "../GenesysParser.cpp"
    break;

  case 79: // probFunction: fWEIB "(" expression "," expression ")"
#line 494 "bisonparser.yy"
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
#line 1664 "../GenesysParser.cpp"
    break;

  case 80: // probFunction: fLOGN "(" expression "," expression ")"
#line 513 "bisonparser.yy"
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
#line 1688 "../GenesysParser.cpp"
    break;

  case 81: // probFunction: fGAMM "(" expression "," expression ")"
#line 532 "bisonparser.yy"
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
#line 1712 "../GenesysParser.cpp"
    break;

  case 82: // probFunction: fERLA "(" expression "," expression ")"
#line 551 "bisonparser.yy"
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
#line 1736 "../GenesysParser.cpp"
    break;

  case 83: // probFunction: fTRIA "(" expression "," expression "," expression ")"
#line 570 "bisonparser.yy"
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
#line 1760 "../GenesysParser.cpp"
    break;

  case 84: // probFunction: fBETA "(" expression "," expression "," expression "," expression ")"
#line 589 "bisonparser.yy"
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
#line 1784 "../GenesysParser.cpp"
    break;

  case 85: // probFunction: fDISC "(" listaparm ")"
#line 608 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleDiscrete(0,0); /*@TODO: NOT IMPLEMENTED YET*/ }
#line 1790 "../GenesysParser.cpp"
    break;

  case 86: // userFunction: "USER" "(" expression ")"
#line 614 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor; }
#line 1796 "../GenesysParser.cpp"
    break;

  case 87: // listaparm: listaparm "," expression "," expression
#line 619 "bisonparser.yy"
                                                 {/*@TODO: NOT IMPLEMENTED YET*/}
#line 1802 "../GenesysParser.cpp"
    break;

  case 88: // listaparm: expression "," expression
#line 620 "bisonparser.yy"
                                                 {/*@TODO: NOT IMPLEMENTED YET*/}
#line 1808 "../GenesysParser.cpp"
    break;

  case 89: // illegal: ILLEGAL
#line 625 "bisonparser.yy"
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
#line 1833 "../GenesysParser.cpp"
    break;

  case 90: // attribute: ATRIB
#line 650 "bisonparser.yy"
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
#line 1848 "../GenesysParser.cpp"
    break;

  case 91: // attribute: ATRIB "[" expression "]"
#line 660 "bisonparser.yy"
                                              {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[3].value.as < obj_t > ().id, index);
		}
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 1862 "../GenesysParser.cpp"
    break;

  case 92: // attribute: ATRIB "[" expression "," expression "]"
#line 669 "bisonparser.yy"
                                                             {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[5].value.as < obj_t > ().id, index);
		}
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 1876 "../GenesysParser.cpp"
    break;

  case 93: // attribute: ATRIB "[" expression "," expression "," expression "]"
#line 678 "bisonparser.yy"
                                                                            {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[7].value.as < obj_t > ().id, index);
		}
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 1890 "../GenesysParser.cpp"
    break;

  case 94: // variable: VARI
#line 692 "bisonparser.yy"
                            {yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[0].value.as < obj_t > ().id)))->getValue();}
#line 1896 "../GenesysParser.cpp"
    break;

  case 95: // variable: VARI "[" expression "]"
#line 693 "bisonparser.yy"
                                                                            { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[3].value.as < obj_t > ().id)))->getValue(index); }
#line 1904 "../GenesysParser.cpp"
    break;

  case 96: // variable: VARI "[" expression "," expression "]"
#line 696 "bisonparser.yy"
                                                                                            { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor)); 
					yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[5].value.as < obj_t > ().id)))->getValue(index);}
#line 1912 "../GenesysParser.cpp"
    break;

  case 97: // variable: VARI "[" expression "," expression "," expression "]"
#line 699 "bisonparser.yy"
                                                                                                     { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[7].value.as < obj_t > ().id)))->getValue(index);}
#line 1920 "../GenesysParser.cpp"
    break;

  case 98: // formula: FORM
#line 707 "bisonparser.yy"
                                    { 
					std::string index = "";
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[0].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1932 "../GenesysParser.cpp"
    break;

  case 99: // formula: FORM "[" expression "]"
#line 714 "bisonparser.yy"
                                                                    {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[3].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1944 "../GenesysParser.cpp"
    break;

  case 100: // formula: FORM "[" expression "," expression "]"
#line 721 "bisonparser.yy"
                                                                                   {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor)) +","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[5].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1956 "../GenesysParser.cpp"
    break;

  case 101: // formula: FORM "[" expression "," expression "," expression "]"
#line 728 "bisonparser.yy"
                                                                                                  {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor)) +","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[7].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1968 "../GenesysParser.cpp"
    break;

  case 102: // assigment: ATRIB "=" expression
#line 740 "bisonparser.yy"
                                                { 
					// @TODO: getCurrentEvent()->getEntity() may be nullptr if simulation hasn't started yet
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[2].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1977 "../GenesysParser.cpp"
    break;

  case 103: // assigment: ATRIB "[" expression "]" "=" expression
#line 744 "bisonparser.yy"
                                                                                          { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor));
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[5].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1986 "../GenesysParser.cpp"
    break;

  case 104: // assigment: ATRIB "[" expression "," expression "]" "=" expression
#line 748 "bisonparser.yy"
                                                                                                        {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor)); 
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[7].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1995 "../GenesysParser.cpp"
    break;

  case 105: // assigment: ATRIB "[" expression "," expression "," expression "]" "=" expression
#line 752 "bisonparser.yy"
                                                                                                                          {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[7].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor));
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[9].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2004 "../GenesysParser.cpp"
    break;

  case 106: // assigment: VARI "=" expression
#line 758 "bisonparser.yy"
                                                                {
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[2].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; 
					}
#line 2013 "../GenesysParser.cpp"
    break;

  case 107: // assigment: VARI "[" expression "]" "=" expression
#line 762 "bisonparser.yy"
                                                                                         { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor));
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[5].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2022 "../GenesysParser.cpp"
    break;

  case 108: // assigment: VARI "[" expression "," expression "]" "=" expression
#line 766 "bisonparser.yy"
                                                                                                       {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor)); 
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[7].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2031 "../GenesysParser.cpp"
    break;

  case 109: // assigment: VARI "[" expression "," expression "," expression "]" "=" expression
#line 770 "bisonparser.yy"
                                                                                                                         {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[7].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor));
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[9].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 2040 "../GenesysParser.cpp"
    break;

  case 110: // pluginFunction: CTEZERO
#line 781 "bisonparser.yy"
                                                     { yylhs.value.as < obj_t > ().valor = 0; }
#line 2046 "../GenesysParser.cpp"
    break;

  case 111: // pluginFunction: fNQ "(" QUEUE ")"
#line 784 "bisonparser.yy"
                                    {   //std::cout << "Queue ID: " << $3.id << ", Size: " << ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->size() << std::endl; 
                                        yylhs.value.as < obj_t > ().valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->size();}
#line 2053 "../GenesysParser.cpp"
    break;

  case 112: // pluginFunction: fLASTINQ "(" QUEUE ")"
#line 786 "bisonparser.yy"
                                    {/*For now does nothing because need acces to list of QUEUE, or at least the last element*/ }
#line 2059 "../GenesysParser.cpp"
    break;

  case 113: // pluginFunction: fFIRSTINQ "(" QUEUE ")"
#line 787 "bisonparser.yy"
                                    { 
                if (((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->size() > 0){
                    //id da 1a entidade da fila, talvez pegar nome
                    yylhs.value.as < obj_t > ().valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->first()->getEntity()->getId();
                }else{
                    yylhs.value.as < obj_t > ().valor = 0;
                } }
#line 2071 "../GenesysParser.cpp"
    break;

  case 114: // pluginFunction: fSAQUE "(" QUEUE "," ATRIB ")"
#line 794 "bisonparser.yy"
                                       {   
                //Util::identification queueID = $3.id;
                Util::identification attrID = yystack_[1].value.as < obj_t > ().id;
                double sum = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[3].value.as < obj_t > ().id)))->sumAttributesFromWaiting(attrID);
                yylhs.value.as < obj_t > ().valor = sum; }
#line 2081 "../GenesysParser.cpp"
    break;

  case 115: // pluginFunction: fAQUE "(" QUEUE "," NUMD "," ATRIB ")"
#line 799 "bisonparser.yy"
                                             {
                //Util::identification queueID = $3.id;
                Util::identification attrID = yystack_[1].value.as < obj_t > ().id;
                double value = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[5].value.as < obj_t > ().id)))->getAttributeFromWaitingRank(yystack_[3].value.as < obj_t > ().valor-1, attrID); // rank starts on 0 in genesys
                yylhs.value.as < obj_t > ().valor = value; }
#line 2091 "../GenesysParser.cpp"
    break;

  case 116: // pluginFunction: fMR "(" RESOURCE ")"
#line 807 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getCapacity();}
#line 2097 "../GenesysParser.cpp"
    break;

  case 117: // pluginFunction: fNR "(" RESOURCE ")"
#line 808 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getNumberBusy();}
#line 2103 "../GenesysParser.cpp"
    break;

  case 118: // pluginFunction: fRESSEIZES "(" RESOURCE ")"
#line 809 "bisonparser.yy"
                                         { /*\TODO: For now does nothing because needs get Seizes, check with teacher*/}
#line 2109 "../GenesysParser.cpp"
    break;

  case 119: // pluginFunction: fSTATE "(" RESOURCE ")"
#line 810 "bisonparser.yy"
                                         {  yylhs.value.as < obj_t > ().valor = static_cast<int>(((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getResourceState()); }
#line 2115 "../GenesysParser.cpp"
    break;

  case 120: // pluginFunction: fIRF "(" RESOURCE ")"
#line 811 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getResourceState() == Resource::ResourceState::FAILED ? 1 : 0; }
#line 2121 "../GenesysParser.cpp"
    break;

  case 121: // pluginFunction: fSETSUM "(" SET ")"
#line 812 "bisonparser.yy"
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
#line 2139 "../GenesysParser.cpp"
    break;

  case 122: // pluginFunction: fNUMSET "(" SET ")"
#line 828 "bisonparser.yy"
                                { yylhs.value.as < obj_t > ().valor = ((Set*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(),yystack_[1].value.as < obj_t > ().id))->getElementSet()->size(); }
#line 2145 "../GenesysParser.cpp"
    break;


#line 2149 "../GenesysParser.cpp"

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


  const short genesyspp_parser::yypact_ninf_ = -224;

  const signed char genesyspp_parser::yytable_ninf_ = -1;

  const short
  genesyspp_parser::yypact_[] =
  {
     313,  -224,  -224,  -224,   395,   -51,   -48,    -9,     9,    14,
      31,    32,    33,    34,    35,    36,    37,  -224,    38,    39,
      40,    41,    42,    44,    45,    46,    49,    50,  -224,  -224,
    -224,  -224,  -224,  -224,    51,   -46,   -69,    52,    53,  -224,
      54,    55,    56,    57,    58,    61,    62,    63,    64,    65,
      66,    67,   -68,    68,   313,   477,   477,    69,   122,  -224,
       3,   134,    77,  -224,    -4,   -73,     7,  -224,    70,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,    71,    72,  -224,
     313,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,    60,    73,   313,   313,    98,   100,    97,
      99,   101,   102,   103,    85,    96,   104,   105,   106,   107,
      91,   313,   313,   313,    86,  -224,  -224,   313,  -224,   395,
     395,   395,   395,   477,   477,   477,   477,   477,   477,   477,
     477,   477,   477,   477,   313,   313,    87,    88,    94,    74,
      95,   108,   111,   112,   113,   115,   109,   110,   116,   114,
     117,   118,   120,   123,   125,   127,   128,   129,   -72,   130,
     313,   313,   -67,  -224,   119,   124,   126,   133,   142,   143,
     144,   145,   146,   147,   148,   138,   139,   151,   -66,  -224,
     -65,  -224,   155,   134,    77,  -224,  -224,   -73,   -73,   -73,
     -73,   -73,   -73,     7,     7,  -224,  -224,  -224,   -64,   -59,
    -224,  -224,  -224,   313,  -224,  -224,  -224,  -224,  -224,  -224,
     313,   313,  -224,   313,   313,   313,   313,   313,   313,   313,
     313,   313,  -224,   313,   313,   152,   153,   149,   313,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
     184,   175,  -224,   150,   313,  -224,   313,  -224,  -224,   313,
    -224,   313,   157,   161,   162,   163,   164,   165,   166,   167,
     168,   158,   159,  -224,   160,   -71,   313,   313,   313,   -23,
     177,   169,   313,   -22,   -19,   -18,   -17,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,   313,   313,   313,  -224,
     313,   156,   204,  -224,   172,   313,  -224,   206,  -224,   173,
     313,  -224,   313,  -224,   313,  -224,   313,   179,   174,  -224,
     187,   -45,   -45,   313,   182,   189,   313,   191,   192,   193,
     194,  -224,   313,  -224,   -15,   -14,  -224,  -224,  -224,   181,
    -224,  -224,   188,  -224,  -224,  -224,   197,   313,   313,   313,
     313,  -224,   -11,    -1,  -224,  -224,   149,   313,   150,   313,
       0,     4,   172,   313,   173,   313,   198,   199,   181,   188
  };

  const signed char
  genesyspp_parser::yydefact_[] =
  {
       0,    40,    41,   110,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,    58,    59,    60,     0,     0,    90,     0,     0,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    94,    98,     0,     0,     0,     0,     0,     2,
       5,     8,    10,    13,    15,    22,    25,    28,    30,    33,
      34,     4,    42,    43,    35,    51,    52,    49,    48,    50,
      54,     6,    37,    38,    39,     3,    53,    90,    94,    14,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    32,    31,     0,     1,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   102,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   106,
       0,    36,     0,     7,     9,    11,    12,    18,    19,    20,
      21,    16,    17,    23,    24,    26,    27,    29,     0,     0,
      63,    64,    65,     0,    67,    66,    68,    69,    70,    71,
       0,     0,    76,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,     0,     0,     0,     0,    91,     0,    61,
      62,   117,   116,   120,   118,   119,   121,   111,   113,   112,
       0,     0,   122,    95,     0,    99,     0,    86,    91,     0,
      95,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    88,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    72,    73,    74,
      77,    78,    79,    80,    81,    82,     0,     0,     0,    45,
       0,     0,     0,   103,    92,     0,   114,     0,   107,    96,
       0,   100,     0,    92,     0,    96,     0,     0,     0,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,     0,    44,     0,     0,    47,    46,   104,    93,
     115,   108,    97,   101,    93,    97,     0,     0,     0,     0,
       0,    84,     0,     0,   105,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0
  };

  const short
  genesyspp_parser::yypgoto_[] =
  {
    -224,  -224,   -54,  -224,    59,   154,    -3,  -224,   -41,   -53,
    -118,    43,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,   248,   249,  -224,  -223,
    -224
  };

  const unsigned char
  genesyspp_parser::yydefgoto_[] =
  {
       0,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,   178,    81,    82,    83,    84,    85,
      86
  };

  const short
  genesyspp_parser::yytable_[] =
  {
     134,    89,   143,   144,   145,   146,    87,   344,   242,   309,
     149,   150,   115,   131,   139,   247,   263,   265,   268,   243,
     310,   116,   132,   270,   248,   264,   266,   269,    90,    88,
     345,    91,   271,   215,   216,   217,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   179,   314,
     319,   182,   183,   321,   323,   325,   357,   358,   315,   320,
      92,   366,   322,   324,   326,   116,   132,   198,   199,   200,
     367,   368,   372,   202,   147,   148,   374,   141,    93,   142,
     369,   373,   151,    94,   152,   375,   213,   214,   135,   136,
     218,   219,   207,   208,   209,   210,   211,   212,   346,   347,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   138,   107,   108,   109,   245,   246,   110,   111,
     112,   117,   118,   119,   120,   121,   122,   123,   205,   206,
     124,   125,   126,   127,   128,   129,   130,   140,   137,   133,
     180,   184,   154,   155,   185,   186,   153,   187,   191,   188,
     189,   190,   192,   181,   197,   223,   201,   220,   221,   272,
     193,   194,   195,   196,   222,   224,   273,   274,   291,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   225,   284,
     285,   226,   227,   228,   289,   229,   232,     0,   203,   249,
     230,   231,   286,   287,   250,   233,   251,   331,   234,   235,
     293,   236,   294,   252,   237,   295,   238,   296,   239,   240,
     241,   244,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   311,   312,   313,   267,   290,   297,   318,   288,
     292,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   327,   328,   329,   332,   330,   316,   335,   341,
     317,   334,   333,   336,   349,   342,   337,   343,   338,   350,
     339,   359,   340,   352,   353,   354,   355,   361,   360,   348,
     378,   379,   351,   113,   114,     0,     0,     0,   356,     0,
       0,     0,     0,     0,   204,     0,     0,     0,     0,     0,
       0,     0,     0,   362,   363,   364,   365,     0,     0,     0,
       0,     0,     0,   370,     0,   371,     1,     2,     3,   376,
       0,   377,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
       0,     0,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,     0,    35,     0,     0,    36,     0,     0,    37,    38,
      39,     0,    40,    41,    42,    43,    44,    45,     0,     0,
      46,    47,    48,    49,    50,     0,     0,    51,    52,    53,
       0,     0,    54,     0,     0,     0,    55,    56,     1,     2,
       3,     0,     0,     0,     0,    57,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,     0,     0,     0,     0,     0,    87,     0,     0,
      37,    38,     0,     0,    40,    41,    42,    43,    44,    45,
       0,     0,    46,    47,    48,    49,    50,     0,     0,    51,
      88,    53,     0,     0,    54,     0,     0,     0,    55,    56,
       1,     2,     3,     0,     0,     0,     0,    57,     0,     0,
       0,     0,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     0,     0,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,     0,     0,     0,     0,     0,    87,
       0,     0,    37,    38,     0,     0,    40,    41,    42,    43,
      44,    45,     0,     0,    46,    47,    48,    49,    50,     0,
       0,    51,    88,    53,     0,     0,    54,     0,     0,     0,
      55,    56,     0,     0,     0,     0,     0,     0,     0,    57
  };

  const short
  genesyspp_parser::yycheck_[] =
  {
      54,     4,     6,     7,     8,     9,    52,    52,    80,    80,
      83,    84,    81,    81,    11,    82,    82,    82,    82,    91,
      91,    90,    90,    82,    91,    91,    91,    91,    79,    75,
      75,    79,    91,   151,   152,   153,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    82,
      82,   115,   116,    82,    82,    82,    81,    81,    91,    91,
      79,    82,    91,    91,    91,    90,    90,   131,   132,   133,
      91,    82,    82,   137,    88,    89,    82,    10,    79,    12,
      91,    91,    85,    79,    87,    91,   149,   150,    55,    56,
     154,   155,   143,   144,   145,   146,   147,   148,   331,   332,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,     0,    79,    79,    79,   180,   181,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,   141,   142,
      79,    79,    79,    79,    79,    79,    79,    13,    79,    81,
      90,    53,    81,    81,    54,    58,    86,    58,    73,    58,
      58,    58,    66,    90,    73,    91,    80,    80,    80,   223,
      66,    66,    66,    66,    80,    80,   230,   231,     3,   233,
     234,   235,   236,   237,   238,   239,   240,   241,    80,   243,
     244,    80,    80,    80,   248,    80,    80,    -1,   139,    80,
      91,    91,    50,    50,    80,    91,    80,    51,    91,    91,
     264,    91,   266,    80,    91,   269,    91,   271,    91,    91,
      91,    91,    80,    80,    80,    80,    80,    80,    80,    91,
      91,    80,   286,   287,   288,    80,    52,    80,   292,    90,
      90,    80,    80,    80,    80,    80,    80,    80,    80,    91,
      91,    91,   306,   307,   308,    51,   310,    80,    52,    80,
      91,   315,    90,    90,    82,    91,   320,    80,   322,    80,
     324,    90,   326,    82,    82,    82,    82,    80,    90,   333,
      82,    82,   336,    35,    35,    -1,    -1,    -1,   342,    -1,
      -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   357,   358,   359,   360,    -1,    -1,    -1,
      -1,    -1,    -1,   367,    -1,   369,     3,     4,     5,   373,
      -1,   375,    -1,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    52,    -1,    -1,    55,    56,
      57,    -1,    59,    60,    61,    62,    63,    64,    -1,    -1,
      67,    68,    69,    70,    71,    -1,    -1,    74,    75,    76,
      -1,    -1,    79,    -1,    -1,    -1,    83,    84,     3,     4,
       5,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      55,    56,    -1,    -1,    59,    60,    61,    62,    63,    64,
      -1,    -1,    67,    68,    69,    70,    71,    -1,    -1,    74,
      75,    76,    -1,    -1,    79,    -1,    -1,    -1,    83,    84,
       3,     4,     5,    -1,    -1,    -1,    -1,    92,    -1,    -1,
      -1,    -1,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    55,    56,    -1,    -1,    59,    60,    61,    62,
      63,    64,    -1,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    74,    75,    76,    -1,    -1,    79,    -1,    -1,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92
  };

  const signed char
  genesyspp_parser::yystos_[] =
  {
       0,     3,     4,     5,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    49,    52,    55,    56,    57,
      59,    60,    61,    62,    63,    64,    67,    68,    69,    70,
      71,    74,    75,    76,    79,    83,    84,    92,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   118,   119,   120,   121,   122,   123,    52,    75,    99,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,   119,   120,    81,    90,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    81,    90,    81,    95,   104,   104,    79,     0,    11,
      13,    10,    12,     6,     7,     8,     9,    88,    89,    83,
      84,    85,    87,    86,    81,    81,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    95,    95,   117,    95,
      90,    90,    95,    95,    53,    54,    58,    58,    58,    58,
      58,    73,    66,    66,    66,    66,    66,    73,    95,    95,
      95,    80,    95,    97,    98,    99,    99,   101,   101,   101,
     101,   101,   101,   102,   102,   103,   103,   103,    95,    95,
      80,    80,    80,    91,    80,    80,    80,    80,    80,    80,
      91,    91,    80,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    80,    91,    91,    95,    95,    82,    91,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      91,    91,    80,    82,    91,    82,    91,    80,    82,    91,
      82,    91,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    50,    50,    90,    95,
      52,     3,    90,    95,    95,    95,    95,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    91,    91,    91,    80,
      91,    95,    95,    95,    82,    91,    80,    91,    95,    82,
      91,    82,    91,    82,    91,    82,    91,    95,    95,    95,
      95,    51,    51,    90,    95,    52,    90,    95,    95,    95,
      95,    80,    91,    80,    52,    75,   122,   122,    95,    82,
      80,    95,    82,    82,    82,    82,    95,    81,    81,    90,
      90,    80,    95,    95,    95,    95,    82,    91,    82,    91,
      95,    95,    82,    91,    82,    91,    95,    95,    82,    82
  };

  const signed char
  genesyspp_parser::yyr1_[] =
  {
       0,    93,    94,    95,    95,    95,    95,    96,    96,    97,
      97,    98,    98,    98,    99,    99,   100,   100,   100,   100,
     100,   100,   100,   101,   101,   101,   102,   102,   102,   103,
     103,   104,   104,   104,   105,   105,   105,   105,   105,   105,
     106,   106,   107,   107,   108,   108,   109,   109,   110,   110,
     110,   110,   110,   110,   110,   111,   111,   111,   111,   111,
     111,   112,   112,   113,   113,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   116,   117,   117,   118,
     119,   119,   119,   119,   120,   120,   120,   120,   121,   121,
     121,   121,   122,   122,   122,   122,   122,   122,   122,   122,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123
  };

  const signed char
  genesyspp_parser::yyr2_[] =
  {
       0,     2,     1,     1,     1,     1,     1,     3,     1,     3,
       1,     3,     3,     1,     2,     1,     3,     3,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     1,     3,
       1,     2,     2,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     8,     6,     8,     8,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     6,     6,     6,     1,     4,     6,     6,     6,
       6,     6,     6,     8,    10,     4,     4,     5,     3,     1,
       1,     4,     6,     8,     1,     4,     6,     8,     1,     4,
       6,     8,     3,     6,     8,    10,     3,     6,     8,    10,
       1,     4,     4,     4,     6,     8,     4,     4,     4,     4,
       4,     4,     4
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
  "ATRIB", "CSTAT", "COUNTER", "fTAVG", "fCOUNT", "ILLEGAL", "RESOURCE",
  "fNR", "fMR", "fIRF", "fRESSEIZES", "fSTATE", "fSETSUM", "fRESUTIL",
  "QUEUE", "fNQ", "fFIRSTINQ", "fLASTINQ", "fSAQUE", "fAQUE", "fENTATRANK",
  "SET", "fNUMSET", "VARI", "FORM", "fNUMGR", "fATRGR", "\"(\"", "\")\"",
  "\"[\"", "\"]\"", "\"+\"", "\"-\"", "\"*\"", "\"^\"", "\"/\"", "\"<\"",
  "\">\"", "\"=\"", "\",\"", "\"USER\"", "$accept", "input", "expression",
  "logicalOr", "logicalXor", "logicalAnd", "logicalNot", "relational",
  "additive", "multiplicative", "power", "unary", "primary", "number",
  "command", "commandIF", "commandFOR", "function", "kernelFunction",
  "elementFunction", "trigonFunction", "mathFunction", "probFunction",
  "userFunction", "listaparm", "illegal", "attribute", "variable",
  "formula", "assigment", "pluginFunction", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  genesyspp_parser::yyrline_[] =
  {
       0,   263,   263,   268,   269,   270,   271,   275,   276,   280,
     281,   285,   286,   287,   291,   292,   296,   297,   298,   299,
     300,   301,   302,   306,   307,   308,   312,   313,   314,   318,
     319,   323,   324,   325,   329,   330,   331,   332,   337,   341,
     348,   349,   353,   354,   358,   359,   364,   365,   369,   370,
     371,   372,   373,   374,   375,   379,   380,   381,   382,   383,
     384,   389,   393,   400,   401,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   418,   437,   456,   475,   494,
     513,   532,   551,   570,   589,   608,   614,   619,   620,   625,
     650,   660,   669,   678,   692,   693,   696,   699,   707,   714,
     721,   728,   740,   744,   748,   752,   758,   762,   766,   770,
     781,   784,   786,   787,   794,   799,   807,   808,   809,   810,
     811,   812,   828
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
#line 2890 "../GenesysParser.cpp"

#line 835 "bisonparser.yy"

void
yy::genesyspp_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
