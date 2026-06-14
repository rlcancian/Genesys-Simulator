#ifndef WCMPATHWAYSTRESSWHOLECELLMVP_H
#define WCMPATHWAYSTRESSWHOLECELLMVP_H

#include "../../../BaseGenesysTerminalApplication.h"

/*!
 * \brief Whole-cell stress example driven by sustained low pathway activity.
 *
 * This example keeps metabolism active but configures a monitored pathway
 * threshold above the achievable FBA objective. The result is an arrest-then-
 * death sequence driven by PathwayStressResponseComponent.
 */
class WcmPathwayStressWholeCellMvp : public BaseGenesysTerminalApplication {
public:
	WcmPathwayStressWholeCellMvp();
	virtual int main(int argc, char** argv) override;
};

#endif /* WCMPATHWAYSTRESSWHOLECELLMVP_H */
