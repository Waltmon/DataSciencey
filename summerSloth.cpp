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
vector<Proj> p;

void loadProj(int dayStarted, int dayFinished, float revenueDollars) {

     Proj *project = new Proj;
     project->dayStarted = dayStarted;
     project->dayFinished = dayFinished;
     project->revenueDollars = revenueDollars;
     p.push_back(*project);

     return;
}

int main() {
	
	bool smearSummerTime;
	cout << "Simulate summer sloth (1 or 0)? ";
	cin >> smearSummerTime;
	
	ofstream outt;
	if(smearSummerTime) outt.open("data_smear.txt");
	else outt.open("data_noSmear.txt");
	
	int nproj, daysTaken;
	float rev, pdf_projInit, rando, mainGaus, weightedSmear;
	
	//simulate a year's worth of projects:
	for(int businessDay=0; businessDay<250; businessDay++) {
            //determine number of projects initiated on this day according to this probability distribution function which sort of peaks around the summer:
            pdf_projInit = 0.5*exp( ((businessDay-125.)*(businessDay-125.))/(-2.*70.*70.) ); // probability distribution function for whether a project comes in on a given day
            rando = 0.001*(rand()%1000);
            nproj = 0;
            //3rd order probability for 3 projects, etc.:
            if(rando < pdf_projInit*pdf_projInit*pdf_projInit) nproj = 3;
            else if(rando < pdf_projInit*pdf_projInit) nproj = 2;
            else if(rando < pdf_projInit) nproj = 1;
            //ignore 4th order and beyond
            
            for(int i=0; i<nproj; i++) {
                    //make project completion time according to semi-gaussian distribution around 3 weeks (15 business days), w/ statistically longer times taken in nice weather (weightedSmear):
                    mainGaus = (.01/4.)*(rand()%3000+rand()%3000+rand()%3000+rand()%3000);
                    if(smearSummerTime) weightedSmear = exp( ((businessDay-125.)*(businessDay-125.))/(-2.*20.*20.) )*(10.+(.01/4.)*(rand()%3000+rand()%3000+rand()%3000+rand()%3000));
                    else weightedSmear = 0.;
                    daysTaken = int(mainGaus + weightedSmear);

                    //just say revenue is a flat fee plus a time multiplier:
                    rev = 1000. + 100.*daysTaken;
                    
                    loadProj(businessDay, businessDay+daysTaken, rev);
            }
    }


     //  analysis part:

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
     for(int im=0; im<p.size(); im++) {
             timeSpent[int(p[im].dayStarted/binWidth)] += (p[im].dayFinished - p[im].dayStarted);
             nentries[int(p[im].dayStarted/binWidth)]++;
     }

     for(int jm=0; jm<nbins; jm++) {
             if(nentries[jm]!=0) timeSpent[jm] = timeSpent[jm]/nentries[jm];
     }
     //get standard errors:
     for(int ie=0; ie<p.size(); ie++) {
             err[int(p[ie].dayStarted/binWidth)] += (timeSpent[int(p[ie].dayStarted/binWidth)] - (p[ie].dayFinished - p[ie].dayStarted))*(timeSpent[int(p[ie].dayStarted/binWidth)] - (p[ie].dayFinished - p[ie].dayStarted));
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

	 return 0;
}
