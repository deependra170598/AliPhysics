//
// Basic analysis task.
//
// Basic analysis task template for analysis jets storing information in both tree
// branches and histograms


#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TTree.h>
#include <TList.h>
#include <AliAnalysisDataSlot.h>
#include <AliAnalysisDataContainer.h>
#include <TProfile.h>
#include <TChain.h>

// aliroot Headers
#include "AliAODEvent.h"
#include "AliParticleContainer.h"
#include "AliJetContainer.h"
#include "AliMCEvent.h"
#include "AliEmcalJet.h"

//My Header
#include "AliAnalysisTaskJetChargeFlavourTemplates.h"

//Globals
using std::cout;
using std::endl;



ClassImp(AliAnalysisTaskJetChargeFlavourTemplates)

//________________________________________________________________________
AliAnalysisTaskJetChargeFlavourTemplates::AliAnalysisTaskJetChargeFlavourTemplates() :
  AliAnalysisTaskEmcalJet("AliAnalysisTaskJetChargeFlavourTemplates", kTRUE),
  fContainer(0),
  pChain(0),
  fPtThreshold(-9999.),
  fCentSelectOn(kTRUE),
  fCentMin(0),
  fCentMax(10),
  fJetRadius(0),
  JetChargeK(0.5),
  Pt(-999),
  Phi(-999),
  Eta(-999),
  JetCharge(-999),
  ParticlePt(-999),
  ParticlePhi(-999),
  ParticleEta(-999),
  ParticleJetCharge(-999),
  LeadingTrackPt(-999),
  PdgCode(0),
  ProgenetorFraction(-1),

  fTreeJets(0)
{

}

//________________________________________________________________________
AliAnalysisTaskJetChargeFlavourTemplates::AliAnalysisTaskJetChargeFlavourTemplates(const char *name) :
  AliAnalysisTaskEmcalJet(name, kTRUE),
  fContainer(0),
  pChain(0),
  fPtThreshold(-9999.),
  fCentSelectOn(kTRUE),
  fCentMin(0),
  fCentMax(10),
  fJetRadius(0),
  JetChargeK(0.5),
  Pt(-999),
  Phi(-999),
  Eta(-999),
  JetCharge(-999),
  ParticlePt(-999),
  ParticlePhi(-999),
  ParticleEta(-999),
  ParticleJetCharge(-999),
  LeadingTrackPt(-999),
  PdgCode(0),
  ProgenetorFraction(-1),




  fTreeJets(0)
{

  SetMakeGeneralHistograms(kTRUE);
  DefineOutput(1, TList::Class());
  DefineOutput(2, TTree::Class());
}

//________________________________________________________________________
AliAnalysisTaskJetChargeFlavourTemplates::~AliAnalysisTaskJetChargeFlavourTemplates()
{
  // Destructor.
}

//________________________________________________________________________
 void AliAnalysisTaskJetChargeFlavourTemplates::UserCreateOutputObjects()
{
  // Echo jet radius
  //Info("TaskJets","Using jet radius R=%f",fJetRadius);

  // Create user output.
  AliAnalysisTaskEmcalJet::UserCreateOutputObjects();
  Bool_t oldStatus = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);
  TH1::AddDirectory(oldStatus);
  //create output TTree
  const char* nameoutput = GetOutputSlot(2)->GetContainer()->GetName();
  fTreeJets = new TTree(nameoutput, nameoutput);
  // Names for the branches

  // Name the branches of your TTree here

  fTreeJets->Branch("Pt",&Pt,"Pt/F");
  fTreeJets->Branch("Phi",&Phi,"Phi/F");
  fTreeJets->Branch("Eta",&Eta,"Eta/F");
  fTreeJets->Branch("JetCharge",&JetCharge,"JetCharge/F");
  fTreeJets->Branch("ParticlePt",&ParticlePt,"ParticlePt/F");
  fTreeJets->Branch("ParticlePhi",&ParticlePhi,"ParticlePhi/F");
  fTreeJets->Branch("ParticleEta",&ParticleEta,"ParticleEta/F");
  fTreeJets->Branch("ParticleJetCharge",&ParticleJetCharge,"ParticleJetCharge/F");
  fTreeJets->Branch("LeadingTrackPt",&LeadingTrackPt,"LeadingTrackPt/F");
  fTreeJets->Branch("PdgCode",&PdgCode,"pdgcode/I");
  fTreeJets->Branch("ProgenetorFraction",&ProgenetorFraction,"ProgenetorFraction/F");







  // Make sure that the outputs get written out
  PostData(1,fOutput);
  PostData(2,fTreeJets);
  // delete [] fShapesVarNames;
}

//________________________________________________________________________
Bool_t AliAnalysisTaskJetChargeFlavourTemplates::Run()
{
  // Run analysis code here, if needed. It will be executed before FillHistograms().




  return kTRUE;
}

//________________________________________________________________________
Bool_t AliAnalysisTaskJetChargeFlavourTemplates::FillHistograms()
{
  // Centrality selection, if enabled
  if (fCentSelectOn){
    if ((fCent>fCentMax) || (fCent<fCentMin))
      return 0;
  }

  // Reset the Tree Parameters
  Pt = -999;
  Phi = -999;
  Eta = -999;
  JetCharge = -999;
  ParticlePt  = -999;
  ParticlePhi = -999;
  ParticleEta = -999;
  ParticleJetCharge = -999;
  LeadingTrackPt = -999;
  PdgCode = 0;
  ProgenetorFraction = -1;

  // Initialise Jet shapes
  Int_t fPdgCodes[100] = {};
  Int_t fCurrentPdg = 0;
  Int_t nMothers = 0;
  Float_t jetCharge = 0;
  Float_t jetChargeParticle = 0;

  Int_t fParticleUniqueID[100] = {};
  Int_t fCurrentParticleUniqueID = 0;


  // Initialise jet pointer
  //cout << "Running Fill Histograms" << endl;
  AliEmcalJet *Jet1 = NULL; //Original Jet in the event                                                                                                         // Get jet container (0 = ?)
  AliJetContainer *JetCont= GetJetContainer(0); //Jet Container for event
  AliEmcalJet *TruthJet = NULL; //Original Jet in the event                                                                                                         // Get jet container (0 = ?)
  AliJetContainer *JetGen= GetJetContainer(1); //Jet Container for event


  AliParticleContainer *MCParticleContainer = JetGen->GetParticleContainer();
  Int_t nAcceptedJets = JetCont->GetNAcceptedJets();
  //TClonesArray *trackArr = dynamic_cast<TClonesArray*>(InputEvent()->FindListObject("HybridTracks"));
  Float_t JetPhi=0;
  Float_t JetParticlePhi=0;
  Float_t JetPt_ForThreshold=0;

  if(JetCont) {

    for (auto Jet1 : JetCont->accepted()) {
      // ...
    
      if(!Jet1)
      {

        continue;
      }

      // Jet is above threshold?

      //cout << "Running A Jet" << endl;

      // Get the jet constituents

      //AliParticleContainer *fMCContainer = GetParticleContainer(1);
      //UInt_t nMCConstituents = fMCContainer->GetNParticles();

      //AliParticleContainer * MCTracks  = dynamic_cast<AliParticleContainer *> (GetParticleContainer("embeddedTracks"));
      //UInt_t nMCTracks = MCTracks->GetNParticles();

      /*
      TIter nextPartColl(&fParticleCollArray);
      AliParticleContainer* tracks = 0;
      while ((tracks = static_cast<AliParticleContainer*>(nextPartColl())))
      {
      AliParticleContainer* fMCContainer = tracks;
      }
      */

      AliParticleContainer *fTrackCont = JetCont->GetParticleContainer();
      UInt_t nJetConstituents = Jet1->GetNumberOfTracks();



      //cout << nTest << "::::" << nMCConstituents << "::::" << nJetConstituents << endl;


      // Must have at least two constituents
      if( nJetConstituents < 2 )
      {
        continue;
      }


      JetPt_ForThreshold = Jet1->Pt();

      if(JetPt_ForThreshold<fPtThreshold)
      {

        continue;
      }
      else {

        //Check the jet leading track is Greater than 5 GeV to reduce cominatorial jets

        if(Jet1->GetLeadingTrack()->Pt() < 5)
        {
            continue;
        }

        LeadingTrackPt = Jet1->GetLeadingTrack()->Pt();

        // Initialising the Tagged PYTHIA Jet.
        Bool_t kHasTruthJet = kFALSE;
        AliEmcalJet* TruthJet;
        UInt_t nTruthConstituents;

        if(Jet1->ClosestJet() != NULL)
        {
          kHasTruthJet = kTRUE;
          TruthJet = Jet1->ClosestJet();
          nTruthConstituents = TruthJet->GetNumberOfTracks();

        }


      


        //Finding Pdg Code using TRUTH Jet.


        if(kHasTruthJet)
        {

          if(TruthJet->Pt()<fPtThreshold)
          {

            continue;
          }

          //std::cout <<"Leading Track Pt" <<  Jet1->GetLeadingTrack()->Pt()<< std::endl;



          // Filling the Tree here

          JetPhi=Jet1->Phi();
          if(JetPhi < -1*TMath::Pi())
            JetPhi += (2*TMath::Pi());
          else if (JetPhi > TMath::Pi())
            JetPhi -= (2*TMath::Pi());

          JetParticlePhi=TruthJet->Phi();
          if(JetParticlePhi < -1*TMath::Pi())
            JetParticlePhi += (2*TMath::Pi());
          else if (JetParticlePhi > TMath::Pi())
            JetParticlePhi -= (2*TMath::Pi());

          // Filling the TTree branches here
          Pt          = Jet1->Pt();
          Phi         = JetPhi;
          Eta         = Jet1->Eta();

          ParticlePt  = TruthJet->Pt();
          ParticlePhi = JetParticlePhi;
          ParticleEta = TruthJet->Eta();


          // Identifing PDG Codes of Mothers
          for (UInt_t iTruthConst = 0; iTruthConst < nTruthConstituents; iTruthConst++ )
          {
            AliMCParticle* TruthParticle = (AliMCParticle*) TruthJet->Track(iTruthConst);
            AliMCParticle *MotherParticle = (AliMCParticle*)  MCParticleContainer->GetParticle(TruthParticle->GetMother());
            AliMCParticle *OneSetBackParticle = (AliMCParticle*)  MCParticleContainer->GetParticle(TruthParticle->GetMother());
            
            //cout << "Start: " << endl;
            while(MotherParticle->GetMother() > 0)
            {
              MotherParticle = (AliMCParticle*) MCParticleContainer->GetParticle(MotherParticle->GetMother());

              //cout << "MotherParticle PDG: " << MotherParticle->PdgCode() << endl;
              //cout << "MotherParticle Label: " << MotherParticle->Label() << endl;
              
              if(MotherParticle->PdgCode() == 2212)
              {
                //cout << "Proton Label: " <<MotherParticle->Label() << endl;
                //cout << "One Down Label: " << OneSetBackParticle->Label() << endl;
                MotherParticle  = OneSetBackParticle;
                break;
              }
              else
              {
              OneSetBackParticle = MotherParticle;           
              }
              
            }

            //cout << "End;" << endl << endl;

            // Insure this isnt a beam proton
            
            fCurrentParticleUniqueID = MotherParticle->GetLabel();
            fCurrentPdg = MotherParticle->PdgCode();
            fPdgCodes[nMothers] = fCurrentPdg;            
            fParticleUniqueID[nMothers] = fCurrentParticleUniqueID;
            //cout << fPdgCodes[nMothers] << endl;
            nMothers++;
          
          }

          Int_t UniquePdgCodes[20] = {};            //To be filled, maximium is that there are  20 uniques
          Float_t UniquePdgFrequency[20] = {};
          Int_t nUniques = 0;

          //Loop of PDG code found
          for(int i = 0; i < nMothers; i++)
          {
            
            // Consider one pdgCode at the time.
            fCurrentPdg = fPdgCodes[i];
            // Loop over unique arry to be filled
            
            for(int j = 0; j < nMothers; j++)
            {

              // If it hasnt matched and the current unique value is empty list the new value and increment frequncy by 1 and then break
              if(UniquePdgCodes[j] != fCurrentPdg && UniquePdgCodes[j] == 0)
              {
                UniquePdgCodes[j] = fCurrentPdg;
                UniquePdgFrequency[j] = UniquePdgFrequency[j] + 1.;
                nUniques ++;
                break;
              }

              //Check if the PDG is already lsited if it matched increase the frequency counter by 1
              else if(UniquePdgCodes[j] == fCurrentPdg)
              {
                UniquePdgFrequency[j] = UniquePdgFrequency[j] + 1.;
                break;
              }
              // Otherwise the PDG hasnt matched and the value isnt zero check the next value

            }
          }

          // Setting Final Pdg Code
          // normalising frequency array

          for(unsigned int i = 0; i < nUniques; i++)
          {
            UniquePdgFrequency[i] = UniquePdgFrequency[i]/nMothers;
          }

          
         // Loop to store largest number And corresponding pdg code
          Float_t CurrentFraction;

          int IndexOfMaximum = -1;
          CurrentFraction = UniquePdgFrequency[0];
          for(unsigned int i = 0;i < nUniques; ++i) 
          {
            if(UniquePdgFrequency[i] >= CurrentFraction)
            {
              IndexOfMaximum = i;
              CurrentFraction = UniquePdgFrequency[i];
            }
          }

          ProgenetorFraction = CurrentFraction;
          fCurrentPdg = fPdgCodes[IndexOfMaximum];
          
          // Now Caluclate the jet Charge



          // Loop over the consituents
          for (UInt_t iJetConst = 0; iJetConst < nJetConstituents; iJetConst++ )
          {
            AliVParticle *JetParticle = Jet1->Track(iJetConst);
            jetCharge += JetParticle->Charge()*pow(JetParticle->Pt(),JetChargeK);
          }

          for (UInt_t iTruthConst = 0; iTruthConst < nTruthConstituents; iTruthConst++ )
          {
            AliMCParticle* TruthParticle = (AliMCParticle*) TruthJet->Track(iTruthConst);
            //Divided by 3 since its parton level and in units of e/3
            jetChargeParticle += TruthParticle->Charge()/3*pow(TruthParticle->Pt(),JetChargeK);
          }



          // Normalise the Non Flavoured Jet CHarge
          jetCharge/=pow(Jet1->Pt(),0.5);

          // Normalise Particle level jet charge
          jetChargeParticle/=pow(TruthJet->Pt(),0.5);


          //Put The Jet Charge in the right place
          JetCharge = jetCharge;
          ParticleJetCharge = jetChargeParticle;

          PdgCode = fCurrentPdg;


          //Outputs for Checking
          /*
          cout << "Final PDG CODE: " << fCurrentPdg << endl;
          cout << "Progenetor Fraction: " << ProgenetorFraction << endl;

          //Output Frequency of Uniques
          cout << "Frequency of Uniques: [" ;
          for(int i = 0; i < nUniques; i++)
          {
            cout << UniquePdgFrequency[i] << "," ;
          }
          cout << "] " << endl;
                   
          //Output for checking the list of raw pdgCodes
          cout << "Pdg Codes: [" ;
          for(int i = 0; i < nMothers; i++)
          {
            cout << fPdgCodes[i] << "," ;
          }
          cout << "] " << endl;

          //Outputs for checking Raw ParticleUniqueIDs
          cout << "Particle Label: [" ;
          for(int i = 0; i < nMothers; i++)
          {
            cout << fParticleUniqueID[i] << "," ;
          }
          cout << "] " << endl;
          
          */
          



          fTreeJets->Fill();




      }

      //cout << "End of Jet" << endl;
      }
    }
  }
  return kTRUE;
}



//________________________________________________________________________
Bool_t AliAnalysisTaskJetChargeFlavourTemplates::RetrieveEventObjects() {
  //
  // retrieve event objects
  //
  if (!AliAnalysisTaskEmcalJet::RetrieveEventObjects())
    return kFALSE;

  return kTRUE;
}


//_______________________________________________________________________
void AliAnalysisTaskJetChargeFlavourTemplates::Terminate(Option_t *)
{
  // Called once at the end of the analysis.


  // Normalise historgrams over number of Jets considered

}
