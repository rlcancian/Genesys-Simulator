/*
 * File:   GroProgram.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAM_H
#define GROPROGRAM_H

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/Plugin.h"
#include <string>

/*!
 * \brief Reusable Gro source program attached to a GenESyS model.
 *
 * GroProgram is intentionally a ModelDataDefinition so several biological
 * components can share the same Gro source text. This first implementation only
 * stores the source and performs a small lexical sanity check; complete Gro
 * parsing and executable semantics belong to later plugin-side helper classes.
 */
class GroProgram : public ModelDataDefinition {
public:
	GroProgram(Model* model, std::string name = "");
	virtual ~GroProgram() override = default;

public:
	virtual std::string show() override;

public: // static plugin interface
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	/*! \brief Replaces the stored Gro source text. */
	void setSourceCode(std::string sourceCode);
	/*! \brief Replaces the stored Gro source text through the property-editor wrapper type. */
	void setSourceCodeProperty(SourceCodeString sourceCode);
	/*! \brief Returns the stored Gro source text. */
	std::string getSourceCode() const;
	/*! \brief Returns the stored Gro source text wrapped for the property editor. */
	SourceCodeString getSourceCodeProperty() const;
	/*! \brief Creates a small starter Gro program and optionally writes it to a file. */
	bool createDefaultGroProgram(const std::string& filename = "");
	/*! \brief Performs a permissive lexical sanity check for the stored source. */
	bool validateSyntax(std::string& errorMessage) const;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
	bool _validateSourceCodeSyntax(const std::string& sourceCode, std::string& errorMessage) const;

	private:
	const struct DEFAULT_VALUES {
		const std::string sourceCode = R"(
include gro
set ( "dt", 0.1 );
set ( "population_max", 2000 );
program p() := {
  skip();
};
program q() := {
  set ( "ecoli_growth_rate", 0.1 );
  set ( "ecoli_division_size_mean", 2.0 );
  set ( "ecoli_division_size_variance", 0.2 );
};
program r() := {
  selected : { message ( 1, tostring ( volume ) ) }
};
program s() := {
  set ( "ecoli_division_size_mean", 1000 ); // essentially turns off gro's division machinery
  rate(1) & volume > 3.14 : {
    divide()
  }
};
ecoli ( [ x := 0, y := 0 ], program q() ); // Try p(), q(), r() or s() here. They are all a bit different.

//----------------
set ( "dt", 0.1 );
set ( "ecoli_growth_rate", 0.1 );

s0 := signal ( 1, 0.2 ); // create two new signals
s1 := signal ( 1, 0.2 );

// These functions provide various growth rates in different states. a and b
// are the intensities of signaling molecules.
fun on a b .  0.1;
fun off a b . 0.0;
fun and a b . if a > 0.1 & b > 0.1 then 0.1 else 0.0 end;

//
// The state program. Parameters are
//
//   this: the state index
//   m_next, d_next: states to go to after division if mother or daughter cell
//   t_next: state to go to after timer is up
//   tf: timer length (use -1 for infinity)
//   gr: growth rate function
//   sigs: rates to emit signals
//
program state ( this, m_next, d_next, t_next, tf, gr, sigs ) := {

  needs q, t;
  active := false;
  needs event;

  true : { active := ( q = this ); }

  !event & active & just_divided & !daughter & q != m_next : { q := m_next, t := 0, event := true }
  !event & active & just_divided & daughter  & q != d_next:  { q := d_next, t := 0, event := true }
  !event & active & tf > 0 & t > tf & q != t_next :          { q := t_next, t := 0, event := true }

  active : {
    set ( "ecoli_growth_rate", gr ( get_signal ( 0 ) ) ( get_signal ( 1 ) ) ),
    emit_signal ( 0, sigs[0] ),
    emit_signal ( 1, sigs[1] )
  }

};

//
// State machine program: coordinates states.
//
program sm() := {

  q := 0;
  t := 0;
  event := false;

  true : { t := t + dt, event := false }

};

//
// The morphogenesis state machine example.
//
program p() := sm() + state ( 0, 1, 2, 0, -1, on, { 0, 0 } )  sharing q, t, event
                    + state ( 1, 1, 3, 5, 60, on, { 50, 0 } ) sharing q, t, event
                    + state ( 2, 2, 3, 6, 60, on, { 0, 50 } ) sharing q, t, event
                    + state ( 3, 3, 3, 3, -1, and, { 0, 0 } ) sharing q, t, event
                    + state ( 5, 5, 5, 7, 40, on, { 0, 0 } )  sharing q, t, event
                    + state ( 6, 6, 6, 8, 40, on, { 0, 0 } )  sharing q, t, event
                    + state ( 7, 7, 7, 7,  0, off, { 0, 0 } ) sharing q, t, event
                    + state ( 8, 8, 8, 8,  0, off, { 0, 0 } ) sharing q, t, event;

// A helper function.
fun test x y . if x = y then 1 else 0 end;

//
// Makes cells in different states report different colors.
//
program report() := {

  gfp := 0; rfp := 0; yfp := 0; cfp := 0;
  needs q;

  true : {
    rfp := 100 * test ( q ) 3;
    yfp := 100 * ( test ( q ) 1 + test ( q ) 2 ),
    cfp := 100 * ( test ( q ) 5 + test ( q ) 7 );
    gfp := 100 * ( test ( q ) 6 + test ( q ) 8 );
  }

  selected : { message ( 2, tostring ( { id, q } ) ) }

};

//
// Hides the state variable in all() -- see below -- so that it is not cut in half when the
// cell divides.
//
program hide() := {

  needs gfp;
  needs rfp;
  needs yfp;
  needs cfp;

  skip();

};

//
// All the programs stuck together
//
program all() := p() + report() sharing q, gfp, rfp, yfp, cfp + hide() sharing gfp, rfp, yfp, cfp;

//
// The main program makes the simulation loop
//
program main() := {

  t := 1;

  true : { t := t + dt }

  t > 120 : {
    reset(),
    ecoli ( [], program all() ),
    t := 0
  }

};

// associate the program with a cell
ecoli ( [], program all() );



)";

	} DEFAULT;

	std::string _sourceCode = DEFAULT.sourceCode;
};

#endif /* GROPROGRAM_H */


/*



//
// gro is protected by the UW OPEN SOURCE LICENSE, which is summarized here.
// Please see the file LICENSE.txt for the complete license.
//
// THE SOFTWARE (AS DEFINED BELOW) AND HARDWARE DESIGNS (AS DEFINED BELOW) IS PROVIDED
// UNDER THE TERMS OF THIS OPEN SOURCE LICENSE ("LICENSE").  THE SOFTWARE IS PROTECTED
// BY COPYRIGHT AND/OR OTHER APPLICABLE LAW.  ANY USE OF THIS SOFTWARE OTHER THAN AS
// AUTHORIZED UNDER THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
//
// BY EXERCISING ANY RIGHTS TO THE SOFTWARE AND/OR HARDWARE PROVIDED HERE, YOU ACCEPT AND
// AGREE TO BE BOUND BY THE TERMS OF THIS LICENSE.  TO THE EXTENT THIS LICENSE MAY BE
// CONSIDERED A CONTRACT, THE UNIVERSITY OF WASHINGTON ("UW") GRANTS YOU THE RIGHTS
// CONTAINED HERE IN CONSIDERATION OF YOUR ACCEPTANCE OF SUCH TERMS AND CONDITIONS.
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//

include gro

set ( "dt", 0.1 );
set ( "ecoli_growth_rate", 0.1 );

s0 := signal ( 1, 0.2 ); // create two new signals
s1 := signal ( 1, 0.2 );

// These functions provide various growth rates in different states. a and b
// are the intensities of signaling molecules.
fun on a b .  0.1;
fun off a b . 0.0;
fun and a b . if a > 0.1 & b > 0.1 then 0.1 else 0.0 end;

//
// The state program. Parameters are
//
//   this: the state index
//   m_next, d_next: states to go to after division if mother or daughter cell
//   t_next: state to go to after timer is up
//   tf: timer length (use -1 for infinity)
//   gr: growth rate function
//   sigs: rates to emit signals
//
program state ( this, m_next, d_next, t_next, tf, gr, sigs ) := {

  needs q, t;
  active := false;
  needs event;

  true : { active := ( q = this ); }

  !event & active & just_divided & !daughter & q != m_next : { q := m_next, t := 0, event := true }
  !event & active & just_divided & daughter  & q != d_next:  { q := d_next, t := 0, event := true }
  !event & active & tf > 0 & t > tf & q != t_next :          { q := t_next, t := 0, event := true }

  active : {
    set ( "ecoli_growth_rate", gr ( get_signal ( 0 ) ) ( get_signal ( 1 ) ) ),
    emit_signal ( 0, sigs[0] ),
    emit_signal ( 1, sigs[1] )
  }

};

//
// State machine program: coordinates states.
//
program sm() := {

  q := 0;
  t := 0;
  event := false;

  true : { t := t + dt, event := false }

};

//
// The morphogenesis state machine example.
//
program p() := sm() + state ( 0, 1, 2, 0, -1, on, { 0, 0 } )  sharing q, t, event
                    + state ( 1, 1, 3, 5, 60, on, { 50, 0 } ) sharing q, t, event
                    + state ( 2, 2, 3, 6, 60, on, { 0, 50 } ) sharing q, t, event
                    + state ( 3, 3, 3, 3, -1, and, { 0, 0 } ) sharing q, t, event
                    + state ( 5, 5, 5, 7, 40, on, { 0, 0 } )  sharing q, t, event
                    + state ( 6, 6, 6, 8, 40, on, { 0, 0 } )  sharing q, t, event
                    + state ( 7, 7, 7, 7,  0, off, { 0, 0 } ) sharing q, t, event
                    + state ( 8, 8, 8, 8,  0, off, { 0, 0 } ) sharing q, t, event;

// A helper function.
fun test x y . if x = y then 1 else 0 end;

//
// Makes cells in different states report different colors.
//
program report() := {

  gfp := 0; rfp := 0; yfp := 0; cfp := 0;
  needs q;

  true : {
    rfp := 100 * test ( q ) 3;
    yfp := 100 * ( test ( q ) 1 + test ( q ) 2 ),
    cfp := 100 * ( test ( q ) 5 + test ( q ) 7 );
    gfp := 100 * ( test ( q ) 6 + test ( q ) 8 );
  }

  selected : { message ( 2, tostring ( { id, q } ) ) }

};

//
// Hides the state variable in all() -- see below -- so that it is not cut in half when the
// cell divides.
//
program hide() := {

  needs gfp;
  needs rfp;
  needs yfp;
  needs cfp;

  skip();

};

//
// All the programs stuck together
//
program all() := p() + report() sharing q, gfp, rfp, yfp, cfp + hide() sharing gfp, rfp, yfp, cfp;

//
// The main program makes the simulation loop
//
program main() := {

  t := 1;

  true : { t := t + dt }

  t > 120 : {
    reset(),
    ecoli ( [], program all() ),
    t := 0
  }

};

// associate the program with a cell
ecoli ( [], program all() );
*/

/*
include gro
set ("dt",0.1);
s := signal (1,1.05);
k0 := 2.5;  // base rate of oscillation
kb := 5;    // cell-cell feedback strength
tr := 10;   // refractory period length
se := 650;  // signal emit magnitude
GO := 0;
WAIT := 1;
program oscillator(g0) := {
gfp := 0.5*volume*g0;
p := [mode := GO, t := 0, x := g0];
true : {gfp := 0.5*volume*(g0 + p.x/150)}
// advance the oscillation phase
p.mode = GO & rate (k0) : {p.x := p.x + 0.01*(150-p.x)}
p.mode = GO & rate(kb*get_signal(s)) : {p.x := p.x + 0.01*(150-p.x)}
// phase reset
p.x > 100 : {
p.x := 0,
p.mode := WAIT,
emit_signal(s, se)
}
// refractory period timer
p.mode = WAIT : {p.t := p.t+dt}
p.mode = WAIT & p.t > tr : {
p.mode := GO,
p.t := 0
}
};
ecoli ( [x:=0, y:=0], program oscillator(1));
program main() := {
//n := 0;
//true : {snapshot("movie/oscillator/" <> tostring(n) <> ".tif"), n:=n+1}
skip();
};
*/
