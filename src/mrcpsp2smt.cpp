#include "parser.h"
#include "errors.h"
#include "encoder.h"
#include "solvingarguments.h"
#include "basiccontroller.h"
#include "mrcpspeventhandler.h"
#include "mrcpsp.h"
#include "mrcpspencoding.h"
#include "smttimeencoding.h"
#include "smttaskencoding.h"
#include "order.h"
#include "doubleorder.h"
#include "omtsatpbencoding.h"
#include "omtsoftpbencoding.h"
#include "mrcpspsatencoding.h"


/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	COMPUTE_UB,
	ENCODING
};


int main(int argc, char **argv) {

	Arguments<ProgramArg> * pargs
	= new Arguments<ProgramArg>(

	//Program arguments
	{
	arguments::arg("filename","Instance file name.")
	},
	1,

	//==========Program options===========
	{

	//Problem specific preprocessing
	arguments::bop("U","upper",COMPUTE_UB,true,
	"If 1, compute a better upper bound than the trivial one using a greedy heuristic. If an upper bound is specified with -u, upper is set to 0. Default: 1."),
	//Encoding parameters
	arguments::sop("E","encoding",ENCODING,"smttime",
	{"smttime","smttask","omtsatpb","omtsoftpb","order","doubleorder"},
	"Encoding of the problem. SMT-based require an SMT solver. Default: smttime.")
	},
	"Solve the Multi-mode Resource-Constrained Project Scheduling Problem (MRCPSP)."
	);


	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	MRCPSP * instance = parser::parseMRCPSP(pargs->getArgument(0));

	MRCPSPEncoding * encoding = NULL;
	string s_encoding = pargs->getStringOption(ENCODING);

	if(s_encoding=="smttime")
		encoding = new SMTTimeEncoding(instance,sargs,false);
	else if(s_encoding=="smttask")
		encoding = new SMTTaskEncoding(instance,sargs,false);
	else if(s_encoding=="doubleorder")
		encoding = new DoubleOrder(instance, sargs->getAMOPBEncoding(), false);
	else if(s_encoding=="omtsatpb")
		encoding = new OMTSATPBEncoding(instance);
	else if(s_encoding=="omtsoftpb")
		encoding = new OMTSoftPBEncoding(instance);

	int UB = sargs->getIntOption(UPPER_BOUND);

	if(sargs->getBoolOption(OUTPUT_ENCODING)){
		FileEncoder * e = sargs->getFileEncoder(encoding);
		SMTFormula * f = encoding->encode(0,UB);
		e->createFile(std::cout,f);
		delete e;
		delete f;
	}
	else{

		Optimizer * opti = sargs->getOptimizer();
		Encoder * e = sargs->getEncoder(encoding);

		if(sargs->getBoolOption(PRINT_CHECKS_STATISTICS)){
			opti->setAfterSatisfiabilityCall([=](int lb, int ub, Encoder * encoder){BasicController::afterSatisfiabilityCall(lb,ub,encoder);});
			opti->setAfterNativeOptimizationCall([=](int lb, int ub, Encoder * encoder){BasicController::afterNativeOptimizationCall(lb,ub,encoder);});
		}

		if(sargs->getBoolOption(PRINT_CHECKS))
			opti->setOnNewBoundsProved([=](int lb, int ub){BasicController::onNewBoundsProved(lb,ub);});
		
		if(sargs->getBoolOption(PRODUCE_MODELS) && sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS))
			opti->setOnSATSolutionFound([=](int & lb, int & ub, int & obj_val){BasicController::onSATSolutionFound(lb,ub,obj_val,encoding);});
		
		UB--; //Solution for UB already found, start with next value
		
		int opt = opti->minimize(e,0,UB,sargs->getBoolOption(USE_ASSUMPTIONS),sargs->getBoolOption(NARROW_BOUNDS));

		if(opt==INT_MIN) //If no better solution found than the one found in the greedy heuristic, that is the objective
			opt = UB+1;

		BasicController::onProvedOptimum(opt);

		delete opti;
		delete e;

	}

	delete instance;
	delete pargs;
	delete sargs;
	delete encoding;

	return 0;
}

