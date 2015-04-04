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

int FTPower_noGAT(int start, int end, int samples, vector<double> rangeStart, vector<double> rangeEnd)
{
    //gROOT->Reset();
    //gROOT->ProcessLine(".x $MGDODIR/Root/LoadMGDOClasses.C"); 
    //gROOT->ProcessLine(".L $MGDODIR/lib/libMGDOClasses.C"); 
    //gROOT->ProcessLine(".include \"$MGDODIR/Majorana\""); 
    char title[200],titleSum[200], parName[200], histname[200], fileName[300], FTWFname[200], outfilename[200];
    
    //string calibrationFile = "/global/project/projectdirs/majorana/data/production/bk_calibration_summedrun.dat";
    int startrun=start;
    int endrun=end;
    int nSamples = samples;
    int runType = 0; //0 is unknown type, 1 is PT cooler, 2 thermosyphon, 3 in shield
    int nChannels = 0;
    vector<double> freqStart=rangeStart;
    vector<double> freqEnd=rangeEnd;
    int nFreqs = freqStart.size();

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
    //double adcScale= 1;
    double eventScale;


    MGTEvent* event = new MGTEvent();
    MGWFFastFourierTransformDefault* FFT = new MGWFFastFourierTransformDefault;
    MGWFBaselineRemover* BLRemove = new MGWFBaselineRemover;
    MGTWaveform* Wave;// = new MGTWaveform();
    //TH1D* hFT = new TH1D();
    //TH1D* hwave = new TH1D();
    //TH1D* hFTsum = new TH1D();
 
    
    //GATCalibrationMap* calMap = new GATCalibrationMap();
    //calMap->ReadCalibrationMapFromTextFile(calibrationFile);

    TH1D* hFTsumArray[nChannels];
    MGTWaveformFT* FTsumArray[nChannels];
    int first[nChannels];
    int totalEntries[nChannels];
    BLRemove->SetStartSample((size_t)0);
    BLRemove->SetBaselineSamples((size_t)512);
    TFile *histFile;
    MGTWaveformFT* FTWave;
    
    if(startrun != endrun){ sprintf(FTWFname, "/global/project/projectdirs/majorana/users/jgruszko/FT/FTWF/%d_to_%d.root", startrun, endrun); }
    else { sprintf(FTWFname, "/global/project/projectdirs/majorana/users/jgruszko/FT/FTWF/%d.root", startrun); }
    TFile *treeFile = new TFile(FTWFname, "RECREATE"); 
    TTree *FTTree = new TTree("FTTree", "FTTree");

    TClonesArray *FTWaveArr = new TClonesArray("MGTWaveformFT");
    FTTree->Branch("FTWaveArr", &FTWaveArr, 32000, 0);
    
    vector< vector<double> > freqPower; //vector of frequency band integral results for each event. Frequency band is first index, wf is 2nd index
    freqPower.resize(nFreqs);
    for(int i = 0; i<nFreqs; i++)
    {
	sprintf(parName, "%.2fto%.2fPower", freqStart.at(i), freqEnd.at(i));
        FTTree->Branch(parName, &freqPower.at(i));
    }
    vector<double> integratedPower;
    FTTree->Branch("integratedPower", &integratedPower);
    
    vector<int> channel;
    FTTree->Branch("channel", &channel);
    //set ranges for low and high frequency power calculations
    double binWidth;
    vector<int> startBin;
    vector<int> stopBin;
    startBin.resize(nFreqs);
    stopBin.resize(nFreqs);
    double sum = 0;
    double totalSum = 0;
    size_t nBins = 0;


    //GATDataSet* DataSet = new GATDataSet(startrun, endrun);

    for(int j=0;j<nChannels;j++)
    {
        FTsumArray[j]= new MGTWaveformFT();
        hFTsumArray[j]= new TH1D();
        sprintf(histname,"Ch%d_AvgFT", j);
        hFTsumArray[j]->SetNameTitle(histname,histname);
        totalEntries[j] = 0;
    }
    TChain* builtChain = new TChain("MGTree");
    for(int r = startrun; r<endrun+1; r++)
    {
        sprintf(fileName, "/global/project/projectdirs/majorana/users/iguinn/Built/OR_run%d.root", r);
	builtChain->Add(fileName);
    }
     //gatChain->SetBranchAddress("run",&run);
     //gatChain->SetBranchAddress("channel",&channel);
     builtChain->SetBranchAddress("event",&event);
     for(int j=0;j<nChannels;j++)
     {
         first[j] = 0;
     }

     int cheasy = 0;
     int nentries= builtChain->GetEntries();
     cout << "number of entries " << nentries << endl;
      //nentries = 10; 

     for(int k=0;k<nentries;k++)
     {
         if(k%100000 == 0)
         {
            cout << "event number " << k << " of " << nentries << " i.e. " << k/double(nentries)*100 << " % done" << endl;
         }
         //FTWaveArr->Delete();
        // gatChain->GetEntry(k);
         builtChain->GetEntry(k);
         int n = event->GetNWaveforms();
        for(int j= 0; j<nFreqs; j++) {freqPower.at(j).resize(n);}
	integratedPower.resize(n);
	channel.resize(n);
          for(int j=0; j<n; j++)
          {
             int ch = event->GetWaveform(j)->GetID();
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
                
		    totalEntries[cheasy]++;
                    Wave=event->GetWaveform(j);
                   if(Wave == 0){ cout<<"0 waveform found at event "<<k <<", waveform "<<j<<endl; }
	     	    //adcScale = calMap->GetScale(ch, "energy",(size_t)run);
                    BLRemove->Transform(Wave);

		    Wave->SetLength(nSamples);
                    FTWave = new((*FTWaveArr)[j]) MGTWaveformFT();
                    FFT->PerformFFT(Wave,FTWave);
                   if(FTWave == 0){ cout<<"0 ft waveform made at event "<<k <<", waveform "<<j<<endl; }
	            if(k == 0 && j == 0)//first entry
	            {
			binWidth = ((FTWave->GetSamplingFrequency()/CLHEP::MHz)/2.0)*(1.0/FTWave->GetDataLength())*(CLHEP::MHz);
			for(int x = 0; x<nFreqs; x++)
			{
			    startBin[x] = (size_t) floor(((freqStart.at(x)*CLHEP::MHz)+(0.5*binWidth))/((double)binWidth));
			    stopBin[x] = (size_t) ceil(((freqEnd.at(x)*CLHEP::MHz)-(0.5*binWidth))/((double)binWidth));
			    cout<<"Integrating from bin "<<startBin[x]<<" to "<<stopBin[x]<<" for range "<<freqStart.at(x)<<" to "<< freqEnd.at(x)<<endl;
			}
			nBins = FTWave->GetDataLength();
		    }
		    //loop over sampling ranges
		    for(int y = 0; y <nFreqs; y++)
		    {
		    	for(int x = startBin[y]; x < stopBin[y]; x++)
			{ 
			    if( x == 0 || x == (int)FTWave->GetDataLength())
			    	sum += std::norm(FTWave->At(x));
			    else
			    	sum += 2*std::norm(FTWave->At(x));
		        }
			freqPower.at(y).at(j) = sum;
		    }
		    for(size_t x = 0; x < nBins; x++)
		    {
			if( x == 0 || x == FTWave->GetDataLength())
			    totalSum += std::norm(FTWave->At(x));
			else
			    totalSum += 2*std::norm(FTWave->At(x));
		    }
		    	
		    integratedPower[j] = totalSum;
		    channel[j] = Wave->GetID();
		    if(channel[j] ==0){ cout<<"Found channel = 0 at event "<<k<<", waveform "<<j<<endl; }
		    sum = 0;
		    totalSum = 0;

                    if(first[cheasy]==0)
                    {
                        FTsumArray[cheasy]->MakeSimilarTo(*FTWave);
                        first[cheasy]++;
                    }
                    else
                        FTsumArray[cheasy]->AddNorms(*FTWave);
                   
		    FTsumArray[cheasy]->LoadIntoHist(hFTsumArray[cheasy], MGTWaveformFT::kPower);
                   
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
 	      //cout << "Scaling by " << adcScale  << " squared to account for gain in channel " << k << endl; 
              //hFTsumArray[k]->Scale(adcScale*adcScale);  //Scale power by gain of channel
         
	//Write histogram to .root file for later use
         if(startrun != endrun){ sprintf(outfilename, "/global/project/projectdirs/majorana/users/jgruszko/FT/FT_Average/%d_to_%d.root", startrun, endrun); }
        else { sprintf(outfilename, "/global/project/projectdirs/majorana/users/jgruszko/FT/FT_Average/%d.root", startrun); }
	histFile = new TFile(outfilename, "UPDATE");
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
    if(argc%2 != 0 && argc != 3){
	cout << "Usage: " << argv[0] << " 'starting run number' 'ending run number (optional)' 'number of waveform samples to take FT of'  'frequency range start' 'frequency range end'.... End of frequency range is required if start is given. Give frequencies in MHz." << endl;
	return 1;
    }
    int startRun = atoi(argv[1]);
    int endRun;
    int nSamples;
    if(argc > 2){ endRun = atoi(argv[2]);}
    else{ endRun = startRun;}
    if(argc > 3){ nSamples = atoi(argv[3]); }
    else{ nSamples = 2018; }
    vector<double> freqStarts;
    vector<double> freqEnds;
    int i = 4;
    while(i<argc) //Fill in the arrays with all the frequency ranges given
    {
	freqStarts.push_back(atof(argv[i]));
	freqEnds.push_back(atof(argv[i+1]));
	i=i+2; 
    }
    return FTPower_noGAT(startRun, endRun, nSamples, freqStarts, freqEnds);
}
