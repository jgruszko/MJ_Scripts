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

int FTPower(int start, int end, double firstStart, double firstEnd, double secondStart, double secondEnd)
{
    //gROOT->Reset();
    //gROOT->ProcessLine(".x $MGDODIR/Root/LoadMGDOClasses.C"); 
    //gROOT->ProcessLine(".L $MGDODIR/lib/libMGDOClasses.C"); 
    //gROOT->ProcessLine(".include \"$MGDODIR/Majorana\""); 
    char title[200],titleSum[200], histname[200], FTWFname[200], outfilename[200];
    
    string calibrationFile = "/global/project/projectdirs/majorana/data/production/bk_calibration_summedrun.dat";
    int startrun=start;
    int endrun=end;
    int runType = 0; //0 is unknown type, 1 is PT cooler, 2 thermosyphon, 3 in shield
    int nChannels = 0;
    Double_t run = 0;
    if(endrun < startrun)
    {
	cout <<"Run range not recognized as valid. Exiting script."<< endl;
	return 0;
    }
    else if (startrun > 40002477 && endrun < 40002493)
    {
	runType = 1;
    	nChannels = 8;
    }
    else if (startrun > 40002945 && endrun < 40003137)
    {
	runType = 2;
	nChannels = 12;
    }
    else if (startrun > 40003439)
    {
	runType = 3;
	nChannels = 18;
    }
    else
    {
	cout <<"Run out of range. Start run is " << startrun <<". Exiting script."<< endl;
	return 0;
    }
    double adcScale= 1;
    double eventScale;

    vector<double>* channel = 0;

    MGTEvent* event = new MGTEvent();
    MGWFFastFourierTransformDefault* FFT = new MGWFFastFourierTransformDefault;
    MGWFBaselineRemover* BLRemove = new MGWFBaselineRemover;
    MGTWaveform* Wave;// = new MGTWaveform();
    //TH1D* hFT = new TH1D();
    //TH1D* hwave = new TH1D();
    //TH1D* hFTsum = new TH1D();
 
    
    GATCalibrationMap* calMap = new GATCalibrationMap();
    calMap->ReadCalibrationMapFromTextFile(calibrationFile);

    TH1D* hFTsumArray[nChannels];
    MGTWaveformFT* FTsumArray[nChannels];
    int first[nChannels];
    int totalEntries[nChannels];
    BLRemove->SetStartSample((size_t)0);
    BLRemove->SetBaselineSamples((size_t)512);
    TFile *histFile;
    MGTWaveformFT* FTWave;
    
    sprintf(FTWFname, "$MJDDATADIR/../../users/jgruszko/FT/FTWF/%d_to_%d", startrun, endrun);
    TFile *treeFile = TFile::Open(FTWFname, "RECREATE"); 
    TTree *FTTree = new TTree("FTTree", "FTTree");

    TClonesArray *FTWaveArr = new TClonesArray("MGTWaveformFT");
    FTTree->Branch("FTWaveArr", &FTWaveArr, 32000, 0);
    vector<double> lowFreqPower;
    vector<double> highFreqPower;
    FTTree->Branch("lowFreqPower", &lowFreqPower);
    FTTree->Branch("highFreqPower", &highFreqPower);
    
    //set ranges for low and high frequency power calculations
    double lowStartFreq = firstStart*CLHEP::MHz;
    double lowStopFreq = firstEnd*CLHEP::MHz;
    double highStartFreq = secondStart*CLHEP::MHz;
    double highStopFreq = secondEnd*CLHEP::MHz;
    double binWidth;
    size_t lowStartBin = 0, lowStopBin= 0, highStartBin = 0, highStopBin= 0;
    double lowPowerSum = 0;
    double highPowerSum = 0;


    GATDataSet* DataSet = new GATDataSet(startrun, endrun);

    for(int j=0;j<nChannels;j++)
    {
        FTsumArray[j]= new MGTWaveformFT();
        hFTsumArray[j]= new TH1D();
        sprintf(histname,"SummedFTForCh%d", j);
        hFTsumArray[j]->SetNameTitle(histname,histname);
        totalEntries[j] = 0;
    }
    //WARNING: const_cast of const TChain to non-const is required to access branches. This is a potentially unsafe operation. Do not make changes to saved data trees.  
    TChain* gatChain = const_cast<TChain*>(DataSet->GetGatifiedChain());
    TChain* builtChain = const_cast<TChain*>(DataSet->GetBuiltChain());
    if(!(gatChain && builtChain))
    {
	cout << "Data not found! Exiting script." << endl;
	return 0;
    }
     gatChain->SetBranchAddress("run",&run);
     gatChain->SetBranchAddress("channel",&channel);
     builtChain->SetBranchAddress("event",&event);
     for(int j=0;j<nChannels;j++)
     {
         first[j] = 0;
     }

     int cheasy = 0;
     int nentries= gatChain->GetEntries();
     cout << "number of entries " << nentries << endl;
    //  nentries = 10; 

     for(int k=0;k<nentries;k++)
     {
         if(k%100000 == 0)
         {
            cout << "event number " << k << " of " << nentries << " i.e. " << k/double(nentries)*100 << " % done" << endl;
         }
         //FTWaveArr->Delete();
         gatChain->GetEntry(k);
         builtChain->GetEntry(k);
         int n = channel->size();
         lowFreqPower.resize(n);
         highFreqPower.resize(n);
          for(int j=0; j<n; j++)
          {
             int ch = channel->at(j);
	     switch(runType)
	     {
		case 1:
               		// if(ch == 88) cheasy = 0;
              		 // if(ch == 89) cheasy = 1;
                	if(ch == 146) cheasy = 0;
                	if(ch == 147) cheasy = 1;
                	if(ch == 148) cheasy = 2;
                	if(ch == 149) cheasy = 3;
                	if(ch == 150) cheasy = 4;
                	if(ch == 151) cheasy = 5;
                	if(ch == 152) cheasy = 6;
                	if(ch == 153) cheasy = 7;
			break;
		case 2:
			if(ch == 112) cheasy = 0;
			if(ch == 113) cheasy = 1;
			if(ch == 114) cheasy = 2;
			if(ch == 115) cheasy = 3;
			if(ch == 118) cheasy = 4;
			if(ch == 119) cheasy = 5;
			if(ch == 146) cheasy = 6;
			if(ch == 147) cheasy = 7;
			if(ch == 148) cheasy = 8;
			if(ch == 149) cheasy = 9;
			if(ch == 150) cheasy = 10;
			if(ch == 151) cheasy = 11;
			break;
		case 3:
			if(ch == 112) cheasy = 0;
			if(ch == 113) cheasy = 1;
			if(ch == 114) cheasy = 2;
			if(ch == 115) cheasy = 3;
			if(ch == 118) cheasy = 4;
			if(ch == 119) cheasy = 5;
			if(ch == 120) cheasy = 6;
			if(ch == 121) cheasy = 7;
			if(ch == 144) cheasy = 8;
			if(ch == 145) cheasy = 9;
			if(ch == 146) cheasy = 10;
			if(ch == 147) cheasy = 11;
			if(ch == 148) cheasy = 12;
			if(ch == 149) cheasy = 13;
			if(ch == 150) cheasy = 14;
			if(ch == 151) cheasy = 15;
			if(ch == 152) cheasy = 16;
			if(ch == 153) cheasy = 17;
			break;
		}
                sprintf(title,"Event %d in channel %d", k, ch);
                sprintf(titleSum,"Summed FT of channel %d", ch);
                
		if((ch>145 && ch<154 && runType == 1) || (((ch>111 && ch<116)||( ch>117 && ch<120) || (ch>145 && ch<152)) && runType == 2) || (((ch>111 && ch<116)||( ch>117 && ch<122) || (ch>143 && ch<154)) && runType == 3))
                {
		    totalEntries[cheasy]++;
                    Wave=event->GetWaveform(j);
                    
	     	    adcScale = calMap->GetScale(ch, "energy",(size_t)run);
                    BLRemove->Transform(Wave);

		    Wave->SetLength(700);
                    FTWave = new((*FTWaveArr)[j]) MGTWaveformFT();
                    FFT->PerformFFT(Wave,FTWave);
	            if(k == 0 && j == 0)//first entry
	            {
			binWidth = ((FTWave->GetSamplingFrequency()/CLHEP::MHz)/2.0)*(1.0/FTWave->GetDataLength())*(CLHEP::MHz);
			lowStartBin = (size_t) floor((lowStartFreq+(0.5*binWidth))/binWidth);
			lowStopBin = (size_t) ceil((lowStopFreq-(0.5*binWidth))/binWidth);
			highStartBin = (size_t) floor((highStartFreq+(0.5*binWidth))/binWidth);
			highStopBin = (size_t) ceil((highStopFreq-(.5*binWidth))/binWidth);
		    }
		    //loop over sampling ranges for low and high freq power
		    if(!(lowStartFreq==0 && lowStopFreq==0))
		    {
		    	for(size_t x = lowStartBin; x < lowStopBin ; x++) 
		    	{  
			    if( x == 0 || x == FTWave->GetDataLength())
			    	lowPowerSum += std::norm(FTWave->At(x));
			    else
			    	lowPowerSum += 2*std::norm(FTWave->At(x));
		        }
		    }
		    else { lowPowerSum = 0; }
		    if(!(highStartFreq ==0 && highStopFreq ==0))
		    {
		    	for(size_t x = highStartBin; x < highStopBin+1 ; x++) 
		    	{  
			    if( x == 0 || x == FTWave->GetDataLength())
			    	highPowerSum += std::norm(FTWave->At(x));
			    else
			    	highPowerSum += 2*std::norm(FTWave->At(x));
		    	}
		   }
		    else { highPowerSum = 0; }
		    lowFreqPower[j] = lowPowerSum/adcScale;
		    highFreqPower[j] = highPowerSum/adcScale;
		    lowPowerSum = 0;
		    highPowerSum = 0;

                    if(first[cheasy]==0)
                    {
                        FTsumArray[cheasy]->MakeSimilarTo(*FTWave);
                        first[cheasy]++;
                    }
                    else
                        FTsumArray[cheasy]->AddNorms(*FTWave);
                   
		    FTsumArray[cheasy]->LoadIntoHist(hFTsumArray[cheasy], MGTWaveformFT::kPower);
                   
              }
           }

         FTTree->Fill();
	FTWaveArr->Delete();
      }
     cout << "done" << endl;
        
        for (int k=0; k<nChannels; k++)
        {
            hFTsumArray[k]->SetMaximum(1e15);
            if(totalEntries[k] > 0)
            {
              eventScale = 1/(double)totalEntries[k];
              cout << "Scaling by " << eventScale  << " to account for "<< totalEntries[k]<<" total entries in channel " << k << endl; 
              hFTsumArray[k]->Scale(eventScale);  //Scale power by number of events in channel
           }
	    else
            {
               cout << "No entries in this channel!" << endl;
            }
              cout << "Scaling by " << adcScale  << " to account for gain in channel " << k << endl; 
              hFTsumArray[k]->Scale(adcScale);  //Scale power by gain of channel
        
         //Write histogram to .root file for later use
         sprintf(outfilename, "$MJDDATADIR/../../users/jgruszko/FT/FT_Average/Ch%d_%d_to_%d.root",k, startrun, endrun);
         histFile = new TFile(outfilename, "RECREATE");
         hFTsumArray[k]->Write();
         histFile->Close();
      }
  treeFile->cd();
	treeFile->Write();
  // FTTree->Write("FTTree", TObject::kOverwrite);
   treeFile->Close();
  // gROOT->Reset();
  return 1;
}

int main(int argc, char* argv[])
{
    if(argc != 2 && argc != 3 && argc !=5 && argc!= 7){
	cout << "Usage: " << argv[0] << " 'starting run number' 'ending run number (optional)' 'first frequency range start (optional)' 'first frequency range end (optional)' 'second frequency range start (optional)' 'second frequency range end (optional)'. End of frequency range is required if start is given. Give frequencies in MHz." << endl;
	return 1;
    }
    int startRun = atoi(argv[1]);
    int endRun;
    double firstFreqStart, firstFreqEnd, secondFreqStart, secondFreqEnd;
    if(argc > 2){ endRun = atoi(argv[2]);}
    else{ endRun = startRun;}
    if(argc > 3)
    { 
	firstFreqStart = atoi(argv[3]);
	firstFreqEnd = atoi(argv[4]);
    }
    else
    {
	firstFreqStart = 0;
	firstFreqEnd = 0;
	secondFreqStart = 0;
	secondFreqEnd = 0;
    }
    if(argc > 5)
    { 
	secondFreqStart = atoi(argv[5]);
	secondFreqEnd = atoi(argv[6]);
    }
    return FTPower(startRun, endRun, firstFreqStart, firstFreqEnd, secondFreqStart, secondFreqEnd);
}
