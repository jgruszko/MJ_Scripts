{
   gROOT->Reset();
 
   TFile *ftFile = new TFile("/global/u1/j/jgruszko/software/FT_Results/FTWF/45000459_to_45000460");
   TTree* FTTree = (TTree*)ftFile->Get("FTTree");

   FTTree->AddFriend("MGTree", "$MJDDATADIR/surfprot/data/built/P3END/OR_run45000459.root");
   FTTree->AddFriend("mjdTree","$MJDDATADIR/surfprot/data/gatified/P3END/mjd_run45000459.root");
   

   gROOT->cd();
 
   int nChannels= 18;
   char wavename[200], ftname[300];
   char response;
   //TH1D* hFTWFArr[nChannels];
   TClonesArray* ftArr = new TClonesArray("MGTWaveformFT");
   MGTEvent* event = new MGTEvent();
   vector<size_t>* channel;
   vector<double>* energy;
   
   TCanvas* can= new TCanvas("can", "Fourier Transform Browser", 1000, 800);
   can->Divide(2, 2);

   FTTree->SetBranchAddress("FTWaveArr", &ftArr);
   FTTree->SetBranchAddress("event", &event);
   FTTree->SetBranchAddress("channel", &channel);
   FTTree->SetBranchAddress("energyCal", &energy);   

   MGTWaveformFT* ftWave;
   MGTWaveform* wave;
   MGTWaveform baseline;
   double energy_keV; 

   int nentries = FTTree->GetEntries();
   nentries = 100;
   int cheasy = 0;
   size_t ch = 0;
   size_t n = 0;
   int jumpTo;

   TH1D* ftHist = new TH1D();
   TH1D* waveHist = new TH1D(); 
   TH1D* baselineHist = new TH1D();  

   for(int i = 0; i < nentries; i++)
   {
     if (response == 'q'){  break;  }
     FTTree->GetEntry(i);
     n = channel->size();  
     for(int j=0; j<n; j++)
     {
	if (response == 'q'){  break;  }
	
	ch =  channel->at(j);
	//if(ch == 88) cheasy = 0;
	//if(ch == 89) cheasy = 1;
	/*
 * if(ch == 146) cheasy = 0;
	if(ch == 147) cheasy = 1;
	if(ch == 148) cheasy = 2;
	if(ch == 149) cheasy = 3;
	if(ch == 150) cheasy = 4;
	if(ch == 151) cheasy = 5;
	if(ch == 152) cheasy = 6;
	if(ch == 153) cheasy = 7;      
*/
	if(ch == 150) cheasy =0;
	if(ch == 151) cheasy =1;                                                                           
	if (ch==150 || ch==151)
	{
	    can->cd(2);
	    wave = event->GetWaveform(j);
	    baseline = *wave;
	    baseline.SetLength(800);
	    baselineHist = baseline.GimmeHist();
	    sprintf(wavename, "Waveform %d of event %d, in channel %d", j, i, ch);
            cout << wavename << endl;
	    baselineHist->SetTitle(wavename);
	    baselineHist->Draw();
	   
	    energy_keV = energy->at(j);
	    cout << "Onboard energy is " << energy_keV << " keV in channel "<< ch << endl;
		
	    
	    can->cd(1);
	    ftWave = (MGTWaveformFT*)ftArr->At(j);
	    ftHist  = ftWave->GimmeUniqueHist();
	    ftHist->SetTitle("FT Power");
	    //ftHist->SetLogy();
	    ftHist->GetYaxis()->SetRangeUser(1, 1E6);
	    ftHist->Draw();
	    can->Update();
	
	    can->cd(4);
	    waveHist = wave->GimmeHist();
	    waveHist->SetTitle("Full Waveform");
	    waveHist->Draw();
	    can->Update();
	    cout << "Enter 'j' to jump to an event,'q' to quit browsing, or any alphabetic character for the next waveform: ";
            cin >> response;
	   if (response == 'j')
	   {
		cout << "Enter event to jump to: ";
		cin >> jumpTo;
		i = jumpTo - 1;
	   }
	   else if (response == 'q'){  break;  }
	
	   can->Clear();
	   can->Divide(2, 2);
	}
      }
    }

} 
