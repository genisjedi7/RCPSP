#include "parser.h"
#include "errors.h"
#include "util.h"


using namespace std;

namespace parser
{


MRCPSP * parseMRCPSP(const string & filename) {
  string extension=filename.substr(filename.rfind(".")+1);
  if (extension=="mm" || extension=="MM" || extension=="sm" || extension=="SM")
	  return parser::parseMRCPSPfromMM(filename);
  else if(extension=="mm2" || extension=="MM2")
	  return parser::parseMRCPSPfromMM2(filename);
  else if(extension=="rcp" || extension=="RCP"){
     return parser::parseMRCPSPfromRCP(filename);
  }
  else if(extension=="prb" || extension=="PRB"){
		return parser::parseMRCPSPfromPRB(filename);
  }
  else if(extension=="data" || extension=="DATA"){
		return parser::parseMRCPSPfromDATA(filename);
  }
  else {
	  cerr << "bad input file extension" << endl;
	  exit(BADFILEEXTENSION_ERROR);
	}
}


/* This method parses an RCPSP instance from an rcp format file,
 * and generalizes it to MRCPSP in the obvious way (single modes, no nonrenewable resources)
 * filename: path to the instance file
 */
MRCPSP * parseMRCPSPfromRCP(const string &  filename)
{

	MRCPSP * instance;
	ifstream f;

	int nresources=0;
	int nactivities=0;
	int aux;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nactivities >> nresources;
	nactivities-=2;

	instance = new MRCPSP(nactivities,nresources,0);

	for (int j=0;j<nresources;j++) {
		f >> aux;
		instance->setCapacity(j,aux);
	}

	for(int i=0;i<=nactivities+1;i++){
		instance->setNModes(i,1);
		f >> aux;
		instance->setDuration(i,0,aux);

		for (int j=0;j<nresources;j++) {
			f >> aux;
			instance->setDemand(i,j,0,aux);
		}
		int nsuccessors;
		f >> nsuccessors;
		for (int k=0;k<nsuccessors;k++) {
			f >> aux;
			instance->addSuccessor(i,aux-1);
		}
	}

	f.close();

	return instance;
}


/* This method parses an RCPSP instance from a data format file,
 * and generalizes it to MRCPSP in the obvious way (single modes, no nonrenewable resources)
 * filename: path to the instance file
 */
MRCPSP * parseMRCPSPfromDATA(const string &  filename)
{

	MRCPSP * instance;
	ifstream f;

	int nresources=0;
	int nactivities=0;
	int aux;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nresources;
	vector<int> capacities(nresources);
	for(int i = 0; i < nresources; i++)
		f >> capacities[i];

	f >> nactivities;

	instance = new MRCPSP(nactivities,nresources,0);
	instance->setNModes(0,1);
	instance->setNModes(nactivities+1,1);
	instance->setDuration(0,0,0);
	instance->setDuration(nactivities+1,0,0);

	for (int i=0;i<nresources;i++){
		instance->setCapacity(i,capacities[i]);
		instance->setDemand(0,i,0,0);
		instance->setDemand(nactivities+1,i,0,0);
	}


	for(int i=1;i<=nactivities;i++){
		instance->setNModes(i,1);
		f >> aux;
		instance->setDuration(i,0,aux);
		instance->addSuccessor(0,i);
		instance->addSuccessor(i,nactivities+1);
	}
	for (int j=0;j<nresources;j++) {
		for(int i=1;i<=nactivities;i++){
			f >> aux;
			instance->setDemand(i,j,0,aux);
		}
	}

	instance->addSuccessor(0,nactivities+1);

	string line_s;
	getline(f,line_s);

	for(int i=1;i<=nactivities;i++){
		getline(f,line_s);
		stringstream str(line_s);
		int suc;
		while(str>>suc)
			instance->addSuccessor(i,suc);
	}



	f.close();

	return instance;
}


/* This method parses an RCPSP instance from a prb format file,
 * and generalizes it to MRCPSP in the obvious way (single modes, no nonrenewable resources)
 * filename: path to the instance file
 */
MRCPSP * parseMRCPSPfromPRB(const string &  filename)
{

	MRCPSP * instance;
	ifstream f;

	int nresources=0;
	int nactivities=0;
	int aux;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nactivities >> nresources;

	instance = new MRCPSP(nactivities,nresources,0);

	for (int i=1;i<=nactivities;i++) {
		int npreds, nmodes;
		f >> npreds;
		if(npreds == 0)
			instance->addSuccessor(0,i);
		else{
			for(int j = 0; j < npreds; j++){
				f >> aux;
				instance->addSuccessor(aux,i);
			}
		}
		f >> nmodes;
		instance->setNModes(i,nmodes);
		for(int m = 0; m < nmodes; m++){
			f >> aux;
			instance->setDuration(i,m,aux);
			for(int r = 0; r < nresources; r++){
				f >> aux;
				instance->setDemand(i,r,m,aux);
			}
		}
	}
	for (int j=0;j<nresources;j++) {
		f >> aux;
		instance->setCapacity(j,aux);
	}


	f.close();


	instance->setNModes(0,1);
	instance->setNModes(nactivities+1,1);
	instance->setDuration(0,0,0);
	instance->setDuration(nactivities+1,0,0);
	for (int j=0;j<nresources;j++){
		instance->setDemand(0,j,0,0);
		instance->setDemand(nactivities+1,j,0,0);
	}
	for(int i = 0; i <= nactivities; i++)
		if(instance->getSuccessors(i).empty())
			instance->addSuccessor(i,nactivities+1);


	return instance;
}

MRCPSP * parseMRCPSPfromMM2(const string &  filename)
{
	ifstream f;
	int nresources = 0;
	int nactivities=0;
	int nresourcesnorew=0;

	MRCPSP * instance;

	f.open(filename.c_str());


	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	string line_s;
	vector<int> nmodes;
	char skip='a';
	int aux;

	do f >> skip; while(skip!=':');

	f >> nactivities;
	nactivities -=2;
	nmodes.resize(nactivities+2);
	do f >> skip; while(skip!=':');
	f >> nresources;
	do f >> skip; while(skip!=':');
	f >> nresourcesnorew;

	instance = new MRCPSP(nactivities,nresources,nresourcesnorew);

	for (int i=1;i<=5;i++)
	getline(f,line_s);

	for(int i=0;i<nactivities+2;i++){
		f >> aux;
		f >> aux; nmodes[i]=aux;
		instance->setNModes(i,aux);
		f >> aux; int nsucessors=aux;

		for(int j =0; j < nsucessors; j++){
			f >> aux;
			instance->addSuccessor(i,aux-1);
		}
		getline(f,line_s);
	}

	//Second half of the file

	for (int i=1;i<=4;i++)
		getline(f,line_s);

	int tascaant=0;
	for(int i=0;i<nactivities+2;i++){
		f >> aux;
		for(int j = 0; j < nmodes[i]; j++){
			f >> aux;
			f >> aux;
			instance->setDuration(i,j,aux);
			for(int r = 0; r < nresources + nresourcesnorew; r++){
				f >> aux;
				instance->setDemand(i,r,j,aux);
			}
		}
	}
	for (int i=1;i<=5;i++)
		getline(f,line_s);

	for (int i=0;i<nresources+nresourcesnorew;i++){
		f >> aux;
		instance->setCapacity(i,aux);
	}

	f.close();

	return instance;
}

MRCPSP * parseMRCPSPfromMM(const string &  filename){
	ifstream f;
	int nresources = 0;
	int nactivities=0;
	int nresourcesnorew=0;

	MRCPSP * instance;

	f.open(filename.c_str());


	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	string line_s;
	vector<int> nmodes;
	char skip='a';
	int aux;

	for (int i=1;i<=5;i++)
		getline(f,line_s);

	do f >> skip; while(skip!=':');

	f >> nactivities;
	nactivities -=2;
	nmodes.resize(nactivities+2);
	do f >> skip; while(skip!=':');
	do f >> skip; while(skip!=':');
	f >> nresources;
	do f >> skip; while(skip!=':');
	f >> nresourcesnorew;

	instance = new MRCPSP(nactivities,nresources,nresourcesnorew);

	for (int i=1;i<=9;i++)
		getline(f,line_s);

	for(int i=0;i<nactivities+2;i++){
		f >> aux;
		f >> aux; nmodes[i]=aux;
		instance->setNModes(i,aux);
		f >> aux; int nsucessors=aux;

		for(int j =0; j < nsucessors; j++){
			f >> aux;
			instance->addSuccessor(i,aux-1);
		}
		getline(f,line_s);
	}

	//Second half of the file

	for (int i=1;i<=4;i++)
		getline(f,line_s);

	int tascaant=0;
	for(int i=0;i<nactivities+2;i++){
		f >> aux;
		for(int j = 0; j < nmodes[i]; j++){
			f >> aux;
			f >> aux;
			instance->setDuration(i,j,aux);
			for(int r = 0; r < nresources + nresourcesnorew; r++){
				f >> aux;
				instance->setDemand(i,r,j,aux);
			}
		}
	}
	for (int i=1;i<=4;i++)
		getline(f,line_s);

	for (int i=0;i<nresources+nresourcesnorew;i++){
		f >> aux;
		instance->setCapacity(i,aux);
	}

	f.close();
	return instance;
}

void parseMRCPSPSolution(istream & str,MRCPSP * instance,vector<int> &starts, vector<int> &modes){
	starts.resize(instance->getNActivities()+2);
	for(int i=0;i<instance->getNActivities()+2;i++)
		starts[i] = readAssingnment(str);

	modes.resize(instance->getNActivities()+2);
	for(int i=0;i<instance->getNActivities()+2;i++)
		modes[i] = readAssingnment(str)-1;
}

void findChar(istream & str, char c){
	char aux;
	do{
		str >> aux;
	}while(aux!=c && !str.eof());
}

int readAssingnment(istream & str){
	findChar(str,':');
	int i;
	str >> i;
	return i;
}
}


