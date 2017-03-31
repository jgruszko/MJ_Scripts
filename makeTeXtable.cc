{
  gROOT->ProcessLine(".x $GATDIR/LoadGATClasses.C");
  TString inFile = "/global/projecta/projectdirs/majorana/users/jgruszko/analysis/DCR/DS2/efficiency/dcr99_eff.txt";
  TString texStr = "/global/projecta/projectdirs/majorana/users/jgruszko/analysis/DCR/DS2/efficiency/dcr99_eff.tex";
  ifstream paramFile(inFile, ios::in);
  ofstream texFile(texStr, ios::out | ios::app);
  if(!paramFile.is_open() || !texFile.is_open()){ 
    cout<<"Failed to open one of the files"<<endl;	
    return 0;
  }
  std::string currentStr, line, toWrite;
  TString tempStr;
  float currentVal;
  long posTex = texFile.tellp();
  texFile<<"\\begin{tabular}{l r r r r r r}"<<endl; //change this to correct column structure
  while(std::getline(paramFile, line)){
    std::istringstream row(line);
    while( row>> currentStr){
      if (toWrite != "") texFile<<std::setw(12)<<toWrite;

      if(!std::isalpha(currentStr[0]) && currentStr.find('.') != std::string::npos){ //in this case, make decimal a percentage and round to tens place
        currentVal = std::stof(currentStr);
        double val =  floor((currentVal * 1000)+.5)/10.;
        tempStr.Form("%3.1f &", val);
        toWrite = tempStr.Data();
      }
      else toWrite = currentStr + " &";
    }
    toWrite.pop_back();
    texFile<<std::setw(12)<<toWrite<<"\\\\"<<endl;
    toWrite = "";
  }
  texFile<<"\\end{tabular}"<<endl;

}

