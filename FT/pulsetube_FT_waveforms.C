{
    gROOT->Reset();
    
    char infile1[200], infile2[200], infilename1[200], infilename2[200],title[200],titleSum[200],plotwave[200],plotFT[200], histname[200], SumFTPlot[200], outfilename[200];
    
    int startrun=40002478;
    int endrun=40002478;
    int nChannels = 8;
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

    TFile *treeFile = TFile::Open("/global/u1/j/jgruszko/software/FT_Histograms/FTWaveforms_pulsetube.root", "RECREATE"); 
    TTree *FTTree = new TTree("FTTree", "FTTree");

    TClonesArray *FTWaveArr = new TClonesArray("MGTWaveformFT");
    FTTree->Branch("FTWaveArr", &FTWaveArr, 32000, 0);

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
     nentries = 1000;

     for(int k=0;k<nentries;k++)
     {
         if(k%100000 == 0)
         {
            cout << "event number " << k << " of " << nentries << " i.e. " << k/double(nentries)*100 << " % done" << endl;
         }
         //FTWaveArr->Delete();
         t1->GetEntry(k);
         int n = channel->size();
            
          for(int j=0; j<n; j++)
          {
             double e_keV = energy->at(j);
             int ch =  channel->at(j);
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
                sprintf(title,"Event %d in channel %d", k, ch);
                sprintf(titleSum,"Summed FT of channel %d", ch);
                
		if(/*ch == 88 || ch == 89 ||*/ (ch>145 && ch<154))
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

		 Wave->SetLength(800);
                    FTWave = new((*FTWaveArr)[j]) MGTWaveformFT();
			cout << "Created wave in FTWaveArr" << endl;
                    FFT->PerformFFT(Wave,FTWave);
                    cout << "Performed FFT" << endl;
		FTWave->Draw();
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
	cout<<"put wave into ttree." << endl;
	FTWaveArr->Clear();
	cout << "cleared FTWaveArr" << endl;
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
              cout << "Scaling by " << eventScale  << " to account for "<< totalEntries[k]<<" total entries in channel " << k << endl; 
              hFTsumArray[k]->Scale(eventScale);  //Scale power by number of events in channel
            }
	    else
            {
               cout << "No 1460 keV entries in this channel!" << endl;
            }
       
        
         //Write histogram to .root file for later use
         sprintf(outfilename, "/global/u1/j/jgruszko/software/FT_Histograms/Ch%d_v2.1.root",k);
         histFile = new TFile(outfilename, "RECREATE");
         hFTsumArray[k]->Write();
         histFile->Close();
	cout << "Wrote sum hist to file" << endl;
      }
  treeFile->cd();
	treeFile->Write();
  // FTTree->Write("FTTree", TObject::kOverwrite);
   cout << "wrote ttree to file" << endl;
   treeFile->Close();
   gROOT->Reset();
}

