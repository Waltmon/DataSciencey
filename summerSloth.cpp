// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <vector>

using namespace std;

/*class Day {

};*/

class Proj {
	public:
		bool finished;//=false;
		int dayStarted, dayFinished, linesTotal, linesUnfinished;
		float revenueDollars;
};

/*class WorkingProj {
	//for simulating a project being worked on
	public:
		int linesLeft;
};*/

class Year {
	public:
		vector<Proj> p;
		int linesDonePerDay[250], projInitPerDay[250];
		bool summerSlothAllowed;
};

/*void finishProj(Year& year, int dayStarted, int dayFinished, float revenueDollars) {

	/*Proj *project = new Proj;
	project->dayStarted = dayStarted;
	project->dayFinished = dayFinished;
	project->revenueDollars = revenueDollars;
	year.p.push_back(*project);*/

	//return;
//}*/

void loadProj(Year& year, int dayStarted) {//vector<int>& workingProjects) {

	Proj *project = new Proj;
	project->dayStarted = dayStarted;
	//project->dayFinished = dayFinished;
	//project->
	project->linesTotal = int((1./4.)*(rand()%1100+rand()%1100+rand()%1100+rand()%1100)); //makes semi-gaussian distribution around 1500 lines
	//cout << "linestot: " << project->linesTotal << endl;
	project->linesUnfinished = project->linesTotal;
	//just say revenue is a flat fee plus a time multiplier:
	project->revenueDollars = 500. + 0.5*project->linesTotal;
	project->finished=false;

	//um, old:
	//int numLines = int(1./4.)*(rand()%3000+rand()%3000+rand()%3000+rand()%3000); //makes semi-gaussian distribution around 1500 lines
	//workingProjects.push_back(numLines);

	year.p.push_back(*project);

	return;
}

/*void unloadProj(vector<int>& workingProjects, vector<int>::iterator it) {

	workingProjects.erase(it);

	return;
}*/

void simulateData(Year& year, bool summerSloth) {

	if(summerSloth) year.summerSlothAllowed=true;
	else year.summerSlothAllowed=false;

	int nproj, daysTaken, extraLines, lineStock, lineStock_init;
	float rev, pdf_projInit, rando, mainGaus, weightedSmear;

	//simulate a year's worth of projects:
	for (int businessDay = 0; businessDay<250; businessDay++) {
		//determine number of projects initiated on this day according to
		//this probability distribution function which sort of peaks around the summer:
		//v1: pdf_projInit = 0.5*exp(((businessDay - 125.)*(businessDay - 125.)) / (-2.*70.*70.));
		pdf_projInit = 0.5*exp(((businessDay - 125.)*(businessDay - 125.)) / (-2.*50.*50.)); //v3
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

		//lineStock gets probability for summer sloth only ************
		if(summerSloth) lineStock_init = int(500. - 100.*exp(-((businessDay-125)*(businessDay-125))/(2.*100.))); // set this day's line-writing capacity as a constant 500 lines, with a dip around summer time due to people wanting to go outside
		else lineStock_init = 500;
		lineStock = lineStock_init;

		//find unfinished projects and work on them with priority given to the earliest ones initiated:
		for (int i = 0; i<year.p.size(); i++) { //nproj
			//workingProjects[i] -= lineStock;
			//if(workingProjects[i]<0) lineStock = abs(workingProjects[i]);
			//else break;
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

void outputAnalysis(Year& year) {//, bool includeSummerSloth) {

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

	/*outt << year.p.size() << endl;
	for(int im = 0; im<year.p.size(); im++) {
		if(year.p[im].finished) outt << year.p[im].dayStarted << " " << year.p[im].dayFinished - year.p[im].dayStarted << endl;
	}*/

	outt << 25 << endl;
	//bin mean project completion times:
	for(int im = 0; im<year.p.size(); im++) {
		//cout << year.p[im].dayFinished << " " << year.p[im].dayStarted << endl;
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
			//cout << timeSpent[jm] << endl;
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
		//cout << year.projInitPerDay[im] << endl;
		//cout << int(year.projInitPerDay[im] / binWidth) << endl;
		//cout << int((sizeof(year.projInitPerDay)/sizeof(int))/10.) << endl;
		projInitPer10Days[int(im/binWidth)] += year.projInitPerDay[im];
	}
	//////////////////////////////////



	//make data file
	//outt << "daysTakenPer" << binWidth << "Days Error revenuePer" << binWidth << "Days" << endl;
	for (int ih = 0; ih<nbins; ih++) {
		outt << timeSpent[ih] << " " << err[ih] << " " << revPer10Days[ih] << " " << projInitPer10Days[ih] << endl;
	}
	outt << revTotal << endl;
	outt.close();



	/*outt.open("data_revenue.txt");
	float revPer10Days[25];
	for (int seti = 0; seti<25; seti++) {
		revPer10Days[seti] = 0.;
	}

	outt << 25 << endl;
	//bin revenues:
	for(int im = 0; im<year.p.size(); im++) {
		revPer10Days[int(year.p[im].dayStarted / binWidth)] += year.p[im].revenueDollars;
	}
	//make data file
	for (int ih = 0; ih<nbins; ih++) {
		outt << revPer10Days[ih] << " " << 0 << endl;
	}
	outt.close();



	outt.open("data_lines.txt");
	int linesPer10Days[25];
	for (int seti = 0; seti<25; seti++) {
		linesPer10Days[seti] = 0;
	}

	outt << 25 << endl;
	//bin lines:
	for(int im = 0; im<sizeof(year.linesDonePerDay)/sizeof(int); im++) {
		linesPer10Days[int(year.linesDonePerDay[im] / binWidth)] += year.linesDonePerDay[im];
	}
	//make data file
	for (int ih = 0; ih<nbins; ih++) {
		outt << linesPer10Days[ih] << " " << 0 << endl;
	}
	outt.close();*/

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

	//system("PAUSE");

	return 0;
}
