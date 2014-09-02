{
   gROOT->Reset();

   TH1D* h = new TH1D();
   TFile *f;
   char chName[200], filename[300], histName[300];
   int nChannels = 8;
   int histRange;
   
   TCanvas *can;
   int num = 0;

      sprintf(chName, "Ch%d", num);
      sprintf(histName, "SummedFTForCh%d",num);
      can = new TCanvas;
      can->cd();
    leg = new TLegend(0.6, 0.6, 0.9, .9);
    leg->SetHeader("FT of Run 40002478-88");
      
      sprintf(filename, "/global/u1/j/jgruszko/software/FT_Histograms/%s_40002478_to_40002488.root", chName);
      f = new TFile(filename);
      h = (TH1D*)f->Get(histName);
      h->GetYaxis()->SetRangeUser(1E6, 1E12);
      can->SetLogy();
      can->Update();
      h->SetLineColor(num+1);
      h->Draw();
      can->Update();
  
    leg->AddEntry(h, histName, "f");
     num++;
   while(num < nChannels)
   {
     if(num%2 ==0)
     {
        sprintf(chName, "Ch%d", num);
        sprintf(filename, "/global/u1/j/jgruszko/software/FT_Histograms/%s_40002478_to_40002488.root", chName);
        f = new TFile(filename);
        sprintf(histName, "SummedFTForCh%d", num);
        h = (TH1D*)f->Get(histName);
        h->SetLineColor((num/2)+1);
        h->GetYaxis()->SetRangeUser(1E6, 1E12);
        h->Draw("same");
        can->Update();
    leg->AddEntry(h, histName, "f");
     }
        num++;
    } 
    leg->Draw(); 
     can->Update();
    can->Print("/global/u1/j/jgruszko/software/Plots/FT_Pulsetube.png");

}        

