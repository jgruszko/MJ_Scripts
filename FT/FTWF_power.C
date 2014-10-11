{
    gROOT->Reset();
    gROOT->ProcessLine(".x $MGDODIR/Root/LoadMGDOClasses.C"); 
    gROOT->ProcessLine(".L $MGDODIR/lib/libMGDOClasses.C"); 
    gROOT->ProcessLine(".include \"$MGDODIR/Majorana\""); 
    gROOT->ProcessLine("#include <complex>"); 
    char infile1[200], infile2[200], infilename1[200], infilename2[200],title[200],titleSum[200], histname[200], SumFTPlot[200], FTWFname[200], outfilename[200];
    
    int startrun=40002478;
    int endrun=40002492;
    int runType = 0; //0 is unknown type, 1 is PT cooler, 2 thermosyphon, 3 in shield
    int nChannels = 0;
    if(endrun < startrun)
    {
	cout <<"Run range not recognized as valid. Exiting script."<< endl;
	break;
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
	cout <<"Run range not recognized as valid. Exiting script."<< endl;
	break;
    }
    double eventScale;
    double adcScale;

    vector<double>* energy;
    vector<double>* channel;

    MGTEvent* event = new MGTEvent();
    MGWFFastFourierTransformDefault* FFT = new MGWFFastFourierTransformDefault;
    MGWFExtremumFinder* maxFinder = new MGWFExtremumFinder;
    MGWFBaselineRemover* BLRemove = new MGWFBaselineRemover;
    MGTWaveform* Wave;// = new MGTWaveform();
    //TH1D* hFT = new TH1D();
    //TH1D* hwave = new TH1D();
    //TH1D* hFTsum = new TH1D();
 
    
    TH1D* hFTsumArray[nChannels];
    MGTWaveformFT* FTsumArray[nChannels];
    int first[nChannels];
    int totalEntries[nChannels];
    maxFinder->SetFindMaximum();
    double maxAverage[nChannels];
    double maxValue;
    int wfCount[nChannels];
    BLRemove->SetStartSample((size_t)0);
    BLRemove->SetBaselineSamples((size_t)512);
    TFile *histFile;
    MGTWaveformFT* FTWave;
    
    sprintf(FTWFname, "/global/u1/j/jgruszko/software/FT_Results/FTWF/%d_to_%d", startrun, endrun);
    TFile *treeFile = TFile::Open(FTWFname, "RECREATE"); 
    TTree *FTTree = new TTree("FTTree", "FTTree");

    TClonesArray *FTWaveArr = new TClonesArray("MGTWaveformFT");
    FTTree->Branch("FTWaveArr", &FTWaveArr, 32000, 0);
    vector<double> lowFreqPower;
    vector<double> highFreqPower;
    FTTree->Branch("lowFreqPower", &lowFreqPower);
    FTTree->Branch("highFreqPower", &highFreqPower);
    
    //set ranges for low and high frequency power calculations
    double lowStartFreq = 2.5*CLHEP::MHz;
    double lowStopFreq = 3.0*CLHEP::MHz;
    double highStartFreq = 12.0*CLHEP::MHz;
    double highStopFreq = 14.0*CLHEP::MHz;
    double binWidth;
    size_t lowStartBin, lowStopBin, highStartBin, highStopBin;
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
	maxAverage[j] = 0;
	wfCount[j] = 0;
    }
    
    TChain* t1 = DataSet->GetChains();
    if(t1 == NULL)
    {
	cout << "Data not found! Exiting script." << endl;
	break;
    }

     t1->SetBranchAddress("energyCal",&energy);
     t1->SetBranchAddress("channel",&channel);
     t1->SetBranchAddress("event",&event);
     
     for(int j=0;j<nChannels;j++)
     {
         first[j] = 0;
     }

     int cheasy = 0;
     int nentries=t1->GetEntries();
     cout << "number of entries " << nentries << endl;
    // nentries = 10; 

     for(int k=0;k<nentries;k++)
     {
         if(k%100000 == 0)
         {
            cout << "event number " << k << " of " << nentries << " i.e. " << k/double(nentries)*100 << " % done" << endl;
         }
         //FTWaveArr->Delete();
         t1->GetEntry(k);
         int n = channel->size();
         lowFreqPower.resize(n);
         highFreqPower.resize(n);
          for(int j=0; j<n; j++)
          {
             double e_keV = energy->at(j);
             int ch =  channel->at(j);
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
                    
                    BLRemove->Transform(Wave);
		    if(e_keV < 1462 && e_keV > 1458)
		    {
			maxFinder->Transform(Wave);
			maxValue = maxFinder->GetTheExtremumValue();
			wfCount[cheasy]++;
			maxAverage[cheasy] += maxValue;
                    }

		    Wave->SetLength(700);
                    FTWave = new((*FTWaveArr)[j]) MGTWaveformFT();
                    FFT->PerformFFT(Wave,FTWave);

	            if(k == 0 && j == 0)//first entry
	            {
			binWidth = ((FTWave->GetSamplingFrequency()/CLHEP::MHz)/2.0)*(1.0/FTWave->GetDataLength())*(CLHEP::MHz);
			lowStartBin = (size_t) floor(lowStartFreq/binWidth);
			lowStopBin = (size_t) ceil(lowStopFreq/binWidth);
			highStartBin = (size_t) floor(highStartFreq/binWidth);
			highStopBin = (size_t) ceil(highStopFreq/binWidth);
		    }
		    //loop over sampling ranges for low and high freq power
		    for(int x = lowStartBin; x < lowStopBin+1 ; x++) 
		    {  
			if( x == 0 || x == FTWave->GetDataLength())
			    lowPowerSum += std::norm(FTWave->At(x));
			else
			    lowPowerSum += 2*std::norm(FTWave->At(x));
		    }

		    for(int x = highStartBin; x < highStopBin+1 ; x++) 
		    {  
			if( x == 0 || x == FTWave->GetDataLength())
			    highPowerSum += std::norm(FTWave->At(x));
			else
			    highPowerSum += 2*std::norm(FTWave->At(x));
		    }
		    lowFreqPower[j] = lowPowerSum;
		    highFreqPower[j] = highPowerSum;
		    lowPowerSum = 0;
		    highPowerSum = 0;

                    if(first[cheasy]==0)
                    {
                        FTsumArray[cheasy]->MakeSimilarTo(*FTWave);
                        first[cheasy]++;
                    }
                    else
                        FTsumArray[cheasy]->AddNorms(*FTWave);
                   
		    FTsumArray[cheasy]->LoadIntoHist(hFTsumArray[cheasy], 2);
                    
              }
           }
         FTTree->Fill();
	FTWaveArr->Clear();
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
            if(wfCount[k] > 0)
            {
	      maxAverage[k] = maxAverage[k]/((double)wfCount[k]);
              adcScale = maxAverage[k]/1460.0;	
              cout << "Scaling by " << adcScale  << " to account for gain in channel " << k << endl; 
              hFTsumArray[k]->Scale(adcScale);  //Scale power by gain of channel
            }
	    else
            {
               cout << "No 1460 keV entries in this channel!" << endl;
            }
       
        
         //Write histogram to .root file for later use
         sprintf(outfilename, "/global/u1/j/jgruszko/software/FT_Results/Average_Histograms/Ch%d_%d_to_%d.root",k, startrun, endrun);
         histFile = new TFile(outfilename, "RECREATE");
         hFTsumArray[k]->Write();
         histFile->Close();
      }
  treeFile->cd();
	treeFile->Write();
  // FTTree->Write("FTTree", TObject::kOverwrite);
   treeFile->Close();
   gROOT->Reset();
}

