#include "task.h"
#include "util.h"
#include "mrcpsp.h"
#include <limits.h>

using namespace std;

Task::Task(MRCPSP * instance, AMOPBEncoding amopbenc) : MRCPSPEncoding(instance)  {
	this->maxsat = maxsat;
	this->amopbenc = amopbenc;
}

SMTFormula * Task::encode(int lb, int ub){
   int N = ins->getNActivities();
   int N_R = ins->getNResources();

   SMTFormula * f = new SMTFormula();

   /*
   Aixo esta definit a smtapi/src/smtapi.h
   f->newIntVar();
   intsum s = 3 * f->newIntVar();
   s += 4 * f->newIntVar();

   f->addClause(s <10);
    */

  //Order encoding of the start times
	for(int i=0;i<N+2; i++){
	   int ESi=ins->ES(i);
      int LSi=ins->LS(i,ub);
      int ECi=ins->EC(i);
      int LCi=ins->LC(i,ub);
      if(ESi > LSi){
        f->addEmptyClause();
        return f;
      }
      //s_i,t = Activitat i ja ha començat al temps t
	   for(int t=ESi; t <= LSi; t++){
			boolvar o = f->newBoolVar("s",i,t); //Create variable s_{i,t}
			if(t>ESi)
				f->addClause(!f->bvar("o",i,t-1) | o); // o_{i,t-i} -> o_{i,t}
		}
      f->addClause(f->bvar("o",i,LSi)); //It has started for sure at the latest start time
	}
   f->addClause(f->bvar("o",0,0)); //S_0 = 0

   //REFER
   //isPred (i es precedent de j) aprox per el reachable, el graf de precedencies extès
   // Variables z_{i,j} i definició

	for (int i = 1; i <= N; ++i) {
    	for (int j = 1; j <= N; ++j) {
    	   	if (!isPred(i,j)) {
    	    	f->newBoolVar("z", i, j);
    	    	int dur = ins->getDuration(i, 0);
    	    	boolvar z = f->bvar("z", i, j);
    	    	clause c=z;
    	    	for (int t = ins->ES(j); t <= ins->LS(j, ub); t++) {
    	        	f->newBoolVar("a1", t);
    	        	f->newBoolVar("a2", t);
			   		boolvar a1 = f->bvar("a1", t);
			   		boolvar a2 = f->bvar("a2", t);
    	        	boolvar si;
    	        	if (t < ins->ES(i)) si= f->falseVar();
    	        	else if(t > ins->LS(i, ub)) si= f->trueVar() ;
    	        	else f->bvar("s", i, t);
    	        	boolvar sj = f->bvar("s", j, t);
    	        	boolvar siprev;
    	        	if(t - dur + 1 < ins->ES(i)) siprev= f->falseVar();
    	        	else if(t - dur + 1 > ins->LS(i, ub)) siprev= f->trueVar();
    	        	else siprev=f->bvar("s", i, t - dur + 1);
			   		//z <-> &t ( (¬s(j,t) | s(i,t)) & (s(j,t) | ¬s(i,t-p(i)+1)) )
    	        	//a1 <-> !j v i
			   		f->addClause( !a1 | !sj | si);
			   		f->addClause( sj | a1);
			   		f->addClause( !si | a1);
    	        	//a2 <-> j v !p
			   		f->addClause( !a2 | sj | !siprev);
			   		f->addClause( !sj | a2);
			   		f->addClause( siprev | a2);			   
			
    	        	f->addClause( a1 | !z); // z -> (¬s(j,t) | s(i,t)) == a1
    	        	f->addClause( a2 | !z); // z -> (s(j,t) | ¬s(i,t-p(i)+1))  == a2
    	        	// &t a==(¬s(j,t) | s(i,t)) & (s(j,t) | ¬s(i,t-p(i)+1)) -> z
    	        	c|=a1;
			   		c|=a2;
    	      	}
    	  	}
      	}
	}
	
	//Precedences
	for(int i=0; i< N+2; i++){
		int ESi=ins->ES(i);
		int LSi=ins->LS(i,ub);
		for(int j : ins->getSuccessors(i)){
			int ESj=ins->ES(j);
			int LSj=ins->LS(j,ub);
         int dur = ins->getDuration(i,0);
         int mint = min(ESi,ESj-dur);
         int maxt = max(LSj-dur,LSi);
         for(int t=mint; t <= maxt; t++){
            boolvar si;
            boolvar sj;
            if(t < ESi)
               si = f->falseVar();
            else if(t > LSi)
               si = f->trueVar();
            else
               si=f->bvar("s",i,t);   // i starts at t or earlier
            if(t+dur<ESj)
               sj=f->falseVar();
            else if(t +dur > LSj)
               sj=f->trueVar();
            else
               sj=f->bvar("s",j,t+dur);  //  j starts at t plus dur or earlier
            f->addClause(si | !sj);
         }
		}
	}

  //Renewable resource constraints
	for (int r=0;r<N_R;r++) {
		for (int t=0;t<ub;t++) {
            char aux[50];
            sprintf(aux,"R_%d_%d",r,t);
            f->setAuxBoolvarPref(aux);
            
			//Encode resource constraints using PB constraints
			if(usepb){
				vector<literal> X;
				vector<int> Q;

				for (int i=1;i<=N;i++) {
					int ESi=ins->ES(i);
					int LCi=ins->LC(i,ub);
					if (t>=ESi && t<LCi){
							int q=ins->getDemand(i,r,0);
							if (q!=0) {
								boolvar x = f->bvar("x",i,t);
								X.push_back(x);
								Q.push_back(q);
							}
					
					}
				}
				if (!X.empty()){
          //S'ha de parametritzar si ho volem fer o no
					util::sortCoefsDecreasing(Q,X);
					f->addPB(Q,X,ins->getCapacity(r),pbenc);
				}
			}

			//Encode resource constraints using AMO-PB constraints
			else{
				vector<vector<literal> > X;
				vector<vector<int> > Q;

				vector<int> vtasques;
				vector<set<int> > groups;

				for (int i=1;i<=N;i++) {
					int ESi=ins->ES(i);
					int LCi=ins->LC(i,ub);
					if (t>=ESi && t<LCi)
						vtasques.push_back(i);
				}

				if(!vtasques.empty())
					ins->computeMinPathCover(vtasques,groups);

				for (const set<int> & group : groups) {
                    //std::cout << "c group_"  << r << "_" << t << ":";
					vector<literal> vars_part;
					vector<int> coefs_part;
					bool printed = false;
					for(int i : group){
							int auxir=ins->getDemand(i,r,0);
							if (auxir!=0) {                               
								boolvar x = f->bvar("x",i,t);
								vars_part.push_back(x);
								coefs_part.push_back(auxir);

							}
						
					}
					if(!coefs_part.empty()){
						X.push_back(vars_part);
						Q.push_back(coefs_part);
					}
				}
				if (!X.empty()){
					util::sortCoefsDecreasing(Q,X);
					f->addAMOPB(Q,X,ins->getCapacity(r),amopbenc);
				}
			}
		}
	}
	f->setDefaultAuxBoolvarPref();
	return f;
}


void Task::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();

	this->modes=vector<int>(N+2);
   this->starts=vector<int>(N+2);
	this->modes[0] = 0;
	this->modes[N+1] = 0;

	for (int i=0;i<=N+1;i++) {
		for (int p=0;p<ins->getNModes(i);p++){
			if(SMTFormula::getBValue(ef.f->bvar("sm",i,p),bmodel)){
				this->modes[i]=p;
            break;
         }
		}

    int ESi=ins->ES(i);
		int LSi=ins->LS(i,ub);

      for(int t = ESi; t <= LSi; t++){
         if(SMTFormula::getBValue(ef.f->bvar("o",i,t),bmodel)){
            this->starts[i]=t;
            break;
         }
      }
	}
}

bool Task::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->bvar("o",N+1,ub));
		for(int i = 1; i <= N; i++)
			for(int t = ins->LC(i,ub); t < ins->LC(i,lastUB); t++)
				for(int g = 0; g < ins->getNModes(i); g++)
					ef.f->addClause(!ef.f->bvar("x",i,t,g));

		return true;
	}
	else return false;
}

Task::~Task() {
}
