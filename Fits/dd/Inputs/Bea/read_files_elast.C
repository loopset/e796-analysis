#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH2.h"

#include <fstream>
#include <iostream>

TGraph* dgrth;
TGraph* pgrth;
TGraphErrors* dgrexp;
TGraphErrors* pgrexp;

void read_files_elast()
{


    Double_t I, tdensd, tdensp, frac;
    Int_t i;


    // DEUTERONS

    TGraphErrors* deutgr = new TGraphErrors("deut_elas_distrib_exp_nanv.dat", "%lg %lg %lg %lg");
    Int_t ndeut = 0;
    ndeut = deutgr->GetN();
    cout << "n" << ndeut << endl;
    Double_t* Nd = new Double_t[ndeut];
    Nd = deutgr->GetX();
    Double_t* thcmd = new Double_t[ndeut];
    thcmd = deutgr->GetY();
    Double_t* sangdd = new Double_t[ndeut];
    sangdd = deutgr->GetEX();
    Double_t* jacobdd = new Double_t[ndeut];
    jacobdd = deutgr->GetEY();

    Double_t* xsd = new Double_t[ndeut];
    Double_t* exsd = new Double_t[ndeut];


    cout << "Input I= " << endl;
    cin >> I;
    cout << "Input frac= " << endl;
    cin >> frac;

    tdensd =
        2 * frac * 6.0221367e23 / (12. + 2. * (frac * 2.0141077785 + (1 - frac) * 1.00782503207)) * 0.59e-3 * 1e-27;

    for(i = 0; i < ndeut; i++)
    {
        // cout <<"xsd= " << jacobdd[i] <<" sangdd= " << sangdd[i]<<endl;
        xsd[i] = (jacobdd[i] * Nd[i]) / (sangdd[i] * I * tdensd);
        exsd[i] = (1. / sqrt(Nd[i])) * xsd[i];
        // cout <<"xsd= " << xsd[i] <<" therd= " << thcmd[i]<<endl;
    }


    dgrexp = new TGraphErrors(ndeut, thcmd, xsd, 0, exsd);
    dgrexp->SetName("xs_g0");
    dgrexp->SaveAs("20O_dd_gs.root");


    /// PROTONS

    TGraphErrors* protgr = new TGraphErrors("prot_elas_distrib_exp_nanv.dat", "%lg %lg %lg %lg");

    Int_t nprot;
    nprot = protgr->GetN();
    Double_t* Np = new Double_t[nprot];
    Np = protgr->GetX();
    Double_t* thcmp = new Double_t[nprot];
    thcmp = protgr->GetY();
    Double_t* sangpp = new Double_t[nprot];
    sangpp = protgr->GetEX();
    Double_t* jacobpp = new Double_t[nprot];
    jacobpp = protgr->GetEY();
    Double_t* xsp = new Double_t[nprot];
    Double_t* exsp = new Double_t[nprot];


    tdensp = 2 * (1 - frac) * 6.0221367e23 / (12. + 2. * (frac * 2.0141077785 + (1 - frac) * 1.00782503207)) * 0.59e-3 *
             1e-27;

    for(i = 0; i < nprot; i++)
    {
        xsp[i] = (jacobpp[i] * Np[i]) / (sangpp[i] * I * tdensp);
        exsp[i] = (1. / sqrt(Np[i])) * xsp[i];
        // cout <<"xsp= " << xsp[i] <<" thera= " << thcmp[i]<<endl;
    }

    pgrexp = new TGraphErrors(nprot, thcmp, xsp, 0, exsp);


    // THEORETICAL RESULTS


    TGraphErrors* dgrthr = new TGraphErrors("o20_meurders_mb.cm", "%lg %lg %lg");

    Int_t dpoint = dgrthr->GetN();
    Double_t* thcmdth = new Double_t(dpoint);
    thcmdth = dgrthr->GetX();
    Double_t* xsdth = new Double_t(dpoint);
    xsdth = dgrthr->GetEY();

    dgrth = new TGraph(dpoint, thcmdth, xsdth);


    // PROTONS
    TGraphErrors* pgrthr = new TGraphErrors("o20_pUNC_mb.cm", "%lg %lg %lg");

    Int_t ppoint = pgrthr->GetN();
    Double_t* thcmpth = new Double_t(ppoint);
    thcmpth = pgrthr->GetX();
    Double_t* xspth = new Double_t(ppoint);
    xspth = pgrthr->GetEY();

    pgrth = new TGraph(ppoint, thcmpth, xspth);


    /// PLOT

    TCanvas* c1 = new TCanvas("c1", "xs", 200, 10, 700, 500);
    // gr->SetXTitle("Run Number");
    // gr->SetYTitle("NcLDC1");
    c1->SetLogy();


    TH1F* vFrame = c1->DrawFrame(20.0, 1, 55.0, 1000);
    // vFrame->SetTitle("photopeak efficiency (%)");  //2nd excited state
    vFrame->SetXTitle("#theta_{CM}(deg)");
    vFrame->GetXaxis()->SetLabelSize(0.035);
    vFrame->GetXaxis()->SetTitleSize(0.05);
    vFrame->GetXaxis()->SetTitleOffset(0.8);
    vFrame->SetYTitle("d#sigma/d#Omega (mb/sr)");
    vFrame->GetYaxis()->SetLabelSize(0.04);
    vFrame->GetYaxis()->SetTitleSize(0.05);


    // Deuterons
    dgrexp->SetMarkerStyle(20);
    dgrexp->Draw("samep");
    dgrth->Draw("samel");

    // Protons
    pgrexp->SetMarkerStyle(23);
    pgrexp->SetMarkerColor(2);
    pgrexp->Draw("samep");
    pgrth->SetLineColor(2);
    pgrth->Draw("samel");

    TLegend* legend = new TLegend(0.65, 0.71, 0.90, 0.83);
    legend->SetTextFont(60);
    legend->SetTextSize(0.035);
    legend->SetFillColor(0);
    legend->SetBorderSize(0);
    legend->AddEntry(dgrexp, " d", "p");
    legend->AddEntry(dgrth, " Meurders", "l");
    legend->AddEntry(pgrexp, " p", "p");
    legend->AddEntry(pgrth, " UNC", "l");

    legend->Draw();


    TPaveText* pt = new TPaveText(0.01, 0.930088, 0.0607882, 0.995, "blNDC");
    pt->SetName("title");
    pt->SetBorderSize(0);
    pt->SetFillColor(0);
    TText* text = pt->AddText("I=1.20339e9");
    text = pt->AddText("f=0.7682");
    pt->Draw();
    // c1->Update();
}
