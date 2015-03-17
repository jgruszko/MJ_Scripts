{
   gROOT->ProcessLine("#include<complex>");
   int startRun = 45003831;
   int endRun = 45003846;
   int selectedCh = 112;
   int currentRun = startRun-1;
   char ftFileName[300], gatFileName[300], builtFileName[300], histFileName[300];

   sprintf(ftFileName, "/global/project/projectdirs/majorana/users/jgruszko/FT/FTWF/%d_to_%d", startRun, endRun);
   TFile *ftFile = new TFile(ftFileName);
   TTree* FTTree = (TTree*)ftFile->Get("FTTree");
   while(currentRun < endRun){
   GATDataSet *data = new GATDataSet(startRun, endRun);
   //TChain* builtChain = data->GetBuiltChain(false);
   TChain* gatChain = data->GetGatifiedChain(false);
   if(/*builtChain == NULL ||*/ gatChain == NULL){ break; } 
   
   //TFile *builtFile = new TFile("$MJDDATADIR/surfprot/data/built/P3END/OR_run45001822.root");
   //TTree* MGTree = (TTree*)builtFile->Get("MGTree");
   
   //TFile *gatFile = new TFile("$MJDDATADIR/surfprot/data/gatified/P3END/mjd_run45001822.root");
   //TTree* mjdTree = (TTree*)gatFile->Get("mjdTree");
   
	
   int nChannels= 1;
   char wavename[200], gifFile[300], histName[200], canName[200], averageName[200], averageTitle[300], integratedTitle[300], title[300], selection[300], axisTitle[200];
   TClonesArray* ftArr = new TClonesArray("MGTWaveformFT");
   std::complex<double> value;
   double power;
   double run;
   vector<double>* energy;
   vector<double>* timestamp;
   vector<double>* ch;
   
   FTTree->SetBranchAddress("FTWaveArr", &ftArr);
   gatChain->SetBranchAddress("timestamp", &timestamp);
   gatChain->SetBranchAddress("channel", &ch);
   gatChain->SetBranchAddress("energyCal", &energy);   
   gatChain->SetBranchAddress("run", &run);   
  
   double energy_keV; 
   double time;
   int nentriesFT = FTTree->GetEntries();
   int nentriesGAT = gatChain->GetEntries();
   if(nentriesFT < nentriesGAT){ int nentries = nentriesFT ;}
   else{ int nentries = nentriesGAT; }
   //nentries = 100;
   
   sprintf(histName, "Ch_%d", selectedCh);
   sprintf(averageName, "Ch_%d_Average", selectedCh); 
   sprintf(title, "FT Power vs. Event Number in Channel %d", selectedCh);
   sprintf(averageTitle, "FT Power Average in Channel %d", selectedCh);
   sprintf(integratedTitle, "Integrated FT Power in Channel %d", selectedCh);
   sprintf(selection, "timestamp > 4E10 && channel == %d && energyCal > 20", selectedCh);
   
   int chan = 0;
   size_t n = 0;
   int nWF = 0; //wf counter
   
   MGTWaveformFT* ftWave;
   //intitialize everything for the FT waveform samples
   FTTree->GetEntry(0);
   gatChain->GetEntry(0);
   if( ftArr->At(0)!= 0){ ftWave = (MGTWaveformFT*) ftArr->At(0); } 
   size_t dataLength = ftWave->GetDataLength();
   double nyquistFreq = ((ftWave->GetSamplingFrequency()/CLHEP::MHz)*0.5);
   double binWidth = nyquistFreq/dataLength;
   double startFreq = 0-binWidth/2.0;
   double endFreq = nyquistFreq - binWidth/2.0;
   //use the number of entries that pass the cut
   int entriesPass = (int) gatChain->Draw("channel", selection, "goff");
   //entriesPass = 100;
  if(entriesPass > 100000)
   { 
	entriesPass = 100000;
	cout<<"Too many entries. Using only the first 100000 waveforms."<< endl;
   }  
   TH2D* powerHist = new TH2D(histName, title, (int) (entriesPass), -0.5, entriesPass-0.5, dataLength,startFreq, endFreq);
   TH1D* averagePower = new TH1D();
   averagePower->SetTitle(averageTitle);
   TH1D* integratedPower = new TH1D();
   integratedPower->SetTitle(integratedTitle);
   TCanvas* can = new TCanvas("can", title, 1000, 1000);
   can->Divide(2,2);
   TLine* line[endRun-startRun];
   int runCount = 0;
   for(int i = 0; i < nentries; i++)
   {
     if(nWF<entriesPass)
     {
	if(i%10000==0){ cout <<" Checking entry "<< i <<" of "<< nentries<<" i.e. "<<100*(((double)i)/nentries)<< " % finished"<<endl; }
	FTTree->GetEntry(i);
	gatChain->GetEntry(i);
	n = ch->size();
	for(int j = 0; j < n; j++)
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
	    if(chan == selectedCh/*&& energy_keV > 30 && time > 4E10*/ && ftWave != 0 )
	    {
		if(run != currentRun){
		    currentRun = run;
		    can->cd(1);
		    line[runCount] = new TLine((double)nWF, startFreq, (double)nWF, endFreq);
		    line[runCount]->SetLineColor(6);
		    line[runCount]->SetLineWidth(2);
		    runCount++;
		}
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
	powerHist->Delete();
	averagePower->Delete();
	integratedPower->Delete();
	startRun = currentRun;
   } 

}
