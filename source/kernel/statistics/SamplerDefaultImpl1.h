/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SamplerDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 2 de Agosto de 2018, 01:10
 */

#ifndef SAMPLERDEFAULTIMPL1_H
#define SAMPLERDEFAULTIMPL1_H

#include "Sampler_if.h"
#include <stdint.h>

//namespace GenesysKernel {

class SamplerDefaultImpl1 : public Sampler_if {
public:

	struct DefaultImpl1RNG_Parameters : public RNG_Parameters {
		uint32_t seed = 16021974;
		uint32_t a = 279470273u; // multiplier
		uint32_t m = 0xfffffffb; // module
		uint32_t c = 0; // increment
		~DefaultImpl1RNG_Parameters() = default;
	};
public:
	SamplerDefaultImpl1();
	virtual ~SamplerDefaultImpl1();
public: // RNG
	virtual double random() override;
public: // continuous probability distributions
	virtual double sampleBeta(double alpha, double beta, double infLimit, double supLimit) override;
	virtual double sampleBeta(double alpha, double beta) override;
	virtual double sampleErlang(double mean, int M, double offset = 0.0) override;
	virtual double sampleExponential(double mean, double offset = 0.0) override;
	//virtual double sampleGamma(double mean, double alpha, double offset=0.0);
	virtual double sampleGamma(double alpha, double beta, double offset = 0.0) override;
	virtual double sampleGumbell(double mode, double scale) override;
	virtual double sampleLogNormal(double mean, double stddev, double offset = 0.0) override;
	virtual double sampleNormal(double mean, double stddev) override;
	virtual double sampleTriangular(double min, double mode, double max) override;
	virtual double sampleUniform(double min, double max) override;
	virtual double sampleWeibull(double alpha, double scale) override;
public: // discrete probability distributions
	//TODO: Poisson, si vous plait!!!!
	virtual double sampleBinomial(int trials, double p) override;
	virtual double sampleBernoulli(double p) override;
	virtual double sampleDiscrete(double prob, double value, ...) override;
	virtual double sampleDiscrete(double *prob, double *value, int size) override;
	virtual double sampleGeometric(double p) override;
public:
	void reset(); //!< reinitialize seed and other parameters so (pseudo) random number sequence will be generated again.
public:
	virtual void setRNGparameters(RNG_Parameters* param) override;
	virtual RNG_Parameters* getRNGparameters() const override;
private:
	RNG_Parameters* _param = new DefaultImpl1RNG_Parameters();
	bool _ownsParam = true;
	uint32_t _xi;
	bool _normalflag;
	double _lastnormal;
};


/*
 
 (*                           mrand.ras

               Random generator for Borland Delphi 3.0

Copyrigth(c) 1998 Private enterprise - firm SOFTLAND, Ukraine


1.  What is it?

    TMultiRand is a component intended for generating random variates of
    different distributions:

                      Uniform
                      Triangular
                      Exponential
                      Erlang
                      Normal
                      Gamma
                      Beta
                      Weibull
                      Lognormal

2. Licensing.

  a) This software is under Copyrigth(c) 1998 SOFTLAND,Ukraine
  b) This software is for free usage and distribution.
  c) The  conditions of the distribution are
        - remaining the source code, comments and manual without changes
        - supplying the manual
  e) The user assumes all risks associated with this software.
     Softland is not responsible for any loss or damage incured
     during the use of this software.


3. Manual

  The manual for this component is in mrand.pdf

4. Where is it used?

This component is used in the discrete-event simulation system Delsi 1.0,
which is implemented as a set of components for Borland Delphi.

Find more info at  http://www.softland.rovno.ua/delsi.htm

5. What is Softland ?

Softland is Ukrainian software firm.

Our skills:

            - Delhpi, C++
            - SQL
            - Computer simulation of queuing systems
            - Web-design

We are looking for the remote development job in areas of
general client/server applications and computer simulation.
The rate of our service is extremely unexpensive.

Find more info at http://www.softland.rovno.ua

*)

unit Mrand;

interface

uses WinTypes, WinProcs, Classes, Dialogs;

const

  MODULUS = 2147483647;

type

TMRand = class(TComponent)
private
  FSeed: comp;
  FSeedInit: comp;
  FMultiplier: comp;
  NormalFlag: boolean;
  NormalResult: real;
  function Uni01: real;
  procedure SetSeed(Value: comp);
  function GetSeed: comp;
  procedure SetMultiplier(Value: comp);
  function GetMultiplier: comp;
  function GammaJonk(Alpha: real): real;
public
  constructor Create(Owner: TComponent); override;
  procedure Reset;
  function Uniform(LowerBound: real; UpperBound: real): real;
  function Triangular(Min: real; Mode: real; Max: real): real;
  function Exponential(Mean: real): real;
  function Erlang(Mean: real; M: integer): real;
  function Normal(Mean: real; Deviation: real): real;
  function Gamma(Mean: real; Alpha: real): real;
  function Beta(ShapeAlpha: real; ShapeBeta: real;
                LowerBound: real; UpperBound: real): real;
  function Weibull(Alpha: real; Scale: real): real;
  function Lognormal(Mean: real; Deviation: real): real;
published
  property Seed: comp read GetSeed write SetSeed;
  property Multiplier: comp read GetMultiplier write SetMultiplier;
end;


procedure Register;

implementation


constructor TMRand.Create(Owner: TComponent);
begin
   inherited Create(Owner);
   FSeed:=1000000000;
   FMultiplier:=950706376;
   NormalFlag:=True;
end;

procedure TMRand.Reset;
begin
   NormalFlag:=True;
   FSeed:=FSeedInit;
end;


procedure TMRand.SetSeed(Value: comp);
begin
   if (Value<1) or (Value>2147483646) then
   begin
      MessageDlg(Name+'Seed must be in the range 2...2147483646',mtError,[mbOK],0);
      FSeed:=1000000000;
   end
   else
      FSeed:=Value;
   FSeedInit:=FSeed;
end;

function TMRand.GetSeed: comp;
begin
   if (FSeed<1) or (FSeed>2147483646) then
   begin
      FSeed:=1000000000;
      FSeedInit:=FSeed;
   end;
   Result:=FSeed;
end;


procedure TMRand.SetMultiplier(Value: comp);
begin
   if (Value<2) or (Value>2147483646) then
   begin
      MessageDlg(Name+'Multuplier must be in the range 2...2147483646',mtError,[mbOK],0);
      FMultiplier:=950706376;
   end
   else
      FMultiplier:=Value;
end;

function TMRand.GetMultiplier: comp;
begin
   if (FMultiplier<2) or (FMultiplier>2147483646) then
      FMultiplier:=950706376;
   Result:=FMultiplier;
end;

function TMRand.Uni01: real;
begin
   FSeed:=FSeed*FMultiplier;
   FSeed:=FSeed-Int(FSeed/MODULUS)*MODULUS;
   Result:=FSeed/MODULUS;
end;


{  Uniform distribution with the range LowerBound....UpperBound  }

function TMRand.Uniform(LowerBound: real; UpperBound: real): real;
begin
   if (LowerBound<0) or (UpperBound<0) or (UpperBound<LowerBound) then
   begin
      MessageDlg('Incorrect parameters of '+Name+'.Uniform',mtError,[mbOK],0);
      Result:=0.0;
      Exit;
   end;
   Result:=LowerBound+(UpperBound-LowerBound)*Uni01
end;


{  Exponential distribution with mean "Mean"  }

function TMRand.Exponential(Mean: real): real;
begin
   if Mean>=0.0 then
      Result:=Mean*(-ln(Uni01))
   else
      begin
         MessageDlg('Incorrect parameter of '+Name+'.Exponential',mtError,[mbOK],0);
         Result:=0;
      end;
end;


{  M-Erlang distribution with mean "Mean"  }

function TMRand.Erlang(Mean: real; M: integer): real;
var
   i: integer;
   P: real;
begin
   if (Mean<0.0) or (M<=0) then
   begin
      MessageDlg('Incorrect parameters of '+Name+'.Erlang',mtError,[mbOK],0);
      Result:=0.0;
      Exit;
   end;

   P:=1;
   for i:=1 to M do
     P:=P*Uni01;
   Result:=(Mean/M)*(-ln(P));
end;


{  Normal distribution with mean "Mean" and deviation "Deviation"   }

function TMRand.Normal(Mean: real; Deviation: real): real;
var U1,U2: real;
    W,Y: real;
begin
   if (Mean<=0.0) or (Deviation<=0.0) then
   begin
      MessageDlg('Incorrect parameter Deviation of '+Name+'.Normal',mtError,[mbOK],0);
      Result:=0.0;
      Exit;
   end;

   if NormalFlag then
   begin
      repeat
        U1:=2*Uni01-1.0;
        U2:=2*Uni01-1.0;
        W:=U1*U1+U2*U2;
      until not (W>=1.0);
      Y:= sqrt((-2* ln (W)) / W);
      Result:=Mean+U1*Y*Deviation;
      NormalResult:=Mean+U2*Y*Deviation;
      NormalFlag:=False;
   end
   else
   begin
      Result:=NormalResult;
      NormalFlag:=True;
   end;
end;


{ Gamma distribution for Alpha<1 and beta=1 }

function TMRand.GammaJonk(Alpha: real ): real;
var R,R1,R2,X,Y:real;
begin
   repeat
     repeat
       R1:=Uni01;
       R2:=Uni01;
     until ((R1>1e-30) and (R2>1e-30));

     if (ln(R2)/alpha<-1e3) then X:=0 else X:= exp(ln(R2)/alpha);
     if (ln(R1)/(1-alpha)<-1e3) then Y:=0 else Y:=exp(ln(R1)/(1-alpha));

   until (X+Y<=1);

   repeat
     R:=Uni01;
   until (R>1e-20);

   Result:=-ln(R)*X/(Y+X);
end;


{  Gamma distribution with the mean "Mean" and shape-parameter "Alpha" }

function TMRand.Gamma(Mean: real; Alpha: real): real;
   var
   i: integer;
   P: real;
   IntAlpha: integer;
   OstAlpha: real;
begin

   if (Mean<=0.0) or (Alpha<=0.0) then
   begin
      MessageDlg('Incorrect parameters of '+Name+'.Gamma',mtError,[mbOK],0);
      Result:=0.0;
      Exit;
   end;

   if Alpha<1.0 then
      Result:=(Mean/Alpha)*GammaJonk(Alpha)
   else
      if Alpha=1.0 then
         Result:=Mean*(-ln(Uni01))
      else
         begin
            IntAlpha:=Round(Int(Alpha));
            OstAlpha:=Alpha-IntAlpha;
            repeat
               P:=1;
               for i:=1 to IntAlpha do
                  P:=P*Uni01;
            until P>0;
            if OstAlpha>0 then
               Result:=(Mean/Alpha)*((-ln(P))+GammaJonk(OstAlpha))
            else
               Result:=(Mean/Alpha)*(-ln(P));

         end;
end;


{ Beta distribution with alpha-parameter "ShapeAlpha,
                         beta-perameter "ShapeBeta,
                         lower bound "LowerBound and
                         upper bound "UpperBound
}

function TMRand.Beta(ShapeAlpha: real; ShapeBeta: real;
                         LowerBound: real; UpperBound: real): real;
var
   X,Y1,Y2: real;
begin

   if (ShapeAlpha<=0.0) or (ShapeBeta<=0.0) or (LowerBound>UpperBound) or
      (LowerBound<0) or (UpperBound<0)
   then
   begin
      MessageDlg('Incorrect parameters of '+Name+'.Beta',mtError,[mbOK],0);
      Result:=0.0;
      Exit;
   end;

   repeat
     Y1:=Gamma(ShapeAlpha,ShapeAlpha);
     Y2:=Gamma(ShapeBeta,ShapeBeta);
     X:=Y1/(Y1+Y2);
   until (X>=0) and (X<=1.0);
   Result:= LowerBound + (UpperBound - LowerBound ) * X;
end;


{  Weibull distribution with shape-parameter "Alpha" and scale "Scale" }

function TMRand.Weibull(Alpha: real; Scale: real): real;
begin
   if (Alpha<=0.0) or (Scale<=0.0) then
   begin
      MessageDlg('Incorrect parameters of '+Name+'.Weibull',mtError,[mbOK],0);
      Result:=0.0;
      Exit;
   end;
   Result:= exp(ln(Scale*(-ln(Uni01)))/Alpha);
end;


{  Lognormal distribution with mean "Mean" and deviation "Deviation"   }

function TMRand.Lognormal(Mean: real; Deviation: real): real;
var
   MeanNorm,DispNorm: real;
begin

   if (Mean<=0.0) or (Deviation<=0.0) then
   begin
      MessageDlg('Incorrect parameters of '+Name+'.Lognormal',mtError,[mbOK],0);
      Result:=0.0;
      Exit;
   end;

   DispNorm:=ln((Deviation*Deviation)/(Mean*Mean)+1.0);
   MeanNorm:=ln(Mean)-0.5*DispNorm;
   Result:=Exp(Normal(MeanNorm,SQRT(DispNorm)));
end;


{ Triangular distribution with minimal value "Min", maximal - "Max" and mode "Mode"}

function TMRand.Triangular(Min: real; Mode: real; Max: real): real;
var
    Part1,Part2,Full,R: real;
begin

   if (Min>Mode) or (Max<Mode) or (Min>Max) or
      (Min<0) or (Max<0) or (Mode<0) then
   begin
      MessageDlg('Incorrect parameters of '+Name+'.Triangular',mtError,[mbOK],0);
      Result:=0.0;
      Exit;
   end;

   Part1:=Mode-Min;
   Part2:=Max-Mode;
   Full:=Max-Min;
   R:=Uni01;
   if R<=Part1/Full then
      Result:=Min+sqrt(Part1*Full*R)
   else
      Result:=Max-sqrt(Part2*Full*(1.0-R));
end;


procedure Register;
begin
   RegisterComponents('MRand', [TMRand]);
end;


end.
 
 */



//namespace\\}
#endif /* SAMPLERDEFAULTIMPL1_H */
