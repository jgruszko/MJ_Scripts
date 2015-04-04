#include <iostream>
#include <stdlib.h>
#include <string>
#include <complex>
#include <vector>
#include "TH1.h"
#include "TH2.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TPad.h"
#include "TChain.h"
#include "MGTEvent.hh"
#include "MGWaveform.hh"
#include "MGWFBaselineRemover.hh"
#include "MGWFFastFourierTransformDefault.hh"
#include "MGTWaveformFT.hh"
#include "MGWFExtremumFinder.hh"
#include "MGTEvent.hh"
#include "GATDataSet.hh"
#include "GATCalibrationMap.hh"

using namespace std;
using namespace CLHEP;

int FTDraw(int start, int end, double channel, double energyCut, double timeCut) 
{
  // gROOT->ProcessLine("#include<complex>");
   int startRun = start;
   int endRun = end;
   int selectedCh = channel;
   int currentRun = startRun-1;
   char ftFileName[300], histFileName[300];

   sprintf(ftFileName, "/global/project/projectdirs/majorana/users/jgruszko/FT/FTWF/%d_to_%d.root", startRun, endRun);
   TFile *ftFile = new TFile(ftFileName);
   if(ftFile->IsZombie()){ 
	cout<<"FT File does not exist for this range."<<endl;
	return 0;
   }
   TTree* FTTree = (TTree*)ftFile->Get("FTTree");
   GATDataSet *data = new GATDataSet(startRun, endRun);
   //TChain* builtChain = data->GetBuiltChain(false);
   TChain* gatChain = const_cast<TChain*>(data->GetGatifiedChain());
   if(/*builtChain == NULL ||*/ !gatChain)
   { 
	cout<<"Gatified TChain does not exist for this range."<<endl;
	return 0;
   }

   char gifFile[300], histName[200], averageName[200], averageTitle[300], integratedTitle[300], title[300], selection[300], axisTitle[200];
   TClonesArray* ftArr = new TClonesArray("MGTWaveformFT");
   double run=0;
   vector<double>* energy = 0;
   vector<double>* timestamp= 0;
   vector<double>* ch= 0;
   
   FTTree->SetBranchAddress("FTWaveArr", &ftArr);
   gatChain->SetBranchAddress("timestamp", &timestamp);
   gatChain->SetBranchAddress("channel", &ch);
   gatChain->SetBranchAddress("energyCal", &energy);   
   gatChain->SetBranchAddress("run", &run);   
   double energy_keV; 
   double time;
   int nentriesFT = FTTree->GetEntries();
   int nentriesGAT = gatChain->GetEntries();
   int nentries;
   if(nentriesFT < nentriesGAT){ nentries = nentriesFT ;}
   else{ nentries = nentriesGAT; }
   nentries = 500;
   
   sprintf(title, "FTWF for Channel %d", selectedCh);
   sprintf(selection, "timestamp > %f && channel == %d && energyCal > %f", timeCut, selectedCh, energyCut);
   
   int chan = 0;
   size_t n = 0;
   int nWF = 0; //wf counter
   
   MGTWaveformFT* ftWave;
   //intitialize everything for the FT waveform samples
   FTTree->GetEntry(0);
   //gatChain->GetEntry(0);

   if( ftArr->At(0)!= 0){ ftWave = (MGTWaveformFT*) ftArr->At(0); } 
   //use the number of entries that pass the cut
   int entriesPass = (int) gatChain->Draw("channel", selection, "goff");
   cout<<"Got "<<entriesPass<< " entries passing the cut"<<endl;
   int totalPlots = floor(entriesPass/100.0)+1;
   int nPlot = 0;
   //entriesPass = 100;
   TCanvas* can[totalPlots];
   TH1D* FTHist[100];
   int first[totalPlots];
   for(int i = 0; i <totalPlots; i++)
   {
   	sprintf(histName, "Ch_%d Plot_%d", selectedCh, i);
	can[i] = new TCanvas(histName,title, 400, 400);
	first[i] = 0;
   }
   for(int i = 0; i < 100; i++){ FTHist[i] = new TH1D(); }

   for(int i = 0; i < nentries; i++)
   {
     if(nWF<entriesPass)
     {
	if(i%10000==0){ cout <<" Checking entry "<< i <<" of "<< nentries<<" i.e. "<<100*(((double)i)/nentries)<< " % finished"<<endl; }
	FTTree->GetEntry(i);
	gatChain->GetEntry(i);
	n = ch->size();
	for(size_t j = 0; j < n; j++)
	{
	   chan = (int) ch->at(j);
	    time = (double)timestamp->at(j); 
   	    energy_keV = (double)energy->at(j);
	    if( ftArr->At(0)!= 0)
	    { 
		ftWave = (MGTWaveformFT*) ftArr->At(j);
		  
	    }
	    else
	    { 
		cout<<"Found 0 FT Waveform at event " <<i <<", waveform "<<j<<endl;
		break; 
	     }    
	    if(chan == selectedCh && energy_keV > energyCut && time > timeCut && ftWave != 0 )
	    {
		nPlot = floor(((double)nWF/entriesPass)*totalPlots);
		ftWave->LoadIntoHist(FTHist[nWF%100], MGTWaveformFT::kPower);
		if(first[nPlot] == 0)
		{
		    if(nPlot > 0)
{
	    sprintf(gifFile, "/global/project/projectdirs/majorana/users/jgruszko/FT/Plots/3D_Power/%d_to_%d_Ch%d_Plot%d.gif", startRun, currentRun, selectedCh, nPlot-1);
	    can[nPlot-1]->Print(gifFile);
   	    TFile *outFile= new TFile(histFileName, "UPDATE");
	    can[nPlot-1]->Write();
	    outFile->Close();
}
		    can[nPlot]->cd();
		    FTHist[nWF%100]->Draw();
		    first[nPlot]++;
		}
		else{ FTHist[nWF%100]->Draw("SAME"); }

		nWF++;
	     }
	  }
	  ftArr->Clear("C");
      }
    }
   sprintf(histFileName, "/global/project/projectdirs/majorana/users/jgruszko/FT/FT_Power/%d_to_%d_Ch%d.root", startRun, currentRun, selectedCh); 
/*    for(int i = 0; i < totalPlots; i++)
    {
	    sprintf(gifFile, "/global/project/projectdirs/majorana/users/jgruszko/FT/Plots/3D_Power/%d_to_%d_Ch%d_Plot%d.gif", startRun, currentRun, selectedCh, i);
	    can[i]->Print(gifFile);
   	    TFile *outFile= new TFile(histFileName, "UPDATE");
	    can[i]->Write();
	    outFile->Close();
    } 
*/	    //outFile->Write();
   return 1;
}

int main(int argc, char* argv[])
{
    if(argc < 4){
        cout << "Usage: " << argv[0] << " 'starting run number' 'ending run number' 'channel' 'energy threshold in keV (optional)' 'timestamp cut in ns (optional)'." << endl;
        return 1;
    }
    int startRun = atoi(argv[1]);
    int endRun;
    double channel, energyCut, timeCut;
    endRun = atoi(argv[2]);
    channel = atoi(argv[3]);
    
   if(argc > 4)
    {
        energyCut = atoi(argv[4]);
        timeCut = atoi(argv[5]);
    }
    else{ 
	energyCut = 0;
	timeCut = 0;
    }
    return FTDraw(startRun, endRun, channel, energyCut, timeCut);
}

