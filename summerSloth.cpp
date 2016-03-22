#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <vector>

using namespace std;

class Proj {
	public:
		bool finished;
		int dayStarted, dayFinished, linesTotal, linesUnfinished;
		float revenueDollars;
};

class Year {
	public:
		vector<Proj> p;
		int linesDonePerDay[250], projInitPerDay[250];
		bool summerSlothAllowed;
};

void loadProj(Year& year, int dayStarted) {

	Proj *project = new Proj;
	project->dayStarted = dayStarted;

	//make semi-gaussian distribution around 550 lines, defining project length:
	project->linesTotal = int((1./4.)*(rand()%1100+rand()%1100+rand()%1100+rand()%1100));

	project->linesUnfinished = project->linesTotal;
	//just say revenue is a flat fee plus a time multiplier:
	project->revenueDollars = 500. + 0.5*project->linesTotal;
	project->finished=false;

	year.p.push_back(*project);

	return;
}

void simulateData(Year& year, bool summerSloth) {

	if(summerSloth) year.summerSlothAllowed=true;
	else year.summerSlothAllowed=false;

	int lineStock, lineStock_init;
	float pdf_projInit, rando;

	//simulate a year's worth of projects:
	for (int businessDay = 0; businessDay<250; businessDay++) {
		//determine number of projects initiated on this day according to
		//this probability distribution function which sort of peaks around the summer:
		pdf_projInit = 0.5*exp(((businessDay - 125.)*(businessDay - 125.)) / (-2.*50.*50.));
		rando = 0.001*(rand() % 1000);

		year.projInitPerDay[businessDay]=0;
		//1st order probability for 1 project, etc.:
		if (rando < pdf_projInit) {
			loadProj(year, businessDay);
			year.projInitPerDay[businessDay]++;
		}
		if (rando < pdf_projInit*pdf_projInit) {
			loadProj(year, businessDay);
			year.projInitPerDay[businessDay]++;
		}
		if (rando < pdf_projInit*pdf_projInit*pdf_projInit) {
			loadProj(year, businessDay);
			year.projInitPerDay[businessDay]++;
		}
		//ignore 4th order and beyond

		if(summerSloth) lineStock_init = int(500. - 100.*exp(-((businessDay-125)*(businessDay-125))/(2.*100.))); // set this day's line-writing capacity as a constant 500 lines, with a dip around summer time due to people wanting to go outside
		else lineStock_init = 500;
		lineStock = lineStock_init;

		//find unfinished projects and work on them with priority given to the earliest ones initiated:
		for (int i = 0; i<year.p.size(); i++) {
			if(!year.p[i].finished) {
				year.p[i].linesUnfinished -= lineStock;
				if(year.p[i].linesUnfinished < 0) {
					lineStock = abs(year.p[i].linesUnfinished);
					year.p[i].linesUnfinished = 0;
					year.p[i].finished=true;
					year.p[i].dayFinished=businessDay;
				}
				else break; //done for the day
			}
		}
		year.linesDonePerDay[businessDay] = lineStock_init - lineStock;
	}
	return;
}

void outputAnalysis(Year& year) {

	ofstream outt;
	if(year.summerSlothAllowed) outt.open("data.txt");
	else outt.open("data_noSummerSloth.txt");

	const float binWidth = 10.;
	const int nbins = int(ceil(250 / binWidth));
	float timeSpent[25], err[25], revPer10Days[25], revTotal=0.;
	int nentries[25];
	int projInitPer10Days[25];
	for (int seti = 0; seti<25; seti++) {
		timeSpent[seti] = 0.;
		err[seti] = 0.;
		nentries[seti] = 0;
		revPer10Days[seti] = 0.;
		projInitPer10Days[seti] = 0;
	}

	//// bin mean project completion times, as well as revenue ////
	for(int im = 0; im<year.p.size(); im++) {
		if(year.p[im].finished) {
			timeSpent[int(year.p[im].dayStarted / binWidth)] += (year.p[im].dayFinished - year.p[im].dayStarted);
			nentries[int(year.p[im].dayStarted / binWidth)]++;
		}
		revPer10Days[int(year.p[im].dayStarted / binWidth)] += year.p[im].revenueDollars;
		revTotal += year.p[im].revenueDollars;
	}

	for(int jm = 0; jm<nbins; jm++) {
		if(nentries[jm] != 0) {
			timeSpent[jm] = timeSpent[jm] / nentries[jm];
		}
	}
	//get standard errors:
	for(int ie = 0; ie<year.p.size(); ie++) {
		err[int(year.p[ie].dayStarted / binWidth)] += (timeSpent[int(year.p[ie].dayStarted / binWidth)] - (year.p[ie].dayFinished - year.p[ie].dayStarted))*(timeSpent[int(year.p[ie].dayStarted / binWidth)] - (year.p[ie].dayFinished - year.p[ie].dayStarted));
	}
	for(int je = 0; je<nbins; je++) {
		if (nentries[je] != 0) err[je] = sqrt(err[je] / (nentries[je] * nentries[je]));
		else err[je] = 0.;
	}

	///// bin projects initiated /////
	for(int im = 0; im<sizeof(year.projInitPerDay)/sizeof(int); im++) {
		projInitPer10Days[int(im/binWidth)] += year.projInitPerDay[im];
	}

	//make data file
		outt << "DaysTakenPerProj_averaged10Days StandardErrorInBin_days Revenue_binned10Days ProjectsInitiated_binned10Days" << endl;
	for (int ih = 0; ih<nbins; ih++) {
		outt << timeSpent[ih] << " " << err[ih] << " " << revPer10Days[ih] << " " << projInitPer10Days[ih] << endl;
	}
	outt << "Revenue_Total_dollars: " << revTotal << endl;
	outt.close();

	return;
}

int main() {

	//simulate 2014 with summer sloth
	Year year2014;
	simulateData(year2014, true);
	outputAnalysis(year2014);

	//simulate 2015 with some kind of program that prevents summer sloth:
	Year year2015;
	simulateData(year2015, false);
	outputAnalysis(year2015);

	return 0;
}
