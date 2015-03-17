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

int FTPlot(int start, int end, double channel, double energyCut, double timeCut) 
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

//TFile *builtFile = new TFile("$MJDDATADIR/surfprot/data/built/P3END/OR_run45001822.root");
   //TTree* MGTree = (TTree*)builtFile->Get("MGTree");
   
   //TFile *gatFile = new TFile("$MJDDATADIR/surfprot/data/gatified/P3END/mjd_run45001822.root");
   //TTree* mjdTree = (TTree*)gatFile->Get("mjdTree");
   
	
   char gifFile[300], histName[200], averageName[200], averageTitle[300], integratedTitle[300], title[300], selection[300], axisTitle[200];
   TClonesArray* ftArr = new TClonesArray("MGTWaveformFT");
   std::complex<double> value;
   double power;
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
   //nentries = 5000;
   
   sprintf(histName, "Ch_%d", selectedCh);
   sprintf(averageName, "Ch_%d_Average", selectedCh); 
   sprintf(title, "FT Power vs. Event Number in Channel %d", selectedCh);
   sprintf(averageTitle, "FT Power Average in Channel %d", selectedCh);
   sprintf(integratedTitle, "Integrated FT Power in Channel %d", selectedCh);
   sprintf(selection, "timestamp > %f && channel == %d && energyCal > %f", timeCut, selectedCh, energyCut);
   
   int chan = 0;
   size_t n = 0;
   int nWF = 0; //wf counter
   
   MGTWaveformFT* ftWave;
   //intitialize everything for the FT waveform samples
   FTTree->GetEntry(0);
   //gatChain->GetEntry(0);
   cout<< gatChain->GetEntries()<< " entries in gat chain"<<endl;
cout<<"Got Entry 0 in both trees."<<endl;

   if( ftArr->At(0)!= 0){ ftWave = (MGTWaveformFT*) ftArr->At(0); } 
   size_t dataLength = ftWave->GetDataLength();
   double nyquistFreq = ((ftWave->GetSamplingFrequency()/CLHEP::MHz)*0.5);
   double binWidth = nyquistFreq/dataLength;
   double startFreq = 0-binWidth/2.0;
   double endFreq = nyquistFreq - binWidth/2.0;
   cout<<"Got WF and FT parameters"<<endl;
   //use the number of entries that pass the cut
   int entriesPass = (int) gatChain->Draw("channel", selection, "goff");
   cout<<"Got "<<entriesPass<< " entries passing the cut"<<endl;
   //entriesPass = 100;
  if(entriesPass > 100000)
   { 
	entriesPass = 100000;
	cout<<"Too many entries. Using only the first 100000 waveforms."<< endl;
   }  
   TCanvas* can = new TCanvas("can", title, 1000, 1000);
   can->Divide(2,2);
   TH2D* powerHist = new TH2D(histName, title, (int) (entriesPass), -0.5, entriesPass-0.5, dataLength,startFreq, endFreq);
   TH1D* averagePower = new TH1D();
   averagePower->SetTitle(averageTitle);
   TH1D* integratedPower = new TH1D();
   integratedPower->SetTitle(integratedTitle);
   TLine* line[endRun-startRun];
   size_t runCount = 0;
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
	dataLength = ftWave->GetDataLength();
	    if(chan == selectedCh && energy_keV > energyCut && time > timeCut && ftWave != 0 )
	    {
		if(run != currentRun){
		    currentRun = run;
		    can->cd(1);
		    line[runCount] = new TLine((double)nWF, startFreq, (double)nWF, endFreq);
		    line[runCount]->SetLineColor(6);
		    line[runCount]->SetLineWidth(2);
		    runCount++;
		}
		for(size_t k = 0; k < dataLength; k++)
	    	{
		    if(k == 0 || k == dataLength - 1)
		    { 
		    	power = std::norm(ftWave->At(k)); 
		    }	
		    else
		    { 
		    	power = 2.*std::norm(ftWave->At(k)); 
		    }		
		   	powerHist->SetBinContent(nWF, k, power);
	        }
		nWF++;
	     }
	  }
	  ftArr->Clear("C");
	 }
	}
	sprintf(axisTitle, "Event Number in Channel %d (timestamp > 4E10)", selectedCh);
	powerHist->GetXaxis()->SetTitle(axisTitle);
	powerHist->GetYaxis()->SetTitle("Frequency (MHz)");
	    can->cd(1);
	    gPad->SetLogy();
	    gPad->SetLogz();
	    can->Update();
	    powerHist->Draw("COLZ");
	    for(size_t a = 0; a < runCount; a++){ line[a]->Draw(); }
	    can->cd(2);
	    gPad->SetLogy();
	    can->Update();
	    averagePower=powerHist->ProjectionY();
	    averagePower->Scale(entriesPass);
	    averagePower->GetXaxis()->SetTitle("Frequency (MHz)");
	    averagePower->GetYaxis()->SetTitle("Power");
	    averagePower->Draw();
	    can->cd(4);
	    can->Update();
	    integratedPower=powerHist->ProjectionX();
	    integratedPower->GetXaxis()->SetTitle(axisTitle);
	    integratedPower->GetYaxis()->SetTitle("Integrated Power");
	    integratedPower->Draw();
	    
	    sprintf(gifFile, "/global/project/projectdirs/majorana/users/jgruszko/FT/Plots/3D_Power/%d_to_%d_Ch%d.gif", startRun, currentRun, selectedCh);
	    can->Print(gifFile);

	    
   sprintf(histFileName, "/global/project/projectdirs/majorana/users/jgruszko/FT/FT_Power/%d_to_%d_Ch%d.root", startRun, currentRun, selectedCh); 
   TFile *outFile= new TFile(histFileName, "RECREATE");
	    powerHist->Write("powerHist");
	    for(size_t a = 0; a < runCount; a++){ line[a]->Write("runBoundary"); }
	    averagePower->Write("averagePower");
	    integratedPower->Write("integratedPower");
	    //outFile->Write();
	    outFile->ls(); 
	    outFile->Close();
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
    return FTPlot(startRun, endRun, channel, energyCut, timeCut);
}

