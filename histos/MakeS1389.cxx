#include "TRuntimeObjects.h"
#include "TFragment.h"
#include "TSceptar.h"
#include "TChannel.h"
#include "TTigress.h"
#include "TSharc.h"
#include "TTriFoil.h"

extern "C"
void MakeFragmentHistograms(TRuntimeObjects& obj) {
  std::shared_ptr<const TFragment> frag = obj.GetFragment();
  TChannel* chan = frag->GetChannel();

  if(frag && chan) {
    
    obj.FillHistogram("channel",
		      2500, 0, 2500, frag->GetChannelNumber());
    obj.FillHistogram("channel_charge",
		      2500, 0, 2500, frag->GetChannelNumber(),
		      3000, 0, 30000, frag->GetCharge());
    obj.FillHistogram("channel_energy",
		      2500, 0, 2500, frag->GetChannelNumber(),
		      10000, 0, 10000, frag->GetEnergy());
    
    
    if(frag->GetChannelNumber()<1199) {
      if(frag->GetSegment()==0) {
        obj.FillHistogram("hpge","core_energy",
			  70, 0, 70, frag->GetArrayNumber(),
			  8000, 0, 4000, frag->GetEnergy());
        obj.FillHistogram("hpge","core_charge",
			  70, 0, 70, frag->GetArrayNumber(),
			  10000, 0, 30000, frag->GetCharge());
      }
    }else{

    }
  }
}

extern "C"
void MakeAnalysisHistograms(TRuntimeObjects& obj) {
  std::shared_ptr<TSharc> sharc = obj.GetDetector<TSharc>();
  std::shared_ptr<TTigress> tigress = obj.GetDetector<TTigress>();
  std::shared_ptr<TTriFoil> tf = obj.GetDetector<TTriFoil>();

  //static long first_ana_timestamp = 0;

  if(tf) {
  }

  if(tigress){

    for(size_t i=0;i<tigress->Size();i++) {
      TTigressHit *hit = tigress->GetTigressHit(i);
        obj.FillHistogram("tigress","energy_array",
		 	                     70, 0, 70, hit->GetArrayNumber(),
			                     8000, 0, 4000, hit->GetEnergy());
        obj.FillHistogram("tigress","doppler_array",
		 	                     70, 0, 70, hit->GetArrayNumber(),
			                     8000, 0, 4000, hit->GetDoppler(0.10));
        for(size_t j=i+1;j<tigress->Size();j++) {
          TTigressHit *hit2 = tigress->GetTigressHit(j);
          obj.FillHistogram("tigress","gam_gam",
		 	                       4000, 0, 4000, hit->GetEnergy(),
			                       4000, 0, 4000, hit2->GetEnergy());
          obj.FillHistogram("tigress","gam_gam",
		 	                       4000, 0, 4000, hit2->GetEnergy(),
			                       4000, 0, 4000, hit->GetEnergy());
          obj.FillHistogram("tigress","dop_dop",
		 	                       4000, 0, 4000, hit->GetDoppler(0.10),
			                       4000, 0, 4000, hit2->GetDoppler(0.10));
          obj.FillHistogram("tigress","dop_dop",
		 	                       4000, 0, 4000, hit2->GetDoppler(0.10),
			                       4000, 0, 4000, hit->GetDoppler(0.10));

        }
    }
    //static int counter = 0;
    //printf("on entry %i \t %lu\n",counter++,tigress->Size());
    //fflush(stdout);
    for(int i=0;i<tigress->GetAddbackMultiplicity();i++) {
      TTigressHit *hit = tigress->GetAddbackHit(i);
      //  obj.FillHistogram("tigress","addback_energy",
                         //70, 0, 70, hit->GetArrayNumber(),
    	//                     8000, 0, 4000, hit->GetEnergy());
    }
    tigress->Clear();
  }
  if(sharc){
  }
}
