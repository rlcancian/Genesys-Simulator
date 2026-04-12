/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ParserDefaultImpl2.h
 * Author: Joao Ortigara (20180801-20181207) rafael.luiz.cancian (20181208-...(
 *
 * 
 */

#ifndef PARSERDEFAULTIMPL2_H
#define PARSERDEFAULTIMPL2_H

#include <string>
#include "Parser_if.h"
#include "Model.h"
#include "../../parser/Genesys++-driver.h"


//namespace GenesysKernel {

class ParserDefaultImpl2 : public Parser_if {
public:
	/*!
	 * \brief Creates parser wrapper bound to a model and an initial sampler.
	 *
	 * The constructor stores \p sampler in the internal driver and marks it as
	 * parser-owned when non-null. This preserves current model behavior where the
	 * parser starts with an internally managed sampler instance.
	 */
	ParserDefaultImpl2(Model* model, Sampler_if* sampler, bool throws = false);
	/*!
	 * \brief Destroys parser wrapper and releases only parser-owned sampler.
	 *
	 * The destructor deletes the current sampler only when ownership is flagged as
	 * internal. Samplers installed through setSampler(...) are non-owned by default
	 * and therefore not deleted at destruction time.
	 */
	virtual ~ParserDefaultImpl2();
public:
	virtual double parse(const std::string expression) override; // may throw exception
    virtual double parse(const std::string expression, bool& success, std::string& errorMessage) override;
    virtual std::string getErrorMessage() override;
	/*!
	 * \brief Replaces parser sampler with an externally-owned sampler.
	 *
	 * If the parser currently owns a previous sampler, that previous sampler is
	 * released before binding the new pointer. The provided sampler pointer is then
	 * tracked as non-owned.
	 */
	virtual void setSampler(Sampler_if* _sampler) override;
	virtual Sampler_if* getSampler() const override;
	virtual genesyspp_driver getParser() const override;
private:
	void _setSamplerInternal(Sampler_if* sampler, bool ownsSampler);
private:
	Model* _model;
	genesyspp_driver _wrapper;
	bool _ownsSampler = false;
};
//namespace\\}
#endif /* PARSERDEFAULTIMPL2_H */
