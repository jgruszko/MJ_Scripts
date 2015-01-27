{
   gROOT->ProcessLine("#include<complex>");
   int startRun = 45003831;
   int endRun = 45003846;
   char ftFileName[300], gatFileName[300], builtFileName[300], histFileName[300];

   sprintf(ftFileName, "/global/project/projectdirs/majorana/users/jgruszko/FT/FTWF/%d_to_%d", startRun, endRun);
   TFile *ftFile = new TFile(ftFileName);
   TTree* FTTree = (TTree*)ftFile->Get("FTTree");
cout<<"Opened FT file"<<endl;
   GATDataSet *data = new GATDataSet(startRun, endRun);
   //TChain* builtChain = data->GetBuiltChain(false);
   TChain* gatChain = data->GetGatifiedChain(false);
   if(/*builtChain == NULL ||*/ gatChain == NULL){ break; } 
   cout<<"Opened gat chain"<<endl;
   
   //TFile *builtFile = new TFile("$MJDDATADIR/surfprot/data/built/P3END/OR_run45001822.root");
   //TTree* MGTree = (TTree*)builtFile->Get("MGTree");
   
   //TFile *gatFile = new TFile("$MJDDATADIR/surfprot/data/gatified/P3END/mjd_run45001822.root");
   //TTree* mjdTree = (TTree*)gatFile->Get("mjdTree");
   
   sprintf(histFileName, "/global/project/projectdirs/majorana/users/jgruszko/FT/FT_Power/%d_to_%d.root", startRun, endRun); 
   TFile *outFile; 

   int nChannels= 18;
   char wavename[200], gifFile[300], histName[200], canName[200], averageName[200], averageTitle[300], integratedTitle[300], title[300], selection[300], axisTitle[200];
   TClonesArray* ftArr = new TClonesArray("MGTWaveformFT");
   std::complex<double> value;
   double power;
   vector<double>* energy;
   vector<int>* timestamp;
   vector<size_t>* ch;
   
   FTTree->SetBranchAddress("FTWaveArr", &ftArr);
   gatChain->SetBranchAddress("timestamp", &timestamp);
   gatChain->SetBranchAddress("channel", &ch);
   gatChain->SetBranchAddress("energyCal", &energy);   
   double energy_keV; 
   int time;
   int nentriesFT = FTTree->GetEntries();
   int nentriesGAT = gatChain->GetEntries();
   if(nentriesFT < nentriesGAT){ int nentries = nentriesFT ;}
   else{ int nentries = nentriesGAT; }
   //nentries = 100;
   int cheasyArr[nChannels] = {112, 113, 114, 115, 118, 119, 120, 121, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153};
   int cheasy = 0;
   size_t chan = 0;
   size_t n = 0;
   int nWF[nChannels]; //wf counter
   TH2D* powerHist[nChannels];
   TH1D* averagePower[nChannels];
   TH1D* integratedPower[nChannels];
   TCanvas* canArr[nChannels];
   MGTWaveformFT* ftWave;
   //intitialize everything for the FT waveform samples
   FTTree->GetEntry(0);
   gatChain->GetEntry(0);
   if( ftArr->At(0)!= 0){ ftWave = (MGTWaveformFT*) ftArr->At(0); cout<<"Got ftwf 0, 0"<<endl; } 
   size_t dataLength = ftWave->GetDataLength();
   double nyquistFreq = ((ftWave->GetSamplingFrequency()/CLHEP::MHz)*0.5);
   double binWidth = nyquistFreq/dataLength;
   double startFreq = 0-binWidth/2.0;
   double endFreq = nyquistFreq - binWidth/2.0;
   int entriesPass[nChannels];
   //intialize channel plots
   for(int a = 0; a < nChannels; a++)
   {
	nWF[a] = 0;
	sprintf(histName, "Ch_%d",cheasyArr[a]);
	sprintf(canName, "can%d", a);
	sprintf(averageName, "Ch_%d_Average", cheasyArr[a]);
	sprintf(title, "FT Power vs. Event Number in Channel %d", cheasyArr[a]);
	sprintf(averageTitle, "FT Power Average in Channel %d", cheasyArr[a]);
	sprintf(integratedTitle, "Integrated FT Power in Channel %d", cheasyArr[a]);
	sprintf(selection, "timestamp > 4E10 && channel == %d", cheasyArr[a]);

	//use the number of entries that pass the cut
	entriesPass[a] = (int) gatChain->Draw("channel", selection, "goff");
	powerHist[a] = new TH2D(histName, title, (int) (entriesPass[a]/10), -0.5, entriesPass[a]-0.5, dataLength,startFreq, endFreq);
	averagePower[a] = new TH1D();
	averagePower[a]->SetTitle(averageTitle);
	integratedPower[a] = new TH1D();
	integratedPower[a]->SetTitle(integratedTitle);
	canArr[a] = new TCanvas(canName, title, 1000, 1000);
	canArr[a]->Divide(2,2);
    }

   for(int i = 0; i < nentries; i++)
   {
	if(i%10==0){ cout <<" Checking entry "<< i <<" of "<< nentries<<" i.e. "<<100*(i/nentries)<< " % finished"<<endl; }
	FTTree->GetEntry(i);
	gatChain->GetEntry(i);
	n = ch->size();
	for(int j = 0; j < n; j++)
	{
	    chan = ch->at(j); 
	    time = timestamp->at(j); 
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
	    if(chan == 112) cheasy = 0;
	    if(chan == 113) cheasy = 1;
	    if(chan == 114) cheasy = 2;
	    if(chan == 115) cheasy = 3;
	    if(chan == 118) cheasy = 4;
	    if(chan == 119) cheasy = 5;
	    if(chan == 120) cheasy = 6;
	    if(chan == 121) cheasy = 7;
	    if(chan == 144) cheasy = 8;
	    if(chan == 145) cheasy = 9;
	    if(chan == 146) cheasy = 10;
	    if(chan == 147) cheasy = 11;
	    if(chan == 148) cheasy = 12;
	    if(chan == 149) cheasy = 13;
	    if(chan == 150) cheasy = 14;
	    if(chan == 151) cheasy = 15;
	    if(chan == 152) cheasy = 16;
	    if(chan == 153) cheasy = 17;
	    if(cheasy > -1 && cheasy < nChannels && time > 4E0 && ftWave != 0)
	    {
		for(int k = 0; k < dataLength; k++)
	    	{
		    if(k == 0 || k == dataLength - 1)
		    { 
		    	power = std::norm(ftWave->At(k)); 
		    }	
		    else
		    { 
		    	power = 2.*std::norm(ftWave->At(k)); 
		    }		
		   	powerHist[cheasy]->SetBinContent(nWF[cheasy], k, power);
	        }
		nWF[cheasy]++;
	     }
	  }
	  ftArr->Clear("C");
	}
	for(int b=0; b<nChannels; b++)
	{
	    sprintf(axisTitle, "Event Number in Channel %d (timestamp > 4E10)", cheasyArr[b]);
	    powerHist[b]->GetXaxis()->SetTitle(axisTitle);
	    powerHist[b]->GetYaxis()->SetTitle("Frequency (MHz)");
	    canArr[b]->cd(1);
	    gPad->SetLogy();
	    gPad->SetLogz();
	    canArr[b]->Update();
	    powerHist[b]->Draw("COLZ");
	    canArr[b]->cd(2);
	    gPad->SetLogy();
	    canArr[b]->Update();
	    averagePower[b]=powerHist[b]->ProjectionY();
	    averagePower[b]->Scale(entriesPass[b]);
	    averagePower[b]->GetXaxis()->SetTitle("Frequency (MHz)");
	    averagePower[b]->GetYaxis()->SetTitle("Power");
	    averagePower[b]->Draw();
	    canArr[b]->cd(4);
	    canArr[b]->Update();
	    integratedPower[b]=powerHist[b]->ProjectionX();
	    integratedPower[b]->GetXaxis()->SetTitle(axisTitle);
	    integratedPower[b]->GetYaxis()->SetTitle("Integrated Power");
	    integratedPower[b]->Draw();
	    
	    sprintf(gifFile, "/global/project/projectdirs/majorana/users/jgruszko/FT/Plots/3D_Power/%d_to_%d_Ch%d.gif", startRun, endRun, b);
	    canArr[b]->Print(gifFile);

	    
   	    outFile= new TFile(histFileName, "UPDATE");
	    powerHist[b]->Write();
	    outFile->Close(); 
	}

}
