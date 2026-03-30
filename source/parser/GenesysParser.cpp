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


#line 51 "../GenesysParser.cpp"


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
#line 143 "../GenesysParser.cpp"

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
      case symbol_kind::S_number: // number
      case symbol_kind::S_arithmetic: // arithmetic
      case symbol_kind::S_logical: // logical
      case symbol_kind::S_relacional: // relacional
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
      case symbol_kind::S_number: // number
      case symbol_kind::S_arithmetic: // arithmetic
      case symbol_kind::S_logical: // logical
      case symbol_kind::S_relacional: // relacional
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
      case symbol_kind::S_number: // number
      case symbol_kind::S_arithmetic: // arithmetic
      case symbol_kind::S_logical: // logical
      case symbol_kind::S_relacional: // relacional
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
      case symbol_kind::S_number: // number
      case symbol_kind::S_arithmetic: // arithmetic
      case symbol_kind::S_logical: // logical
      case symbol_kind::S_relacional: // relacional
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

#line 817 "../GenesysParser.cpp"


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
      case symbol_kind::S_number: // number
      case symbol_kind::S_arithmetic: // arithmetic
      case symbol_kind::S_logical: // logical
      case symbol_kind::S_relacional: // relacional
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
#line 250 "bisonparser.yy"
                    { driver.setResult(yystack_[0].value.as < obj_t > ().valor);}
#line 1057 "../GenesysParser.cpp"
    break;

  case 3: // input: illegal
#line 251 "bisonparser.yy"
      { yylhs.value.as < obj_t > () = yystack_[0].value.as < obj_t > (); }
#line 1063 "../GenesysParser.cpp"
    break;

  case 4: // expression: number
#line 256 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1069 "../GenesysParser.cpp"
    break;

  case 5: // expression: function
#line 257 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1075 "../GenesysParser.cpp"
    break;

  case 6: // expression: command
#line 258 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1081 "../GenesysParser.cpp"
    break;

  case 7: // expression: assigment
#line 259 "bisonparser.yy"
                                      {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1087 "../GenesysParser.cpp"
    break;

  case 8: // expression: arithmetic
#line 260 "bisonparser.yy"
                                           {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1093 "../GenesysParser.cpp"
    break;

  case 9: // expression: logical
#line 261 "bisonparser.yy"
                                        {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1099 "../GenesysParser.cpp"
    break;

  case 10: // expression: relacional
#line 262 "bisonparser.yy"
                                       {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1105 "../GenesysParser.cpp"
    break;

  case 11: // expression: "(" expression ")"
#line 263 "bisonparser.yy"
                                            {yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor;}
#line 1111 "../GenesysParser.cpp"
    break;

  case 12: // expression: attribute
#line 264 "bisonparser.yy"
                                        {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1117 "../GenesysParser.cpp"
    break;

  case 13: // expression: variable
#line 269 "bisonparser.yy"
                                                   {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1123 "../GenesysParser.cpp"
    break;

  case 14: // expression: formula
#line 273 "bisonparser.yy"
                                                   {yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1129 "../GenesysParser.cpp"
    break;

  case 15: // number: NUMD
#line 280 "bisonparser.yy"
              { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1135 "../GenesysParser.cpp"
    break;

  case 16: // number: NUMH
#line 281 "bisonparser.yy"
              { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1141 "../GenesysParser.cpp"
    break;

  case 17: // arithmetic: expression "+" expression
#line 285 "bisonparser.yy"
                                     { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor + yystack_[0].value.as < obj_t > ().valor;}
#line 1147 "../GenesysParser.cpp"
    break;

  case 18: // arithmetic: expression "-" expression
#line 286 "bisonparser.yy"
                                     { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor - yystack_[0].value.as < obj_t > ().valor;}
#line 1153 "../GenesysParser.cpp"
    break;

  case 19: // arithmetic: expression "/" expression
#line 287 "bisonparser.yy"
                                     { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor / yystack_[0].value.as < obj_t > ().valor;}
#line 1159 "../GenesysParser.cpp"
    break;

  case 20: // arithmetic: expression "*" expression
#line 288 "bisonparser.yy"
                                     { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor * yystack_[0].value.as < obj_t > ().valor;}
#line 1165 "../GenesysParser.cpp"
    break;

  case 21: // arithmetic: expression "^" expression
#line 289 "bisonparser.yy"
                                     { yylhs.value.as < obj_t > ().valor = pow(yystack_[2].value.as < obj_t > ().valor,yystack_[0].value.as < obj_t > ().valor);}
#line 1171 "../GenesysParser.cpp"
    break;

  case 22: // arithmetic: "-" expression
#line 290 "bisonparser.yy"
                                     { yylhs.value.as < obj_t > ().valor = -yystack_[0].value.as < obj_t > ().valor;}
#line 1177 "../GenesysParser.cpp"
    break;

  case 23: // arithmetic: mathMIN "(" expression "," expression ")"
#line 293 "bisonparser.yy"
                                                            {std::cout <<"MIN(" << yystack_[3].value.as < obj_t > ().valor << "," << yystack_[1].value.as < obj_t > ().valor <<")"<< std::endl;
														 yylhs.value.as < obj_t > ().valor = std::min(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1184 "../GenesysParser.cpp"
    break;

  case 24: // arithmetic: mathMAX "(" expression "," expression ")"
#line 295 "bisonparser.yy"
                                                            { yylhs.value.as < obj_t > ().valor = std::max(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1190 "../GenesysParser.cpp"
    break;

  case 25: // logical: expression oAND expression
#line 299 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = (int) yystack_[2].value.as < obj_t > ().valor && (int) yystack_[0].value.as < obj_t > ().valor;}
#line 1196 "../GenesysParser.cpp"
    break;

  case 26: // logical: expression oOR expression
#line 300 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = (int) yystack_[2].value.as < obj_t > ().valor || (int) yystack_[0].value.as < obj_t > ().valor;}
#line 1202 "../GenesysParser.cpp"
    break;

  case 27: // logical: expression oNAND expression
#line 301 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = !((int) yystack_[2].value.as < obj_t > ().valor && (int) yystack_[0].value.as < obj_t > ().valor);}
#line 1208 "../GenesysParser.cpp"
    break;

  case 28: // logical: expression oXOR expression
#line 302 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = (!(int) yystack_[2].value.as < obj_t > ().valor && (int) yystack_[0].value.as < obj_t > ().valor) || ((int) yystack_[2].value.as < obj_t > ().valor && !(int) yystack_[0].value.as < obj_t > ().valor);}
#line 1214 "../GenesysParser.cpp"
    break;

  case 29: // logical: oNOT expression
#line 303 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = !(int) yystack_[0].value.as < obj_t > ().valor;}
#line 1220 "../GenesysParser.cpp"
    break;

  case 30: // relacional: expression "<" expression
#line 307 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor < yystack_[0].value.as < obj_t > ().valor ? 1 : 0;}
#line 1226 "../GenesysParser.cpp"
    break;

  case 31: // relacional: expression ">" expression
#line 308 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor > yystack_[0].value.as < obj_t > ().valor ? 1 : 0;}
#line 1232 "../GenesysParser.cpp"
    break;

  case 32: // relacional: expression oLE expression
#line 309 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor <= yystack_[0].value.as < obj_t > ().valor ? 1 : 0;}
#line 1238 "../GenesysParser.cpp"
    break;

  case 33: // relacional: expression oGE expression
#line 310 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor >= yystack_[0].value.as < obj_t > ().valor ? 1 : 0;}
#line 1244 "../GenesysParser.cpp"
    break;

  case 34: // relacional: expression oEQ expression
#line 311 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor == yystack_[0].value.as < obj_t > ().valor ? 1 : 0;}
#line 1250 "../GenesysParser.cpp"
    break;

  case 35: // relacional: expression oNE expression
#line 312 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = yystack_[2].value.as < obj_t > ().valor != yystack_[0].value.as < obj_t > ().valor ? 1 : 0;}
#line 1256 "../GenesysParser.cpp"
    break;

  case 36: // command: commandIF
#line 316 "bisonparser.yy"
                    { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1262 "../GenesysParser.cpp"
    break;

  case 37: // command: commandFOR
#line 317 "bisonparser.yy"
                    { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1268 "../GenesysParser.cpp"
    break;

  case 38: // commandIF: cIF expression expression cELSE expression
#line 321 "bisonparser.yy"
                                                   { yylhs.value.as < obj_t > ().valor = yystack_[3].value.as < obj_t > ().valor != 0 ? yystack_[2].value.as < obj_t > ().valor : yystack_[0].value.as < obj_t > ().valor; }
#line 1274 "../GenesysParser.cpp"
    break;

  case 39: // commandIF: cIF expression expression
#line 322 "bisonparser.yy"
                                                  { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor != 0 ? yystack_[0].value.as < obj_t > ().valor : 0;}
#line 1280 "../GenesysParser.cpp"
    break;

  case 40: // commandFOR: cFOR variable "=" expression cTO expression cDO assigment
#line 327 "bisonparser.yy"
                                                                {yylhs.value.as < obj_t > ().valor = 0; }
#line 1286 "../GenesysParser.cpp"
    break;

  case 41: // commandFOR: cFOR attribute "=" expression cTO expression cDO assigment
#line 328 "bisonparser.yy"
                                                                  {yylhs.value.as < obj_t > ().valor = 0; }
#line 1292 "../GenesysParser.cpp"
    break;

  case 42: // function: mathFunction
#line 332 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1298 "../GenesysParser.cpp"
    break;

  case 43: // function: trigonFunction
#line 333 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1304 "../GenesysParser.cpp"
    break;

  case 44: // function: probFunction
#line 334 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1310 "../GenesysParser.cpp"
    break;

  case 45: // function: kernelFunction
#line 335 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1316 "../GenesysParser.cpp"
    break;

  case 46: // function: elementFunction
#line 336 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1322 "../GenesysParser.cpp"
    break;

  case 47: // function: pluginFunction
#line 337 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1328 "../GenesysParser.cpp"
    break;

  case 48: // function: userFunction
#line 338 "bisonparser.yy"
                         { yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1334 "../GenesysParser.cpp"
    break;

  case 49: // kernelFunction: fTNOW
#line 342 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getSimulatedTime();}
#line 1340 "../GenesysParser.cpp"
    break;

  case 50: // kernelFunction: fTFIN
#line 343 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getReplicationLength();}
#line 1346 "../GenesysParser.cpp"
    break;

  case 51: // kernelFunction: fMAXREP
#line 344 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getNumberOfReplications();}
#line 1352 "../GenesysParser.cpp"
    break;

  case 52: // kernelFunction: fNUMREP
#line 345 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getCurrentReplicationNumber();}
#line 1358 "../GenesysParser.cpp"
    break;

  case 53: // kernelFunction: fIDENT
#line 346 "bisonparser.yy"
                 { yylhs.value.as < obj_t > ().valor = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getId();}
#line 1364 "../GenesysParser.cpp"
    break;

  case 54: // kernelFunction: simulEntitiesWIP
#line 347 "bisonparser.yy"
                            { yylhs.value.as < obj_t > ().valor = driver.getModel()->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Entity>());}
#line 1370 "../GenesysParser.cpp"
    break;

  case 55: // elementFunction: fTAVG "(" CSTAT ")"
#line 352 "bisonparser.yy"
                               {
                    StatisticsCollector* cstat = ((StatisticsCollector*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<StatisticsCollector>(), yystack_[1].value.as < obj_t > ().id)));
                    double value = cstat->getStatistics()->average();
                    yylhs.value.as < obj_t > ().valor = value; }
#line 1379 "../GenesysParser.cpp"
    break;

  case 56: // elementFunction: fCOUNT "(" COUNTER ")"
#line 356 "bisonparser.yy"
                                 {
					Counter* counter = ((Counter*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Counter>(), yystack_[1].value.as < obj_t > ().id)));
                    double value = counter->getCountValue();
                    yylhs.value.as < obj_t > ().valor = value; }
#line 1388 "../GenesysParser.cpp"
    break;

  case 57: // trigonFunction: fSIN "(" expression ")"
#line 363 "bisonparser.yy"
                                  { yylhs.value.as < obj_t > ().valor = sin(yystack_[1].value.as < obj_t > ().valor); }
#line 1394 "../GenesysParser.cpp"
    break;

  case 58: // trigonFunction: fCOS "(" expression ")"
#line 364 "bisonparser.yy"
                                  { yylhs.value.as < obj_t > ().valor = cos(yystack_[1].value.as < obj_t > ().valor); }
#line 1400 "../GenesysParser.cpp"
    break;

  case 59: // mathFunction: fROUND "(" expression ")"
#line 368 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = round(yystack_[1].value.as < obj_t > ().valor);}
#line 1406 "../GenesysParser.cpp"
    break;

  case 60: // mathFunction: fFRAC "(" expression ")"
#line 369 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor - (int) yystack_[1].value.as < obj_t > ().valor;}
#line 1412 "../GenesysParser.cpp"
    break;

  case 61: // mathFunction: fTRUNC "(" expression ")"
#line 370 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = trunc(yystack_[1].value.as < obj_t > ().valor);}
#line 1418 "../GenesysParser.cpp"
    break;

  case 62: // mathFunction: fEXP "(" expression ")"
#line 371 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = exp(yystack_[1].value.as < obj_t > ().valor);}
#line 1424 "../GenesysParser.cpp"
    break;

  case 63: // mathFunction: fSQRT "(" expression ")"
#line 372 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = sqrt(yystack_[1].value.as < obj_t > ().valor);}
#line 1430 "../GenesysParser.cpp"
    break;

  case 64: // mathFunction: fLOG "(" expression ")"
#line 373 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = log10(yystack_[1].value.as < obj_t > ().valor);}
#line 1436 "../GenesysParser.cpp"
    break;

  case 65: // mathFunction: fLN "(" expression ")"
#line 374 "bisonparser.yy"
                                    { yylhs.value.as < obj_t > ().valor = log(yystack_[1].value.as < obj_t > ().valor);}
#line 1442 "../GenesysParser.cpp"
    break;

  case 66: // mathFunction: fMOD "(" expression "," expression ")"
#line 375 "bisonparser.yy"
                                               { yylhs.value.as < obj_t > ().valor = (int) yystack_[3].value.as < obj_t > ().valor % (int) yystack_[1].value.as < obj_t > ().valor; }
#line 1448 "../GenesysParser.cpp"
    break;

  case 67: // probFunction: fRND1
#line 379 "bisonparser.yy"
                                                     { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleUniform(0.0,1.0);}
#line 1454 "../GenesysParser.cpp"
    break;

  case 68: // probFunction: fEXPO "(" expression ")"
#line 380 "bisonparser.yy"
                                     { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleExponential(yystack_[1].value.as < obj_t > ().valor);}
#line 1460 "../GenesysParser.cpp"
    break;

  case 69: // probFunction: fNORM "(" expression "," expression ")"
#line 381 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleNormal(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1466 "../GenesysParser.cpp"
    break;

  case 70: // probFunction: fUNIF "(" expression "," expression ")"
#line 382 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleUniform(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1472 "../GenesysParser.cpp"
    break;

  case 71: // probFunction: fWEIB "(" expression "," expression ")"
#line 383 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleWeibull(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1478 "../GenesysParser.cpp"
    break;

  case 72: // probFunction: fLOGN "(" expression "," expression ")"
#line 384 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleLogNormal(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1484 "../GenesysParser.cpp"
    break;

  case 73: // probFunction: fGAMM "(" expression "," expression ")"
#line 385 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleGamma(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1490 "../GenesysParser.cpp"
    break;

  case 74: // probFunction: fERLA "(" expression "," expression ")"
#line 386 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleErlang(yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1496 "../GenesysParser.cpp"
    break;

  case 75: // probFunction: fTRIA "(" expression "," expression "," expression ")"
#line 387 "bisonparser.yy"
                                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleTriangular(yystack_[5].value.as < obj_t > ().valor,yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1502 "../GenesysParser.cpp"
    break;

  case 76: // probFunction: fBETA "(" expression "," expression "," expression "," expression ")"
#line 388 "bisonparser.yy"
                                                                                  { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleBeta(yystack_[7].value.as < obj_t > ().valor,yystack_[5].value.as < obj_t > ().valor,yystack_[3].value.as < obj_t > ().valor,yystack_[1].value.as < obj_t > ().valor);}
#line 1508 "../GenesysParser.cpp"
    break;

  case 77: // probFunction: fDISC "(" listaparm ")"
#line 389 "bisonparser.yy"
                                                    { yylhs.value.as < obj_t > ().valor = driver.getSampler()->sampleDiscrete(0,0); /*@TODO: NOT IMPLEMENTED YET*/ }
#line 1514 "../GenesysParser.cpp"
    break;

  case 78: // userFunction: "USER" "(" expression ")"
#line 395 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = yystack_[1].value.as < obj_t > ().valor; }
#line 1520 "../GenesysParser.cpp"
    break;

  case 79: // listaparm: listaparm "," expression "," expression
#line 400 "bisonparser.yy"
                                                 {/*@TODO: NOT IMPLEMENTED YET*/}
#line 1526 "../GenesysParser.cpp"
    break;

  case 80: // listaparm: expression "," expression
#line 401 "bisonparser.yy"
                                                 {/*@TODO: NOT IMPLEMENTED YET*/}
#line 1532 "../GenesysParser.cpp"
    break;

  case 81: // illegal: ILLEGAL
#line 406 "bisonparser.yy"
                          {
		driver.setResult(-1);
		if(driver.getThrowsException()){
			if(yystack_[0].value.as < obj_t > ().valor == 0){
			  throw std::string("Literal nao encontrado");
			}else if(yystack_[0].value.as < obj_t > ().valor == 1){
			  throw std::string("Caracter invalido encontrado");
			}
		} else {
			if(yystack_[0].value.as < obj_t > ().valor == 0){
			  driver.setErrorMessage(std::string("Literal nao encontrado"));
			}else if(yystack_[0].value.as < obj_t > ().valor == 1){
				driver.setErrorMessage(std::string("Caracter invalido encontrado"));
			}
		}
	}
#line 1553 "../GenesysParser.cpp"
    break;

  case 82: // attribute: ATRIB
#line 427 "bisonparser.yy"
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
#line 1568 "../GenesysParser.cpp"
    break;

  case 83: // attribute: ATRIB "[" expression "]"
#line 437 "bisonparser.yy"
                                              {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[3].value.as < obj_t > ().id, index);
		}
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 1582 "../GenesysParser.cpp"
    break;

  case 84: // attribute: ATRIB "[" expression "," expression "]"
#line 446 "bisonparser.yy"
                                                             {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[5].value.as < obj_t > ().id, index);
		}
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 1596 "../GenesysParser.cpp"
    break;

  case 85: // attribute: ATRIB "[" expression "," expression "," expression "]"
#line 455 "bisonparser.yy"
                                                                            {  
		double attributeValue = 0.0;
		std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
		if (driver.getModel()->getSimulation()->getCurrentEvent() != nullptr) {
			// it could crach because there may be no current entity, if the parse is running before simulation and therefore there is no CurrentEntity
			attributeValue = driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->getAttributeValue(yystack_[7].value.as < obj_t > ().id, index);
		}
		yylhs.value.as < obj_t > ().valor = attributeValue; 
	}
#line 1610 "../GenesysParser.cpp"
    break;

  case 86: // variable: VARI
#line 469 "bisonparser.yy"
                            {yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[0].value.as < obj_t > ().id)))->getValue();}
#line 1616 "../GenesysParser.cpp"
    break;

  case 87: // variable: VARI "[" expression "]"
#line 470 "bisonparser.yy"
                                                                            { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[3].value.as < obj_t > ().id)))->getValue(index); }
#line 1624 "../GenesysParser.cpp"
    break;

  case 88: // variable: VARI "[" expression "," expression "]"
#line 473 "bisonparser.yy"
                                                                                            { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor)); 
					yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[5].value.as < obj_t > ().id)))->getValue(index);}
#line 1632 "../GenesysParser.cpp"
    break;

  case 89: // variable: VARI "[" expression "," expression "," expression "]"
#line 476 "bisonparser.yy"
                                                                                                     { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					yylhs.value.as < obj_t > ().valor = ((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[7].value.as < obj_t > ().id)))->getValue(index);}
#line 1640 "../GenesysParser.cpp"
    break;

  case 90: // formula: FORM
#line 484 "bisonparser.yy"
                                    { 
					std::string index = "";
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[0].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1652 "../GenesysParser.cpp"
    break;

  case 91: // formula: FORM "[" expression "]"
#line 491 "bisonparser.yy"
                                                                    {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[3].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1664 "../GenesysParser.cpp"
    break;

  case 92: // formula: FORM "[" expression "," expression "]"
#line 498 "bisonparser.yy"
                                                                                   {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor)) +","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[5].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1676 "../GenesysParser.cpp"
    break;

  case 93: // formula: FORM "[" expression "," expression "," expression "]"
#line 505 "bisonparser.yy"
                                                                                                  {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor)) +","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[1].value.as < obj_t > ().valor));
					Formula* formula = dynamic_cast<Formula*>(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Formula>(), yystack_[7].value.as < obj_t > ().id));
					std::string expression = formula->getExpression(index);
					//std::cout << "Formula["<< index <<"]="<< expression << std::endl;
					double value = 0.0; //@TODO: Can't parse the epression!  //formula->getValue(index);
					yylhs.value.as < obj_t > ().valor = value;}
#line 1688 "../GenesysParser.cpp"
    break;

  case 94: // assigment: ATRIB "=" expression
#line 517 "bisonparser.yy"
                                                { 
					// @TODO: getCurrentEvent()->getEntity() may be nullptr if simulation hasn't started yet
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[2].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1697 "../GenesysParser.cpp"
    break;

  case 95: // assigment: ATRIB "[" expression "]" "=" expression
#line 521 "bisonparser.yy"
                                                                                          { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor));
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[5].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1706 "../GenesysParser.cpp"
    break;

  case 96: // assigment: ATRIB "[" expression "," expression "]" "=" expression
#line 525 "bisonparser.yy"
                                                                                                        {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor)); 
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[7].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor;}
#line 1715 "../GenesysParser.cpp"
    break;

  case 97: // assigment: ATRIB "[" expression "," expression "," expression "]" "=" expression
#line 529 "bisonparser.yy"
                                                                                                                          {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[7].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor));
					driver.getModel()->getSimulation()->getCurrentEvent()->getEntity()->setAttributeValue(yystack_[9].value.as < obj_t > ().id, yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1724 "../GenesysParser.cpp"
    break;

  case 98: // assigment: VARI "=" expression
#line 535 "bisonparser.yy"
                                                                {
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[2].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; 
					}
#line 1733 "../GenesysParser.cpp"
    break;

  case 99: // assigment: VARI "[" expression "]" "=" expression
#line 539 "bisonparser.yy"
                                                                                         { 
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor));
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[5].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1742 "../GenesysParser.cpp"
    break;

  case 100: // assigment: VARI "[" expression "," expression "]" "=" expression
#line 543 "bisonparser.yy"
                                                                                                       {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor)); 
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[7].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1751 "../GenesysParser.cpp"
    break;

  case 101: // assigment: VARI "[" expression "," expression "," expression "]" "=" expression
#line 547 "bisonparser.yy"
                                                                                                                         {
					std::string index = std::to_string(static_cast<unsigned int>(yystack_[7].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[5].value.as < obj_t > ().valor))+","+std::to_string(static_cast<unsigned int>(yystack_[3].value.as < obj_t > ().valor));
					((Variable*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), yystack_[9].value.as < obj_t > ().id)))->setValue(yystack_[0].value.as < obj_t > ().valor, index);
					yylhs.value.as < obj_t > ().valor = yystack_[0].value.as < obj_t > ().valor; }
#line 1760 "../GenesysParser.cpp"
    break;

  case 102: // pluginFunction: CTEZERO
#line 558 "bisonparser.yy"
                                                     { yylhs.value.as < obj_t > ().valor = 0; }
#line 1766 "../GenesysParser.cpp"
    break;

  case 103: // pluginFunction: fNQ "(" QUEUE ")"
#line 561 "bisonparser.yy"
                                    {   //std::cout << "Queue ID: " << $3.id << ", Size: " << ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), $3.id)))->size() << std::endl; 
                                        yylhs.value.as < obj_t > ().valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->size();}
#line 1773 "../GenesysParser.cpp"
    break;

  case 104: // pluginFunction: fLASTINQ "(" QUEUE ")"
#line 563 "bisonparser.yy"
                                    {/*For now does nothing because need acces to list of QUEUE, or at least the last element*/ }
#line 1779 "../GenesysParser.cpp"
    break;

  case 105: // pluginFunction: fFIRSTINQ "(" QUEUE ")"
#line 564 "bisonparser.yy"
                                    { 
                if (((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->size() > 0){
                    //id da 1a entidade da fila, talvez pegar nome
                    yylhs.value.as < obj_t > ().valor = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[1].value.as < obj_t > ().id)))->first()->getEntity()->getId();
                }else{
                    yylhs.value.as < obj_t > ().valor = 0;
                } }
#line 1791 "../GenesysParser.cpp"
    break;

  case 106: // pluginFunction: fSAQUE "(" QUEUE "," ATRIB ")"
#line 571 "bisonparser.yy"
                                       {   
                //Util::identification queueID = $3.id;
                Util::identification attrID = yystack_[1].value.as < obj_t > ().id;
                double sum = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[3].value.as < obj_t > ().id)))->sumAttributesFromWaiting(attrID);
                yylhs.value.as < obj_t > ().valor = sum; }
#line 1801 "../GenesysParser.cpp"
    break;

  case 107: // pluginFunction: fAQUE "(" QUEUE "," NUMD "," ATRIB ")"
#line 576 "bisonparser.yy"
                                             {
                //Util::identification queueID = $3.id;
                Util::identification attrID = yystack_[1].value.as < obj_t > ().id;
                double value = ((Queue*)(driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), yystack_[5].value.as < obj_t > ().id)))->getAttributeFromWaitingRank(yystack_[3].value.as < obj_t > ().valor-1, attrID); // rank starts on 0 in genesys
                yylhs.value.as < obj_t > ().valor = value; }
#line 1811 "../GenesysParser.cpp"
    break;

  case 108: // pluginFunction: fMR "(" RESOURCE ")"
#line 584 "bisonparser.yy"
                                        { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getCapacity();}
#line 1817 "../GenesysParser.cpp"
    break;

  case 109: // pluginFunction: fNR "(" RESOURCE ")"
#line 585 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getNumberBusy();}
#line 1823 "../GenesysParser.cpp"
    break;

  case 110: // pluginFunction: fRESSEIZES "(" RESOURCE ")"
#line 586 "bisonparser.yy"
                                         { /*\TODO: For now does nothing because needs get Seizes, check with teacher*/}
#line 1829 "../GenesysParser.cpp"
    break;

  case 111: // pluginFunction: fSTATE "(" RESOURCE ")"
#line 587 "bisonparser.yy"
                                         {  yylhs.value.as < obj_t > ().valor = static_cast<int>(((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getResourceState()); }
#line 1835 "../GenesysParser.cpp"
    break;

  case 112: // pluginFunction: fIRF "(" RESOURCE ")"
#line 588 "bisonparser.yy"
                                         { yylhs.value.as < obj_t > ().valor = ((Resource*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), yystack_[1].value.as < obj_t > ().id))->getResourceState() == Resource::ResourceState::FAILED ? 1 : 0; }
#line 1841 "../GenesysParser.cpp"
    break;

  case 113: // pluginFunction: fSETSUM "(" SET ")"
#line 589 "bisonparser.yy"
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
#line 1859 "../GenesysParser.cpp"
    break;

  case 114: // pluginFunction: fNUMSET "(" SET ")"
#line 605 "bisonparser.yy"
                                { yylhs.value.as < obj_t > ().valor = ((Set*)driver.getModel()->getDataManager()->getDataDefinition(Util::TypeOf<Set>(),yystack_[1].value.as < obj_t > ().id))->getElementSet()->size(); }
#line 1865 "../GenesysParser.cpp"
    break;


#line 1869 "../GenesysParser.cpp"

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


  const short genesyspp_parser::yypact_ninf_ = -309;

  const signed char genesyspp_parser::yytable_ninf_ = -23;

  const short
  genesyspp_parser::yypact_[] =
  {
     445,  -309,  -309,  -309,   527,   -68,   -64,   -57,   -56,   -55,
     -54,   -51,   -50,   -45,   -36,   -35,   -34,  -309,   -33,   -32,
     -31,   -30,   -27,   -26,   -25,   -24,   -23,   -22,  -309,  -309,
    -309,  -309,  -309,  -309,   527,   -49,   -73,   -21,   -20,  -309,
     -19,   -18,   -17,   -16,   -15,   -14,   -13,   -12,   -10,    -5,
      22,    31,   -72,    18,   527,   527,    46,   108,  1807,  -309,
    -309,  -309,  -309,  -309,  -309,  -309,  -309,  -309,  -309,  -309,
    -309,  -309,  -309,  -309,  -309,  -309,  -309,  -309,  -309,   199,
     527,   527,   527,   527,   527,   527,   527,   527,   527,   527,
     527,   527,   527,   527,   527,   527,   527,   527,   527,   527,
     527,   527,   358,    61,    67,   -59,    51,   527,   527,    96,
      97,    92,    94,    95,    98,    99,    81,    89,    93,   104,
     105,   106,    85,   527,   527,   527,    29,    -6,   527,  -309,
     527,   527,   527,   527,   527,   527,   527,   527,   527,   527,
     527,   527,   527,   527,   527,  1110,  1120,  1204,   811,  1214,
    1224,  1234,  1244,  1254,  1264,   820,   906,  1348,   915,   924,
     933,   942,   951,   960,   969,  1055,  1064,   -75,   527,  1356,
     527,   527,   527,   527,   174,  1807,    80,    82,    83,   109,
     111,   112,   113,   114,   118,   119,   120,    70,    74,   121,
     606,  1807,   616,  -309,  1366,    60,    60,    60,    60,   199,
     199,  1807,  1807,   110,   110,   -11,   -11,   -11,    60,    60,
    -309,  -309,  -309,   527,  -309,  -309,  -309,  -309,  -309,  -309,
     527,   527,  -309,   527,   527,   527,   527,   527,   527,   527,
     527,   527,  -309,   527,    20,   527,   626,   636,  1374,  1388,
      84,   527,  -309,  -309,  -309,  -309,  -309,  -309,  -309,  -309,
    -309,  -309,  -309,   123,   170,  -309,    86,   527,  -309,   527,
    -309,  1399,  1409,  1493,  1503,  1513,  1523,  1533,  1543,  1553,
    1073,  1082,  1807,  1091,    60,  -309,   527,  -309,   527,   527,
     527,   527,   646,   122,    87,   527,   656,   666,  -309,  -309,
    -309,  -309,  -309,  -309,  -309,  -309,  -309,   527,   527,   527,
     752,   762,  1637,  1645,  1807,   100,   527,  -309,   125,  1807,
     124,   527,  -309,   527,  1655,  1100,  1807,  -309,   527,  -309,
     527,   -48,   -48,   527,  1663,   130,   527,  1671,  1691,  -309,
     527,  1699,  1755,   -71,   -69,  -309,  -309,  1807,   128,  -309,
    1807,   138,  -309,  1783,  -309,  -309,   527,   527,   527,   527,
    -309,   772,   782,  1807,  1807,    84,   527,    86,   527,   792,
     802,   100,   527,   124,   527,  1791,  1799,   128,   138
  };

  const signed char
  genesyspp_parser::yydefact_[] =
  {
       0,    15,    16,   102,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    67,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    49,    50,
      51,    52,    53,    54,     0,     0,    82,     0,     0,    81,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    86,    90,     0,     0,     0,     0,     2,     4,
       8,     9,    10,     6,    36,    37,     5,    45,    46,    43,
      42,    44,    48,     3,    12,    13,    14,     7,    47,    29,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,     0,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    39,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    98,     0,    11,     0,    32,    33,    34,    35,    25,
      26,    27,    28,    17,    18,    20,    21,    19,    30,    31,
      57,    58,    59,     0,    61,    60,    62,    63,    64,    65,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    77,     0,    18,     0,     0,     0,     0,     0,
      83,     0,    55,    56,   109,   108,   112,   110,   111,   113,
     103,   105,   104,     0,     0,   114,    87,     0,    91,     0,
      78,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    80,     0,    38,    83,     0,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    66,    23,
      24,    69,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,    95,    84,     0,   106,     0,    99,
      88,     0,    92,     0,     0,     0,    79,    84,     0,    88,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,     0,    41,    40,    96,    85,   107,
     100,    89,    93,     0,    85,    89,     0,     0,     0,     0,
      76,     0,     0,    97,   101,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0
  };

  const short
  genesyspp_parser::yypgoto_[] =
  {
    -309,  -309,    -4,  -309,  -309,  -309,  -309,  -309,  -309,  -309,
    -309,  -309,  -309,  -309,  -309,  -309,  -309,  -309,  -309,   144,
     153,  -309,  -308,  -309
  };

  const unsigned char
  genesyspp_parser::yydefgoto_[] =
  {
       0,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,   167,    73,    74,
      75,    76,    77,    78
  };

  const short
  genesyspp_parser::yytable_[] =
  {
      79,   136,   137,   103,   333,   232,   136,   137,   107,   123,
     346,    80,   347,   335,   336,    81,   233,   108,   124,   108,
     -22,   124,    82,    83,    84,    85,   104,   334,    86,    87,
     102,   172,   136,   137,    88,   130,   131,   132,   133,   134,
     135,   136,   137,    89,    90,    91,    92,    93,    94,    95,
     126,   127,    96,    97,    98,    99,   100,   101,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   -22,   119,
     -22,   -22,   136,   137,   120,   141,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   169,   125,
     -22,   121,   -22,   174,   175,   -22,   -22,   -22,   129,   193,
     122,   -22,   138,   139,   140,   141,   142,   143,   144,   190,
     191,   192,   136,   137,   194,   128,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   173,   170,   138,   139,   140,   141,   142,   171,   176,
     178,   177,   179,   180,   183,   184,   181,   182,   189,   185,
     242,   253,   243,   244,   234,   254,   236,   237,   238,   239,
     186,   187,   188,   284,   281,   283,   285,   325,   308,   105,
     130,   131,   132,   133,   134,   135,   136,   137,   106,   245,
     323,   246,   247,   248,   249,   140,   141,   142,   250,   251,
     252,   255,   307,     0,     0,   130,   131,   132,   133,   261,
     339,   136,   137,     0,   326,     0,   262,   263,   348,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   349,   273,
       0,   274,     0,     0,     0,     0,     0,   282,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   286,     0,   287,   240,   138,   139,   140,
     141,   142,   143,   144,     0,   241,     0,     0,     0,     0,
       0,     0,   300,     0,   301,   302,   303,   304,     0,     0,
       0,   309,   138,   139,   140,   141,   142,   143,   144,     0,
       0,     0,     0,   314,   315,   316,     0,     0,     0,     0,
       0,     0,   324,     0,     0,     0,     0,   327,     0,   328,
       0,     0,     0,     0,   331,     0,   332,     0,     0,   337,
       0,     0,   340,     0,     0,     0,   343,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   351,   352,   353,   354,     0,     0,     0,     0,
       0,     0,   359,     0,   360,     0,     0,     0,   365,     0,
     366,     1,     2,     3,   130,   131,   132,   133,   134,   135,
     136,   137,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     0,     0,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,     0,    35,     0,     0,
      36,     0,     0,    37,    38,     0,     0,    40,    41,    42,
      43,    44,    45,     0,     0,    46,    47,    48,    49,    50,
       0,     0,    51,    52,    53,     0,     0,    54,     0,     0,
       0,   138,   168,   140,   141,   142,   143,   144,     1,     2,
       3,    56,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,     0,     0,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,     0,    35,     0,     0,    36,     0,     0,
      37,    38,    39,     0,    40,    41,    42,    43,    44,    45,
       0,     0,    46,    47,    48,    49,    50,     0,     0,    51,
      52,    53,     0,     0,    54,     0,     0,     0,     0,    55,
       1,     2,     3,     0,     0,     0,     0,     0,    56,     0,
       0,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     0,     0,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,     0,    35,     0,     0,    36,
       0,     0,    37,    38,     0,     0,    40,    41,    42,    43,
      44,    45,     0,     0,    46,    47,    48,    49,    50,     0,
       0,    51,    52,    53,     0,     0,    54,     0,     0,     0,
       0,    55,   130,   131,   132,   133,   134,   135,   136,   137,
      56,     0,   130,   131,   132,   133,   134,   135,   136,   137,
       0,     0,   130,   131,   132,   133,   134,   135,   136,   137,
       0,     0,   130,   131,   132,   133,   134,   135,   136,   137,
       0,     0,   130,   131,   132,   133,   134,   135,   136,   137,
       0,     0,   130,   131,   132,   133,   134,   135,   136,   137,
       0,     0,   130,   131,   132,   133,   134,   135,   136,   137,
       0,     0,     0,     0,     0,     0,     0,     0,   256,   138,
     139,   140,   141,   142,   143,   144,     0,   257,   258,   138,
     139,   140,   141,   142,   143,   144,     0,   259,   275,   138,
     139,   140,   141,   142,   143,   144,     0,   276,   277,   138,
     139,   140,   141,   142,   143,   144,     0,   278,   305,   138,
     139,   140,   141,   142,   143,   144,     0,   306,   310,   138,
     139,   140,   141,   142,   143,   144,     0,   311,   312,   138,
     139,   140,   141,   142,   143,   144,     0,   313,   130,   131,
     132,   133,   134,   135,   136,   137,     0,     0,   130,   131,
     132,   133,   134,   135,   136,   137,     0,     0,   130,   131,
     132,   133,   134,   135,   136,   137,     0,     0,   130,   131,
     132,   133,   134,   135,   136,   137,     0,     0,   130,   131,
     132,   133,   134,   135,   136,   137,     0,     0,   130,   131,
     132,   133,   134,   135,   136,   137,     0,   130,   131,   132,
     133,   134,   135,   136,   137,     0,   130,   131,   132,   133,
     134,   135,   136,   137,   317,   138,   139,   140,   141,   142,
     143,   144,     0,   318,   319,   138,   139,   140,   141,   142,
     143,   144,     0,   320,   355,   138,   139,   140,   141,   142,
     143,   144,     0,   356,   357,   138,   139,   140,   141,   142,
     143,   144,     0,   358,   361,   138,   139,   140,   141,   142,
     143,   144,     0,   362,   363,   138,   139,   140,   141,   142,
     143,   144,     0,   364,   138,   139,   140,   141,   142,   143,
     144,     0,   213,   138,   139,   140,   141,   142,   143,   144,
       0,   220,   130,   131,   132,   133,   134,   135,   136,   137,
       0,   130,   131,   132,   133,   134,   135,   136,   137,     0,
     130,   131,   132,   133,   134,   135,   136,   137,     0,   130,
     131,   132,   133,   134,   135,   136,   137,     0,   130,   131,
     132,   133,   134,   135,   136,   137,     0,   130,   131,   132,
     133,   134,   135,   136,   137,     0,   130,   131,   132,   133,
     134,   135,   136,   137,     0,   130,   131,   132,   133,   134,
     135,   136,   137,     0,     0,     0,     0,     0,     0,   138,
     139,   140,   141,   142,   143,   144,     0,   221,   138,   139,
     140,   141,   142,   143,   144,     0,   223,   138,   139,   140,
     141,   142,   143,   144,     0,   224,   138,   139,   140,   141,
     142,   143,   144,     0,   225,   138,   139,   140,   141,   142,
     143,   144,     0,   226,   138,   139,   140,   141,   142,   143,
     144,     0,   227,   138,   139,   140,   141,   142,   143,   144,
       0,   228,   138,   139,   140,   141,   142,   143,   144,     0,
     229,   130,   131,   132,   133,   134,   135,   136,   137,     0,
     130,   131,   132,   133,   134,   135,   136,   137,     0,   130,
     131,   132,   133,   134,   135,   136,   137,     0,   130,   131,
     132,   133,   134,   135,   136,   137,     0,   130,   131,   132,
     133,   134,   135,   136,   137,     0,   130,   131,   132,   133,
     134,   135,   136,   137,     0,     0,   130,   131,   132,   133,
     134,   135,   136,   137,     0,     0,   130,   131,   132,   133,
     134,   135,   136,   137,     0,     0,     0,     0,   138,   139,
     140,   141,   142,   143,   144,     0,   230,   138,   139,   140,
     141,   142,   143,   144,     0,   231,   138,   139,   140,   141,
     142,   143,   144,     0,   297,   138,   139,   140,   141,   142,
     143,   144,     0,   298,   138,   139,   140,   141,   142,   143,
     144,     0,   299,   138,   139,   140,   141,   142,   143,   144,
     210,   330,     0,   138,   139,   140,   141,   142,   143,   144,
     211,     0,     0,   138,   139,   140,   141,   142,   143,   144,
     130,   131,   132,   133,   134,   135,   136,   137,     0,     0,
     130,   131,   132,   133,   134,   135,   136,   137,     0,     0,
     130,   131,   132,   133,   134,   135,   136,   137,     0,     0,
     130,   131,   132,   133,   134,   135,   136,   137,     0,     0,
     130,   131,   132,   133,   134,   135,   136,   137,     0,     0,
     130,   131,   132,   133,   134,   135,   136,   137,     0,     0,
     130,   131,   132,   133,   134,   135,   136,   137,     0,     0,
       0,     0,     0,     0,   212,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   214,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   215,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   216,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   217,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   218,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   219,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   130,   131,   132,   133,   134,   135,
     136,   137,   130,   131,   132,   133,   134,   135,   136,   137,
       0,     0,   130,   131,   132,   133,   134,   135,   136,   137,
     130,   131,   132,   133,   134,   135,   136,   137,     0,     0,
       0,     0,     0,     0,   130,   131,   132,   133,   134,   135,
     136,   137,     0,     0,   235,   130,   131,   132,   133,   134,
     135,   136,   137,     0,     0,   130,   131,   132,   133,   134,
     135,   136,   137,     0,   279,     0,     0,     0,   222,     0,
       0,   138,   139,   140,   141,   142,   143,   144,   280,   138,
     139,   140,   141,   142,   143,   144,   260,     0,     0,   138,
     139,   140,   141,   142,   143,   144,     0,   138,   139,   140,
     141,   142,   143,   144,     0,     0,     0,     0,     0,     0,
       0,   138,   139,   140,   141,   142,   143,   144,     0,   288,
       0,     0,   138,   139,   140,   141,   142,   143,   144,   289,
       0,     0,   138,   139,   140,   141,   142,   143,   144,   130,
     131,   132,   133,   134,   135,   136,   137,     0,     0,   130,
     131,   132,   133,   134,   135,   136,   137,     0,     0,   130,
     131,   132,   133,   134,   135,   136,   137,     0,     0,   130,
     131,   132,   133,   134,   135,   136,   137,     0,     0,   130,
     131,   132,   133,   134,   135,   136,   137,     0,     0,   130,
     131,   132,   133,   134,   135,   136,   137,     0,     0,   130,
     131,   132,   133,   134,   135,   136,   137,     0,     0,     0,
       0,     0,     0,   290,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   291,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   292,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   293,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   294,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   295,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   296,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   130,   131,   132,   133,   134,   135,   136,
     137,   130,   131,   132,   133,   134,   135,   136,   137,     0,
       0,   130,   131,   132,   133,   134,   135,   136,   137,   130,
     131,   132,   133,   134,   135,   136,   137,   130,   131,   132,
     133,   134,   135,   136,   137,     0,     0,     0,   321,     0,
       0,     0,     0,     0,     0,     0,   322,   130,   131,   132,
     133,   134,   135,   136,   137,   130,   131,   132,   133,   134,
     135,   136,   137,     0,     0,     0,     0,     0,     0,     0,
     138,   139,   140,   141,   142,   143,   144,     0,   138,   139,
     140,   141,   142,   143,   144,   329,     0,     0,   138,   139,
     140,   141,   142,   143,   144,   338,   138,   139,   140,   141,
     142,   143,   144,   341,   138,   139,   140,   141,   142,   143,
     144,   130,   131,   132,   133,   134,   135,   136,   137,     0,
       0,     0,     0,   342,   138,   139,   140,   141,   142,   143,
     144,   344,   138,   139,   140,   141,   142,   143,   144,   130,
     131,   132,   133,   134,   135,   136,   137,   130,   131,   132,
     133,   134,   135,   136,   137,   130,   131,   132,   133,   134,
     135,   136,   137,   130,   131,   132,   133,   134,   135,   136,
     137,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   345,   138,   139,
     140,   141,   142,   143,   144,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   350,     0,     0,   138,   139,   140,   141,
     142,   143,   144,   367,   138,   139,   140,   141,   142,   143,
     144,   368,   138,   139,   140,   141,   142,   143,   144,     0,
     138,   139,   140,   141,   142,   143,   144
  };

  const short
  genesyspp_parser::yycheck_[] =
  {
       4,    12,    13,    52,    52,    80,    12,    13,    81,    81,
      81,    79,    81,   321,   322,    79,    91,    90,    90,    90,
       0,    90,    79,    79,    79,    79,    75,    75,    79,    79,
      34,    90,    12,    13,    79,     6,     7,     8,     9,    10,
      11,    12,    13,    79,    79,    79,    79,    79,    79,    79,
      54,    55,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    48,    79,
      50,    51,    12,    13,    79,    86,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,    81,
      80,    79,    82,   107,   108,    85,    86,    87,     0,    80,
      79,    91,    83,    84,    85,    86,    87,    88,    89,   123,
     124,   125,    12,    13,   128,    79,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,    90,    81,    83,    84,    85,    86,    87,    81,    53,
      58,    54,    58,    58,    73,    66,    58,    58,    73,    66,
      80,    91,    80,    80,   168,    91,   170,   171,   172,   173,
      66,    66,    66,     3,    90,    52,    90,    52,    91,    35,
       6,     7,     8,     9,    10,    11,    12,    13,    35,    80,
      90,    80,    80,    80,    80,    85,    86,    87,    80,    80,
      80,    80,    80,    -1,    -1,     6,     7,     8,     9,   213,
      80,    12,    13,    -1,    90,    -1,   220,   221,    90,   223,
     224,   225,   226,   227,   228,   229,   230,   231,    90,   233,
      -1,   235,    -1,    -1,    -1,    -1,    -1,   241,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   257,    -1,   259,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,   276,    -1,   278,   279,   280,   281,    -1,    -1,
      -1,   285,    83,    84,    85,    86,    87,    88,    89,    -1,
      -1,    -1,    -1,   297,   298,   299,    -1,    -1,    -1,    -1,
      -1,    -1,   306,    -1,    -1,    -1,    -1,   311,    -1,   313,
      -1,    -1,    -1,    -1,   318,    -1,   320,    -1,    -1,   323,
      -1,    -1,   326,    -1,    -1,    -1,   330,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   346,   347,   348,   349,    -1,    -1,    -1,    -1,
      -1,    -1,   356,    -1,   358,    -1,    -1,    -1,   362,    -1,
     364,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      52,    -1,    -1,    55,    56,    -1,    -1,    59,    60,    61,
      62,    63,    64,    -1,    -1,    67,    68,    69,    70,    71,
      -1,    -1,    74,    75,    76,    -1,    -1,    79,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,     3,     4,
       5,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    52,    -1,    -1,
      55,    56,    57,    -1,    59,    60,    61,    62,    63,    64,
      -1,    -1,    67,    68,    69,    70,    71,    -1,    -1,    74,
      75,    76,    -1,    -1,    79,    -1,    -1,    -1,    -1,    84,
       3,     4,     5,    -1,    -1,    -1,    -1,    -1,    93,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    52,
      -1,    -1,    55,    56,    -1,    -1,    59,    60,    61,    62,
      63,    64,    -1,    -1,    67,    68,    69,    70,    71,    -1,
      -1,    74,    75,    76,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    84,     6,     7,     8,     9,    10,    11,    12,    13,
      93,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,     6,     7,     8,     9,
      10,    11,    12,    13,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    83,    84,    85,    86,    87,    88,
      89,    -1,    91,    83,    84,    85,    86,    87,    88,    89,
      -1,    91,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,     6,     7,     8,     9,    10,    11,    12,    13,    -1,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,     6,     7,     8,     9,
      10,    11,    12,    13,    -1,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      84,    85,    86,    87,    88,    89,    -1,    91,    83,    84,
      85,    86,    87,    88,    89,    -1,    91,    83,    84,    85,
      86,    87,    88,    89,    -1,    91,    83,    84,    85,    86,
      87,    88,    89,    -1,    91,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    83,    84,    85,    86,    87,    88,
      89,    -1,    91,    83,    84,    85,    86,    87,    88,    89,
      -1,    91,    83,    84,    85,    86,    87,    88,    89,    -1,
      91,     6,     7,     8,     9,    10,    11,    12,    13,    -1,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,     6,     7,     8,     9,
      10,    11,    12,    13,    -1,    -1,     6,     7,     8,     9,
      10,    11,    12,    13,    -1,    -1,     6,     7,     8,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    83,    84,
      85,    86,    87,    88,    89,    -1,    91,    83,    84,    85,
      86,    87,    88,    89,    -1,    91,    83,    84,    85,    86,
      87,    88,    89,    -1,    91,    83,    84,    85,    86,    87,
      88,    89,    -1,    91,    83,    84,    85,    86,    87,    88,
      89,    -1,    91,    83,    84,    85,    86,    87,    88,    89,
      80,    91,    -1,    83,    84,    85,    86,    87,    88,    89,
      80,    -1,    -1,    83,    84,    85,    86,    87,    88,    89,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,    80,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,    80,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,    80,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,    80,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,    80,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,    80,    -1,    -1,    83,    84,    85,
      86,    87,    88,    89,     6,     7,     8,     9,    10,    11,
      12,    13,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,     6,     7,     8,     9,    10,    11,
      12,    13,    -1,    -1,    48,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    -1,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    50,    -1,    -1,    -1,    80,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    50,    83,
      84,    85,    86,    87,    88,    89,    80,    -1,    -1,    83,
      84,    85,    86,    87,    88,    89,    -1,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    -1,    80,
      -1,    -1,    83,    84,    85,    86,    87,    88,    89,    80,
      -1,    -1,    83,    84,    85,    86,    87,    88,    89,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,     6,     7,     8,     9,    10,    11,    12,
      13,     6,     7,     8,     9,    10,    11,    12,    13,    -1,
      -1,     6,     7,     8,     9,    10,    11,    12,    13,     6,
       7,     8,     9,    10,    11,    12,    13,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,     6,     7,     8,
       9,    10,    11,    12,    13,     6,     7,     8,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    84,    85,    86,    87,    88,    89,    -1,    83,    84,
      85,    86,    87,    88,    89,    80,    -1,    -1,    83,    84,
      85,    86,    87,    88,    89,    82,    83,    84,    85,    86,
      87,    88,    89,    82,    83,    84,    85,    86,    87,    88,
      89,     6,     7,     8,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    82,    83,    84,    85,    86,    87,    88,    89,     6,
       7,     8,     9,    10,    11,    12,    13,     6,     7,     8,
       9,    10,    11,    12,    13,     6,     7,     8,     9,    10,
      11,    12,    13,     6,     7,     8,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    -1,    -1,    83,    84,    85,    86,
      87,    88,    89,    82,    83,    84,    85,    86,    87,    88,
      89,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      83,    84,    85,    86,    87,    88,    89
  };

  const signed char
  genesyspp_parser::yystos_[] =
  {
       0,     3,     4,     5,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    49,    52,    55,    56,    57,
      59,    60,    61,    62,    63,    64,    67,    68,    69,    70,
      71,    74,    75,    76,    79,    84,    93,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   112,   113,   114,   115,   116,   117,    96,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    96,    52,    75,   113,   114,    81,    90,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    81,    90,    81,    96,    96,    79,     0,
       6,     7,     8,     9,    10,    11,    12,    13,    83,    84,
      85,    86,    87,    88,    89,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,   111,    84,    96,
      81,    81,    90,    90,    96,    96,    53,    54,    58,    58,
      58,    58,    58,    73,    66,    66,    66,    66,    66,    73,
      96,    96,    96,    80,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      80,    80,    80,    91,    80,    80,    80,    80,    80,    80,
      91,    91,    80,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    80,    91,    96,    48,    96,    96,    96,    96,
      82,    91,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    91,    91,    80,    82,    91,    82,    91,
      80,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    82,    91,    82,    91,    50,
      50,    90,    96,    52,     3,    90,    96,    96,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    91,    91,    91,
      96,    96,    96,    96,    96,    82,    91,    80,    91,    96,
      82,    91,    82,    91,    96,    96,    96,    82,    91,    82,
      91,    51,    51,    90,    96,    52,    90,    96,    96,    80,
      91,    96,    96,    52,    75,   116,   116,    96,    82,    80,
      96,    82,    82,    96,    82,    82,    81,    81,    90,    90,
      80,    96,    96,    96,    96,    82,    91,    82,    91,    96,
      96,    82,    91,    82,    91,    96,    96,    82,    82
  };

  const signed char
  genesyspp_parser::yyr1_[] =
  {
       0,    94,    95,    95,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    97,    97,    98,    98,    98,
      98,    98,    98,    98,    98,    99,    99,    99,    99,    99,
     100,   100,   100,   100,   100,   100,   101,   101,   102,   102,
     103,   103,   104,   104,   104,   104,   104,   104,   104,   105,
     105,   105,   105,   105,   105,   106,   106,   107,   107,   108,
     108,   108,   108,   108,   108,   108,   108,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   110,   111,
     111,   112,   113,   113,   113,   113,   114,   114,   114,   114,
     115,   115,   115,   115,   116,   116,   116,   116,   116,   116,
     116,   116,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117
  };

  const signed char
  genesyspp_parser::yyr2_[] =
  {
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     2,     6,     6,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     3,     1,     1,     5,     3,
       8,     8,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     6,     1,     4,     6,
       6,     6,     6,     6,     6,     8,    10,     4,     4,     5,
       3,     1,     1,     4,     6,     8,     1,     4,     6,     8,
       1,     4,     6,     8,     3,     6,     8,    10,     3,     6,
       8,    10,     1,     4,     4,     4,     6,     8,     4,     4,
       4,     4,     4,     4,     4
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
  "\">\"", "\"=\"", "\",\"", "NEG", "\"USER\"", "$accept", "input",
  "expression", "number", "arithmetic", "logical", "relacional", "command",
  "commandIF", "commandFOR", "function", "kernelFunction",
  "elementFunction", "trigonFunction", "mathFunction", "probFunction",
  "userFunction", "listaparm", "illegal", "attribute", "variable",
  "formula", "assigment", "pluginFunction", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  genesyspp_parser::yyrline_[] =
  {
       0,   250,   250,   251,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   269,   273,   280,   281,   285,   286,   287,
     288,   289,   290,   293,   295,   299,   300,   301,   302,   303,
     307,   308,   309,   310,   311,   312,   316,   317,   321,   322,
     327,   328,   332,   333,   334,   335,   336,   337,   338,   342,
     343,   344,   345,   346,   347,   352,   356,   363,   364,   368,
     369,   370,   371,   372,   373,   374,   375,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   395,   400,
     401,   406,   427,   437,   446,   455,   469,   470,   473,   476,
     484,   491,   498,   505,   517,   521,   525,   529,   535,   539,
     543,   547,   558,   561,   563,   564,   571,   576,   584,   585,
     586,   587,   588,   589,   605
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
#line 2867 "../GenesysParser.cpp"

#line 612 "bisonparser.yy"

void
yy::genesyspp_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
