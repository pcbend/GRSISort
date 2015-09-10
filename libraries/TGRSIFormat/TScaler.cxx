
#include "TScaler.h"

ClassImp(TScalerData)
ClassImp(TScaler)

TScalerData::TScalerData(){
   Clear();
	fScaler.resize(4); //we expect to have four scaler values
}

TScalerData::TScalerData(const TScalerData& rhs) {
  ((TScalerData&)rhs).Copy(*this);
}

void TScalerData::Copy(TObject &rhs) const {
  ((TScalerData&)rhs).fTimeStamp       =  fTimeStamp;      
  ((TScalerData&)rhs).fScaler          =  fScaler;        
  ((TScalerData&)rhs).fLowTimeStamp    =  fLowTimeStamp;   
  ((TScalerData&)rhs).fHighTimeStamp   =  fHighTimeStamp;  
}

void TScalerData::SetTimeStamp() {
   Long64_t time = GetHighTimeStamp();
   time  = time << 28;
   time |= GetLowTimeStamp() & 0x0fffffff;
   fTimeStamp = time;
}

void TScalerData::Clear(Option_t* opt) {
//Clears the TScalerData and leaves it a "junk" state. By junk, I just mean default
//so that we can tell that this Scaler is no good.
   fTimeStamp        =  0;
   fLowTimeStamp     =  0;
   fHighTimeStamp    =  0;
   fScaler.clear();
}

void TScalerData::Print(Option_t* opt) const {
  printf("time: %16lld",GetTimeStamp());
  for(size_t i = 0; i < fScaler.size(); ++i) {
	 printf("\t Scaler[%lu]: 0x%07x", i, fScaler[i]);
  }
  printf("\n");
}

TScaler::TScaler(){
   this->Clear();
}

TScaler::TScaler(const TScaler& rhs){
   rhs.Copy(*this);
}

TScaler::~TScaler(){
   Clear();
	for(auto addrIt = fScalerMap.begin(); addrIt != fScalerMap.end(); ++addrIt) {
		for(auto dataIt = addrIt->second.begin(); dataIt != addrIt->second.end(); ++dataIt) { 
			if(dataIt->second != NULL) {
				delete (dataIt->second);
			}
			dataIt->second = 0;
		}
	}
}

void TScaler::Copy(TObject &obj) const {
	((TScaler&)obj).Clear();
   ((TScaler&)obj).fTimePeriod =  fTimePeriod;
   ((TScaler&)obj).fNumberOfTimePeriods = fNumberOfTimePeriods;
   ((TScaler&)obj).fTotalTimePeriod =  fTotalTimePeriod;
   ((TScaler&)obj).fTotalNumberOfTimePeriods = fTotalNumberOfTimePeriods;

   //We want to provide a copy of each of the data in the Scaler rather than a copy of the pointer
	for(auto addrIt = fScalerMap.begin(); addrIt != fScalerMap.end(); ++addrIt) {
		for(auto dataIt = addrIt->second.begin(); dataIt != addrIt->second.end(); ++dataIt) { 
			if(dataIt->second != NULL) {
				((TScaler&)obj).AddData(addrIt->first, dataIt->second);
			}
		}
	}
}

Bool_t TScaler::MapIsEmpty() const {
//Checks to see if the scaler map is empty. We need this because we need to put a default
//Scaler in at time T=0 to prevent bad things from happening. This function says the map
//is empty when only the default is there, which it essentially is.
   return (fScalerMap.size() == 1); //We check for size 1 because we always start with an empty event at time 0.
}

void TScaler::AddData(UInt_t address, TScalerData* scaler){
//Adds a Scaler status word for a given address at a given time in the current run. 
//Makes a copy of the pointer to store in the map.
   fScalerMap[address].insert(std::make_pair(scaler->GetTimeStamp(),new TScalerData(*scaler)));
   fTimePeriod.clear();
   fNumberOfTimePeriods.clear();
   fTotalTimePeriod = 0;
   fTotalNumberOfTimePeriods.clear();
}

std::vector<UInt_t> TScaler::GetScaler(UInt_t address, ULong64_t time) const {
//Returns the vector of scaler values for address "address" at the time "time".
   if(MapIsEmpty()){
      printf("Empty\n");
   }
   //Check that this address exists
	if(fScalerMap.find(address) == fScalerMap.end()) {
		return std::vector<UInt_t>();
	}
   //The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one because we want to know what the last Scaler event was.
   return (--(fScalerMap.find(address)->second.upper_bound(time)))->second->GetScaler();
}

UInt_t TScaler::GetScaler(UInt_t address, ULong64_t time, size_t index) const {
//Returns the "index"th scaler value for address "address" at the time "time".
   if(MapIsEmpty()){
      printf("Empty\n");
   }
   //Check that this address exists
	if(fScalerMap.find(address) == fScalerMap.end()) {
		return 0;
	}
   //if the time is before our first entry, we return zero
	if(fScalerMap.find(address)->second.upper_bound(time) == fScalerMap.find(address)->second.begin()) {
		return 0;
	}
   //The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one because we want to know what the last Scaler event was.
   return (std::prev(fScalerMap.find(address)->second.upper_bound(time)))->second->GetScaler(index);
}

UInt_t TScaler::GetScalerDifference(UInt_t address, ULong64_t time, size_t index) const {
//Returns the difference between "index"th scaler value for address "address" at the time "time" and the previous scaler value.
   if(MapIsEmpty()){
      printf("Empty\n");
   }
   //Check that this address exists
	if(fScalerMap.find(address) == fScalerMap.end()) {
		return 0;
	}
   //if this is the before the first scaler, we just return zero
	if(fScalerMap.find(address)->second.upper_bound(time) == fScalerMap.find(address)->second.begin()) {
		return 0;
	}
   //if this is the first scaler, we just return it's value
	if(std::prev(fScalerMap.find(address)->second.upper_bound(time)) == fScalerMap.find(address)->second.begin()) {
      //The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one because we want to know what the last Scaler event was.
		return (std::prev(fScalerMap.find(address)->second.upper_bound(time)))->second->GetScaler(index);
	}
   //The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one because we want to know what the last Scaler event was.
   return (std::prev(fScalerMap.find(address)->second.upper_bound(time)))->second->GetScaler(index) - (std::prev(fScalerMap.find(address)->second.upper_bound(time),2))->second->GetScaler(index);
}

void TScaler::Print(Option_t *opt) const{
   if(MapIsEmpty()){
      printf("Empty\n");
   } else {
	   for(auto addrIt = fScalerMap.begin(); addrIt != fScalerMap.end(); ++addrIt) {
			printf("*****************************\n");
			printf("        Scaler 0x%08x        \n",addrIt->first);
			printf("*****************************\n");
		   for(auto dataIt = addrIt->second.begin(); dataIt != addrIt->second.end(); ++dataIt) { 
				dataIt->second->Print();
			}
      }
   }
}

void TScaler::Clear(Option_t *opt){
	for(auto addrIt = fScalerMap.begin(); addrIt != fScalerMap.end(); ++addrIt) {
		for(auto dataIt = addrIt->second.begin(); dataIt != addrIt->second.end(); ++dataIt) { 
			if(dataIt->second != NULL) {
				delete (dataIt->second);
			}
			dataIt->second = 0;
		}
	}
	fScalerMap.clear();
   fTimePeriod.clear();
   fNumberOfTimePeriods.clear();
   fTotalTimePeriod = 0;
   fTotalNumberOfTimePeriods.clear();
}

void TScaler::Streamer(TBuffer &R__b)
{
   // Stream an object of class TScaler.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TScaler::Class(),this);
   } else {
      R__b.WriteClassBuffer(TScaler::Class(),this);
   }
}

ULong64_t TScaler::GetTimePeriod(UInt_t address) {
//Get time period of scaler readouts for address "address" by calculating all time differences and choosing the one that occurs most often.
//Returns 0 if the address doesn't exist in the map.
	if(fScalerMap.find(address) == fScalerMap.end()) {
		return 0;
	}
   if(fTimePeriod[address] == 0) {
		//start loop at second element
		for(auto it = ++(fScalerMap[address].begin()); it != fScalerMap[address].end(); ++it) { 
			//compare timestamp of current element with that of the previous element
			ULong64_t diff = it->first - std::prev(it)->first;
			fNumberOfTimePeriods[address][diff]++;
      }
      int counter = 0;
      for(auto it = fNumberOfTimePeriods[address].begin(); it != fNumberOfTimePeriods[address].end(); ++it){
         if(it->second > counter) {
            counter = it->second;
            fTimePeriod[address] = it->first;
         }
      }
   }

   return fTimePeriod[address];
}

ULong64_t TScaler::GetTimePeriod() {
//Get time period of scaler readouts for all addresses found by calculating all time differences and choosing the one that occurs most often.
   if(fTotalTimePeriod == 0) {
      //loop over all addresses in the ScalerMap
		for(auto addrIt = fScalerMap.begin(); addrIt != fScalerMap.end(); ++addrIt) {
			//check if we've already got the time differences for this address, if not, we create them by asking for the time period for this address
			if(fNumberOfTimePeriods.find(addrIt->first) == fNumberOfTimePeriods.end()) {
				GetTimePeriod(addrIt->first);
			}
			//add these time differences (we can't use map::insert, because that skips all existing entries)
			for(auto it = fNumberOfTimePeriods[addrIt->first].begin(); it != fNumberOfTimePeriods[addrIt->first].end(); ++it) {
				fTotalNumberOfTimePeriods[it->first] += it->second;
			}
		}
      //now we have all time differences, so we can select the one that occurs most often
      int counter = 0;
      for(auto it = fTotalNumberOfTimePeriods.begin(); it != fTotalNumberOfTimePeriods.end(); ++it){
         if(it->second > counter) {
            counter = it->second;
            fTotalTimePeriod = it->first;
         }
      }
   }

   return fTotalTimePeriod;
}

Long64_t TScaler::Merge(TCollection *list){
   TIter it(list);
   TScaler* scaler = NULL;

   while((scaler = (TScaler*)it.Next())) {
      *this += *scaler;
   }
   
   return 0;

}

void TScaler::operator+=(const TScaler& rhs){                           
   this->Add(&rhs);    
}

void TScaler::Add(const TScaler* scaler){
	for(auto addrIt = scaler->fScalerMap.begin(); addrIt != scaler->fScalerMap.end(); ++addrIt) {
		for(auto dataIt = addrIt->second.begin(); dataIt != addrIt->second.end(); ++dataIt) { 
			if(dataIt->second != NULL) {
				AddData(addrIt->first, dataIt->second);
			}
		}
	}
   //We want to rebuild our cycle length map and this is the easiest way to do it
   fNumberOfTimePeriods.clear();
   fTimePeriod.clear();
   fTotalNumberOfTimePeriods.clear();
   fTotalTimePeriod = 0;
}
