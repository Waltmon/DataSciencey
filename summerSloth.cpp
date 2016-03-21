#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <vector>

using namespace std;

class Proj {
	public:
		int dayStarted, dayFinished;
		float revenueDollars;
};

class Year {
	public:
		vector<Proj> p;
};

void loadProj(Year& year, int dayStarted, int dayFinished, float revenueDollars) {

	Proj *project = new Proj;
	project->dayStarted = dayStarted;
	project->dayFinished = dayFinished;
	project->revenueDollars = revenueDollars;
	year.p.push_back(*project);

	return;
}

void simulateData(Year& year, bool includeSummerSloth) {

	int nproj, daysTaken;
	float rev, pdf_projInit, rando, mainGaus, weightedSmear;

	//simulate a year's worth of projects:
	for(int businessDay=0; businessDay<250; businessDay++) {
		//determine number of projects initiated on this day according to
		//this probability distribution function which sort of peaks around the summer:
		pdf_projInit = 0.5*exp( ((businessDay-125.)*(businessDay-125.))/(-2.*70.*70.) );
		rando = 0.001*(rand()%1000);
		nproj = 0;
		//3rd order probability for 3 projects, etc.:
		if(rando < pdf_projInit*pdf_projInit*pdf_projInit) nproj = 3;
		else if(rando < pdf_projInit*pdf_projInit) nproj = 2;
		else if(rando < pdf_projInit) nproj = 1;
		//ignore 4th order and beyond

		for(int i=0; i<nproj; i++) {
			//make project completion time according to semi-gaussian distribution around
			//3 weeks (15 business days), w/ statistically longer times taken in nice weather (weightedSmear):
			mainGaus = (.01/4.)*(rand()%3000+rand()%3000+rand()%3000+rand()%3000);
			if(includeSummerSloth) weightedSmear = exp(((businessDay-125.)*(businessDay-125.))/(-2.*20.*20.))*(10.+(.01/4.)*(rand()%3000+rand()%3000+rand()%3000+rand()%3000));
			else weightedSmear = 0.;
			daysTaken = int(mainGaus + weightedSmear);

			//just say revenue is a flat fee plus a time multiplier:
			rev = 1000. + 100.*daysTaken;

			loadProj(year, businessDay, businessDay+daysTaken, rev);
		}
	}
	return;
}

void outputAnalysis(Year& year, bool includeSummerSloth) {

	ofstream outt;
	if(includeSummerSloth) outt.open("data.txt");
	else outt.open("data_noSummerSloth.txt");

	float binWidth = 10.;
	const int nbins = int(ceil(250/binWidth));
	float timeSpent[nbins], err[nbins];
	int nentries[nbins];
	for(int seti=0; seti<nbins; seti++) {
		timeSpent[seti]=0.;
		err[seti]=0.;
		nentries[seti]=0;
	}

	//bin mean project completion times:
	for(int im=0; im<year.p.size(); im++) {
		timeSpent[int(year.p[im].dayStarted/binWidth)] += (year.p[im].dayFinished - year.p[im].dayStarted);
		nentries[int(year.p[im].dayStarted/binWidth)]++;
	}

	for(int jm=0; jm<nbins; jm++) {
		if(nentries[jm]!=0) timeSpent[jm] = timeSpent[jm]/nentries[jm];
	}
	//get standard errors:
	for(int ie=0; ie<year.p.size(); ie++) {
		err[int(year.p[ie].dayStarted/binWidth)] += (timeSpent[int(year.p[ie].dayStarted/binWidth)] - (year.p[ie].dayFinished - year.p[ie].dayStarted))*(timeSpent[int(year.p[ie].dayStarted/binWidth)] - (year.p[ie].dayFinished - year.p[ie].dayStarted));
	}
	for(int je=0; je<nbins; je++) {
		if(nentries[je]!=0) err[je] = sqrt(err[je]/(nentries[je]*nentries[je]));
		else err[je] =0.;
	}

	//make data file
	outt << "daysPer" << binWidth << "Days Error" << endl;
	for(int ih=0; ih<nbins; ih++) {
		outt << timeSpent[ih] << " " << err[ih] << endl;
	}

	outt.close();

	return;
}

int main() {

	//simulate and analyze a "real" year (with summer sloth)
	Year year_real;
	simulateData(year_real,true);
	outputAnalysis(year_real,true);

	//simulate one without sloth, for comparison:
	Year year_noSloth;
	simulateData(year_noSloth,false);
	outputAnalysis(year_noSloth,false);

	return 0;
}
