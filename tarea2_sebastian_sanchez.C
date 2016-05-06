//Global variables
const Double_t binning[75] = {0.5, 0.65, 0.82, 1.01, 1.22, 1.46, 1.72, 2.00, 2.31, 2.65, 3.00, 3.36, 3.73, 4.12, 4.54, 5.00, 5.49, 6.00, 6.54, 7.10, 7.69, 8.30, 8.95, 9.62, 10.3, 11.0, 11.8, 12.6, 13.4, 14.2, 15.1, 16.1, 17.0, 18.0, 19.0, 20.0, 21.1, 22.2, 23.4, 24.6, 25.9, 27.2, 28.7, 30.2, 31.8, 33.5, 35.4, 37.3, 39.4, 41.6, 44.0, 46.6, 49.3, 52.3, 55.6, 59.1, 63.0, 67.3, 72.0, 77.4, 83.4, 90.2, 98.0, 107.0, 118.0, 132.0, 149.0, 170.0, 198.0, 237.0, 290.0, 370.0, 500.0, 700.0, 1000.0};


void parte1(void)
{

	//Retrieve data tree from root file
	TFile * f = new TFile("data_tree.root","READ");
	TTree * t = (TTree*) f->Get("Data");


	//Now call specific instructions for exercise 1_1)
	ejercicio_1_1(t);

	//Now call specific instructions for exercise 1_2)
	ejercicio_1_2(t);
	
	//Now call specific instructions for exercise 1_3)
	ejercicio_1_3(t);

}

void ejercicio_1_1(TTree* t)
{
	
	//Histograms
	TH1F * base = new TH1F("base","base",74,binning);
	TH1F * cut1 = new TH1F("cut1","cut1",74,binning);
	TH1F * cut2 = new TH1F("cut2","cut2",74,binning);

	t -> Draw("energy>>base");
	t -> Draw("energy>>cut1","betahval>0.5");
	t -> Draw("energy>>cut2","inside_tracker==1 && trchisqx>0 && trchisqx<10 && trchisqy>0 && trchisqy<10");

	//Define Efficiency objects for both cuts
	TEfficiency * eff_cut1 = new TEfficiency( (const TH1) cut1, (const TH1) base);
	TEfficiency * eff_cut2 = new TEfficiency( (const TH1) cut2, (const TH1) base);

	//Paint the efficiencies and get graphs of them
	eff_cut1 -> Paint();
	eff_cut2 -> Paint();
	TGraphAsymmErrors * geff_cut1 = eff_cut1->GetPaintedGraph();
	TGraphAsymmErrors * geff_cut2 = eff_cut2->GetPaintedGraph();

	//Make the graphs nice
	geff_cut1->SetLineWidth(2);
	geff_cut2->SetLineWidth(2);
	
	geff_cut1->SetLineColor(kRed);
	geff_cut2->SetLineColor(kBlue);
	
	//Now set define a background and draw 
	TCanvas canvas;
	TH1F * hframe = canvas.DrawFrame(0,0,1100,1);
	geff_cut1->Draw("P");
	geff_cut2->Draw("P");
	
	//Add cosmetics...
	hframe->SetTitle("Cut Efficiency Comparison; Energy [GeV]; Efficiency");
	canvas.SetLogx();
	canvas.SetGridx();
	canvas.SetGridy();

	//Add a legend
    TLegend *leg = new TLegend(0.18,0.55,0.65,0.70,NULL,"brNDC");
    leg->SetLineColor(0);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    TLegendEntry *entry=leg->AddEntry(geff_cut1,"BetaHval > 0.5","LPE");
    TLegendEntry *entry=leg->AddEntry(geff_cut2,"Tracker & 0<Tr#Chi^{2}_{x}<10 & 0<Tr#Chi^{2}_{y}<10","LPE");
	leg->Draw();

	//Save Canvas
	canvas.SaveAs("AMS_Cuts.png");

	return;
}

void ejercicio_1_2(TTree* t)
{
	//Define a histogram, whose limits were defined by a previous exploratory analysis with TTree::Draw
	TH2F * h2 = new TH2F("h2","Energy vs [TOF charge > 0]; Charge ; Energy (GeV)",30,0,30,600,0,6000);
	
	//Fill with the postive charge cut
	t -> Draw("energy:chargetof>>h2","chargetof>0");
	
	//Get a profile histogram
	TH1D * h2charge = h2 -> ProfileX();
	h2charge -> SetTitle("Profile; Charge ; Energy (GeV)");

	//Draw Scatter Plot and profile
	TCanvas canvas;
	canvas.Divide(1,2,0.01,0);
	canvas.cd(1);

	h2 -> SetMarkerStyle(8);
	h2 -> SetMarkerSize(0.4);
	h2 -> SetMarkerColor(kRed+1);
	h2 -> Draw();
	
	gPad -> SetLogy();
	gPad -> SetGridy();
	gPad -> Modified();
	gPad -> Update();

	canvas.cd(2);
	gPad -> SetGridy();
	gStyle->SetOptStat(0);
	h2charge -> Draw("E1");

	/*
		The profile histogram has contains the mean and SD of energy for each integer chargetof bin.
		If desired this information can be retrieved for further analysis by using, e.g:

		for(int i = 1; i <= h2charge->GetNbinsX(); i++) 
		{	
			mean = h2charge -> GetBinContent(i);
			SD	 = h2charge -> GetBinError(i);
			//Do something with mean and SD
		}


	*/
	canvas.SaveAs("AMS_Energy_ChargeTOF.png");


	return;
}

void ejercicio_1_3( TTree* t)
{
	//Define a reference time (as UNIX time): 00:00:00 of August 7th, 2011, 
	Double_t time_zero = TTimeStamp(2011,8,7,0,0,0).AsDouble();
	
	//Create a 2D hitstogram with binning for 3 days, form August 7th to 9th, 2011; and a binning for energy
	TH2F * h2 = new TH2F("h2","h2",3,time_zero,time_zero + 3.*24.*3600.,  600,0,12000);

	//Fill the histogram (a lot is going to the over/underflow bins, but no one cares)
	t->Draw("energy:time>>h2");
	h2->Draw("colz");

	//Make a Projection to find the number of events for each day
	TH1D* h_day = h2->ProjectionX();
	
	//Make a profile to find the mean and SD of energy for each day
	TH1D* h_dayly_E = h2->ProfileX();

		
	//Draw profile and projection
	TCanvas canvas;
	canvas.Divide(1,2,0.01,0);
	canvas.cd(1);

	h_day ->SetLineWidth(2);
	h_day -> SetLineColor(kGreen+3);
	h_day -> SetTitle("Dayly Event Count; ;Events");
	h_day -> Draw();
	
	gPad -> SetGridy();
	gPad -> Modified();
	gPad -> Update();

	canvas.cd(2);
	gPad -> SetGridy();
	h_dayly_E -> SetLineWidth(2);
	h_dayly_E -> SetLineColor(kRed);
	h_dayly_E -> SetTitle("Dayly energy profile; Date ; Energy (GeV)");
	h_dayly_E -> GetXaxis() -> SetTimeDisplay(1);
	h_dayly_E -> GetXaxis() -> SetTimeFormat("%d-%m-%Y");
	h_dayly_E -> Draw("E1");
	
	//Just as in the previous exercise, the specific values can be obatined form the histogram by doing a loop over its bins and retrieving the bincontents
	canvas.SaveAs("AMS_dayly_energy.png");

	return;
}


void parte2(void)
{
	
	//Exercise 2_1
	TTree * t = ejercicio_2_1();	//See definition below





	return;
}

TTree * ejercicio_2_1(void)
{

	//Create a TTree to hold the Jpsi data
	TTree * t = new TTree("t","t");
	
	/* This would be the easiest way to read the CSV and store the data in a better format (a ROOT file) for further analysis
	
	   t->ReadFile("Jpsi.csv");

	But the homework says we should only read the necessary data, so let's dig into the CSV file and not-save what we won't need
	*/

	//Variables and Branches for the 2 particles 4-momenta
	Float_t p1[4];
	Float_t p2[4];

	TBranch * b_p1 = t -> Branch("p1",p1);
	TBranch * b_p2 = t -> Branch("p2",p2);


	ifstream in;
	in.open("./Jpsi.csv");
	Int_t nev = 0;

	in >> NULL;
    while (1) 
	{
		in >> NULL;
		if (!in.good()) break;
		//if (nev < 5) printf("x=%8f, y=%8f, z=%8f\n",x,y,z);
		t->
		nev++;
	}
	
	printf(" I just read  %d events\n",nev);

	in.close();




	return t;
}
