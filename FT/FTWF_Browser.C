{
   gROOT->Reset();
 
   TFile *ftFile = new TFile("/global/project/projectdirs/majorana/users/jgruszko/FT/FTWF/45003831_to_45003846");
   TTree* FTTree = (TTree*)ftFile->Get("FTTree");
   GATDataSet* data = new GATDataSet(45003831, 45003846);
   TChain* gat = data->GetGatifiedChain(false);
   TChain* built = data->GetBuiltChain(false);


   gROOT->cd();
 
   int nChannels= 18;
   char wavename[200], ftname[300], energyText[400], HFText[400], LFText[400];
   char response;
   //TH1D* hFTWFArr[nChannels];
   TClonesArray* ftArr = new TClonesArray("MGTWaveformFT");
   MGTEvent* event = new MGTEvent();
   vector<size_t>* channel;
   vector<double>* energy;
   vector<double>* lowFreqPower;
   vector<double>* highFreqPower;
   
   TCanvas* can= new TCanvas("can", "Fourier Transform Browser", 1000, 800);
   can->Divide(2, 2);
   can->cd(3);
   TPaveText *pt = new TPaveText(.05, .1, .95, .8, "NB");
 
   FTTree->SetBranchAddress("FTWaveArr", &ftArr);
   built->SetBranchAddress("event", &event);
   gat->SetBranchAddress("channel", &channel);
   gat->SetBranchAddress("energyCal", &energy);   
   FTTree->SetBranchAddress("highFreqPower", &highFreqPower);   
   FTTree->SetBranchAddress("lowFreqPower", &lowFreqPower);   

   MGTWaveformFT* ftWave;
   MGTWaveform* wave;
   MGTWaveform baseline;
   double energy_keV; 
   double lowPower; 
   double highPower; 

   int nentries = FTTree->GetEntries();
   //nentries = 100;
   int cheasy = 0;
   size_t ch = 0;
   size_t n = 0;
   int jumpToEvent;
   int jumpToWF = -1;

   TH1D* ftHist = new TH1D();
   TH1D* waveHist = new TH1D(); 
   TH1D* baselineHist = new TH1D();  

   for(int i = 0; i < nentries; i++)
   {
     if (response == 'q'){  break;  }
     FTTree->GetEntry(i);
     gat->GetEntry(i);
     built->GetEntry(i);
     n = channel->size();  
     for(int j=0; j<n; j++)
     {
	if (response == 'q'){  break;  }
	if(jumpToWF != -1)
	{
	    if(jumpToWF < n) { j = jumpToWF; }
	    else{ cout << "There is no waveform " << jumpToWF << " in event "<< i <<". Showing waveform "<< j <<" instead." <<endl; }
	}
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
	can->cd(2);
	    wave = event->GetWaveform(j);
	    baseline = *wave;
	    baseline.SetLength(512);
	    baselineHist = baseline.GimmeHist();
	    sprintf(wavename, "Baseline of waveform %d of event %d", j, i);
            cout << wavename << endl;
	    baselineHist->SetTitle(wavename);
	    baselineHist->Draw();
	   
	    energy_keV = energy->at(j);
	    lowPower = lowFreqPower->at(j);	   
	    highPower = highFreqPower->at(j);	   
		
	can->cd(3);
	    pt->Clear();
	    sprintf(energyText, "Onboard energy is %g keV in channel %d", energy_keV, ch);
	    sprintf(LFText,"Low frequency power is %E", lowPower);
	    sprintf(HFText,"High frequency power is %E", highPower);
	    pt->AddText(energyText);
	    pt->AddText(LFText);
	    pt->AddText(HFText);
	    pt->Draw();
	    cout << "Onboard energy is " << energy_keV << " keV in channel "<< ch << endl;
	    cout << "Low frequency power is " << lowPower <<endl;
	    cout << "High frequency power is " << highPower <<endl;
		
	    
	    can->cd(1);
	    gPad->SetLogy();
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
		cout << "Enter 	event to jump to: ";
		cin >> jumpToEvent;
		i = jumpToEvent - 1;
		cout << "Enter 	waveform of event to jump to: ";
		cin >> jumpToWF;
		break;
		
	   }
	   else if (response == 'q'){  break;  }
	   else{ jumpToWF = -1; }	
	   can->Clear();
	   can->Divide(2, 2);
	}
      }
    }

} 
