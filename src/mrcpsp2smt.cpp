#include <iostream>
#include <fstream>
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


using namespace std;
using namespace util;

/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	COMPUTE_UB,
	ACTIVITY,
	ENCODING
}



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
	"Solve the Multi-mode Resource-Constrained Project Scheduling Problem (MRCPSP). Default: 0."
	);


	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	MRCPSP * instance = parser::parseMRCPSP(pargs->getArgument(0));

	MRCPSPEncoding * encoding = NULL;
	string s_encoding = pargs->getStringOption(ENCODING);

	if(s_encoding=="smttime")
		encoding = new SMTTimeEncoding(instance,sargs,false);
	else if(s_encoding=="smttask")
		encoding = new SMTTaskEncoding(instance,sargs,false);
	else if(s_encoding=="order")
		encoding = new Order(instance,sargs->getAMOPBEncoding(),pargs->getIntOption(ORDER),false);
	else if(s_encoding=="doubleorder")
		encoding = new DoubleOrder(instance, sargs->getAMOPBEncoding(), false);
	else if(s_encoding=="omtsatpb")
		encoding = new OMTSATPBEncoding(instance);
	else if(s_encoding=="omtsoftpb")
		encoding = new OMTSoftPBEncoding(instance);

	instance->computeExtPrecs();
	instance->computeSteps();


	if(pargs->getBoolOption(COMPUTE_ENERGY_PREC)){
		instance->computeEnergyPrecedences();
		instance->recomputeExtPrecs();
	}

	if(pargs->getBoolOption(REDUCE_NR_DEMAND)){
		instance->reduceNRDemandMin();
	}

	int UB = sargs->getIntOption(UPPER_BOUND);
	int LB = sargs->getIntOption(LOWER_BOUND);
	if(LB==INT_MIN)
		LB=instance->trivialLB();
	if(pargs->getBoolOption(COMPUTE_UB) && sargs->getIntOption(UPPER_BOUND)== INT_MIN){
		vector<int> starts;
		vector<int> modes;
		
		bool sat = getSatModes(sargs,modes,instance);
		if(sat){
			UB = instance->computePSS(starts,modes);
			if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS)){
				cout << "c Solution found by greeey heuristic:" << endl;
				if(sargs->getBoolOption(PRINT_CHECKS))
					BasicController::onNewBoundsProved(LB,UB);
				cout << "v ";
				instance->printSolution(cout, starts, modes);
				cout << endl << endl;
			}
			//If just satisfiability check, and satisfiability detected in preprocess, print SAT and exit
			if(sargs->getStringOption(OPTIMIZER)=="check"){
				BasicController::onProvedSAT();
				return 0;
			}
		}
		else{
			BasicController::onProvedUNSAT();
			return 0;
		}
	}
	

	if(sargs->getBoolOption(OUTPUT_ENCODING)){
		FileEncoder * e = sargs->getFileEncoder(encoding);
		SMTFormula * f = encoding->encode(LB,UB);
		e->createFile(std::cout,f);
		delete e;
		delete f;
	}
	else{

		if(sargs->getBoolOption(PRINT_NOOPTIMAL_SOLUTIONS))
			cout << "c Trying to improve solution with exact solving:" << endl;

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
		
		int opt = opti->minimize(e,LB,UB,sargs->getBoolOption(USE_ASSUMPTIONS),sargs->getBoolOption(NARROW_BOUNDS));

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

