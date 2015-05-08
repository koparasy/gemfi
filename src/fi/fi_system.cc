#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <dmtcp.h>
#include "cpu/o3/cpu.hh"
#include "cpu/base.hh"

#include "fi/faultq.hh"
#include "fi/cpu_threadInfo.hh"
#include "fi/fi_system.hh"
#include "fi/o3cpu_injfault.hh"
#include "fi/cpu_injfault.hh"
#include "fi/genfetch_injfault.hh"
#include "fi/iew_injfault.hh"
// #include "fi/mem_injfault.hh"
#include "fi/opcode_injfault.hh"
#include "fi/pc_injfault.hh"
#include "fi/regdec_injfault.hh"
#include "fi/reg_injfault.hh"
#include "fi/loadstore_injfault.hh"
#include "sim/pseudo_inst.hh"
#include "arch/vtophys.hh"

#include "config/the_isa.hh"
#include "base/types.hh"
#include "arch/types.hh"
#include "arch/registers.hh"
#include "base/trace.hh"

#include "mem/mem_object.hh"


#include <sys/time.h>  

// #include <dmtcpaware.h>


using namespace std;

unsigned fi_IntRegs;
unsigned fi_FloatRegs;
unsigned fi_CCRegs;

Fi_System *fi_system;

  Fi_System::Fi_System(Params *p)
:MemObject(p)
{

  std:: stringstream s1;
  in_name = p->input_fi;
  meta_file=p->meta_file;
  text_start=p->text_start;
  setcheck(p->check_before_init);
  setCheckBeforeFI(p->checkBeforeFI);
  setswitchcpu(p->fi_switch);
  fi_enable = 0;
  fi_system = this;
  BinaryFile = p->FileType; 
  setmaincheckpoint(false);
  mainInjectedFaultQueue.setName("MainFaultQueue");
  mainInjectedFaultQueue.setHead(NULL);
  mainInjectedFaultQueue.setTail(NULL);
  fetchStageInjectedFaultQueue.setName("FetchStageFaultQueue");
  fetchStageInjectedFaultQueue.setHead(NULL);
  fetchStageInjectedFaultQueue.setTail(NULL);
  decodeStageInjectedFaultQueue.setName("DecodeStageFaultQueue");
  decodeStageInjectedFaultQueue.setHead(NULL);
  decodeStageInjectedFaultQueue.setTail(NULL);
  iewStageInjectedFaultQueue.setName("IEWStageFaultQueue");
  iewStageInjectedFaultQueue.setHead(NULL);
  iewStageInjectedFaultQueue.setTail(NULL);
  LoadStoreInjectedFaultQueue.setName("LoadStoreFaultQueue");
  LoadStoreInjectedFaultQueue.setHead(NULL);
  LoadStoreInjectedFaultQueue.setTail(NULL);
  fi_execute = false;
  fi_decode= false; 
  fi_fetch= false;
  fi_loadstore=false;
  fi_main= false;
  totalNumFaults=0;

  allthreads = NULL;

  if(in_name.size() > 1){
    if ( BinaryFile ){
      input.open (in_name.c_str(), ifstream::in|ios::binary);
      if (input.good())
	      getFromBinaryFile(input);
    }
    else{
      if (input.good())
      input.open (in_name.c_str(), ifstream::in);
      getFromFile(input);
    }
    input.close();
  }




}
Fi_System::~Fi_System(){

}

  void
Fi_System::init()
{

  if (DTRACE(FaultInjection)) {
    std::cout << "Fi_System:init()\n";
  }
}


void 
Fi_System:: dump(){
  InjectedFault *p;

  if (DTRACE(FaultInjection)) {
    std::cout <<"===Fi_System::dump()===\n";
    std::cout << "Input: " << in_name << "\n";
    std::cout<<  "MetaData Input " << meta_file << "Is EMTPY : "<<meta_file.empty()<<"\n";
    for(fi_activation_iter = fi_activation.begin(); fi_activation_iter != fi_activation.end(); ++fi_activation_iter){
      fi_activation_iter->second->dump();
    }


    p=mainInjectedFaultQueue.head;
    while(p){

      p->dump();
      p=p->nxt;
    }

    p=fetchStageInjectedFaultQueue.head;
    while(p){
      p->dump();
      p=p->nxt;
    }

    p=decodeStageInjectedFaultQueue.head;
    while(p){
      p->dump();
      p=p->nxt;
    }

    p=iewStageInjectedFaultQueue.head;
    while(p){
      p->dump();
      p=p->nxt;
    }

    p=LoadStoreInjectedFaultQueue.head;
    while(p){
      p->dump();
      p=p->nxt;
    }
    std::cout <<"~===Fi_System::dump()===\n"; 
  }
}

  void
Fi_System::startup()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "Fi_System:startup()\n";
  }
  //dump();
}

  Fi_System *
Fi_SystemParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "Fi_System:create()\n";
  }
  return new Fi_System(this);
}

  Port *
Fi_System::getPort(const string &if_name, int idx)
{
  std::cout << "Fi_System:getPort() " << "if_name: " << if_name << " idx: " << idx <<  "\n";
  panic("No Such Port\n");
}

//Initialize faults from a file
//Note that the conditions of how the faults are
//stored in a file are very strict.
void
Fi_System:: getFromBinaryFile(std::ifstream &os){
  do{
    unsigned char type;
    unsigned int time;
    unsigned char bit;
    os.read((char*) &(type), sizeof(unsigned char));
    if (os.eof())
      break;
    os.read((char*)&(time), sizeof(int));
    if (os.eof())
      break;
    os.read((char*)&(bit), sizeof(unsigned char));
    if (os.eof())
      break;
    DPRINTF(FaultInjection,"TYPE : %d\n",type);
    switch( type ){
      case 1: 
              DPRINTF(FaultInjection,"Fetch Instr: %d Bit : %d \n",time,(int) bit);
              new GeneralFetchInjectedFault(time,bit);
              break;
      case 2:
             DPRINTF(FaultInjection,"Decode Instr: %d Bit : %d \n",time,(int) bit);
             new RegisterDecodingInjectedFault(time,bit);
             break;
      case 3:
              DPRINTF(FaultInjection,"IEW Instr: %d Bit : %d \n",time,(int) bit);
             new IEWStageInjectedFault(time,bit);
             break;
      case 4:
              DPRINTF(FaultInjection,"LDS Instr: %d Bit : %d \n",time,(int) bit);
             new LoadStoreInjectedFault(time,bit);
             break;
      default:
             std::cout<<"Should Never reach here\n";
             assert(NULL);
             break;
    }
  }while ( 1 );

}



void
Fi_System:: getFromFile(std::ifstream &os){
  string check;

  while(os.good()){
    os>>check;		
    DPRINTF(FaultInjection,"I read %s\n",check);
    if(check.compare("CPUInjectedFault") ==0){
      new CPUInjectedFault(os);
    }
    else if(check.compare("InjectedFault") ==0){
      new InjectedFault(os);
    }
    else if(check.compare("GeneralFetchInjectedFault") ==0){
      new GeneralFetchInjectedFault(os);
    }
    else if(check.compare("IEWStageInjectedFault") ==0){
      new IEWStageInjectedFault(os);
    }
    // 		else if(check.compare("MemoryInjectedFault") ==0){
    // 			new MemoryInjectedFault(os);
    // 		}
    else if(check.compare("O3CPUInjectedFault") ==0){
      new O3CPUInjectedFault(os);
    }
    else if(check.compare("OpCodeInjectedFault") ==0){
      new OpCodeInjectedFault(os);
    }
    else if(check.compare("PCInjectedFault") ==0){
      new PCInjectedFault(os);
    }
    else if(check.compare("RegisterInjectedFault") ==0){
      new RegisterInjectedFault(os);
    }
    else if(check.compare("RegisterDecodingInjectedFault") ==0){
      new RegisterDecodingInjectedFault(os);
    }
    else if(check.compare("LoadStoreInjectedFault")==0){
      new LoadStoreInjectedFault(os);
    }
    else if (check.compare("END")==0){
      DPRINTF(FaultInjection,"Finished Reading Files\n");
      break;
    }
    else{ 
      assert(0&&"THIS SHOULD NEVER HAPPEN WHEN PARSING FAULTS\n");
    }
  }
}




//delete all info and restart from the begining

void 
Fi_System::delete_faults(){

  while(!mainInjectedFaultQueue.empty())
    mainInjectedFaultQueue.remove(mainInjectedFaultQueue.head);

  while(!fetchStageInjectedFaultQueue.empty())
    fetchStageInjectedFaultQueue.remove(fetchStageInjectedFaultQueue.head);

  while(!decodeStageInjectedFaultQueue.empty())
    decodeStageInjectedFaultQueue.remove(decodeStageInjectedFaultQueue.head);

  while(!iewStageInjectedFaultQueue.empty())
    iewStageInjectedFaultQueue.remove(iewStageInjectedFaultQueue.head);

  while(!LoadStoreInjectedFaultQueue.empty())
    LoadStoreInjectedFaultQueue.remove(LoadStoreInjectedFaultQueue.head);



  mainInjectedFaultQueue.setName("MainFaultQueue");
  mainInjectedFaultQueue.setHead(NULL);
  mainInjectedFaultQueue.setTail(NULL);
  fetchStageInjectedFaultQueue.setName("FetchStageFaultQueue");
  fetchStageInjectedFaultQueue.setHead(NULL);
  fetchStageInjectedFaultQueue.setTail(NULL);
  decodeStageInjectedFaultQueue.setName("DecodeStageFaultQueue");
  decodeStageInjectedFaultQueue.setHead(NULL);
  decodeStageInjectedFaultQueue.setTail(NULL);
  iewStageInjectedFaultQueue.setName("IEWStageFaultQueue");
  iewStageInjectedFaultQueue.setHead(NULL);
  iewStageInjectedFaultQueue.setTail(NULL);
  LoadStoreInjectedFaultQueue.setName("LoadStoreFaultQueue");
  LoadStoreInjectedFaultQueue.setHead(NULL);
  LoadStoreInjectedFaultQueue.setTail(NULL);
}


  void
Fi_System:: reset()
{

  std:: stringstream s1;
  totalNumFaults=0;
  delete_faults();

  if(in_name.size() > 1){
    if (DTRACE(FaultInjection)) {
      std::cout << "Fi_System::Reading New Faults \n";
    }
    if ( BinaryFile ){
      input.open (in_name.c_str(), ifstream::in|ios::binary);
      getFromBinaryFile(input);
    }
    else{
      input.open (in_name.c_str(), ifstream::in);
      getFromFile(input);
    }

    input.close();

    if (DTRACE(FaultInjection)) {
      std::cout << "~Fi_System::Reading New Faults \n";
    }
    //   dump();
  }
}



int 
Fi_System:: get_fi_fetch_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *fetch_instr , uint64_t *fetch_time ){

  *fetch_time=0;
  *fetch_instr=0;

  // Case :: specific cpu ---- specific thread
  if((p->getWhere().compare(curCpu))==0 && (p->getThread()).compare("all") != 0 && thread.getThreadId() == atoi( (p->getThread()).c_str() ) ){ // case thread_id - cpu_id
    thread.CalculateFetchedTime(curCpu,fetch_instr,fetch_time);
  }//Case :: ALL cores --- specific Thread
  else if((p->getWhere().compare("all") == 0) && (p->getThread()).compare("all") != 0 && thread.getThreadId() == atoi( (p->getThread()).c_str() )){// case thread_id - all
    thread.CalculateFetchedTime("all",fetch_instr,fetch_time);
  }//Case :: Specific Cpu --- All threads
  else if( ( (p->getWhere().compare(curCpu)) == 0  ) && (((p->getThread()).compare("all")) == 0)  ){ //case cpu_id - all
    allthreads->CalculateFetchedTime(curCpu,fetch_instr,fetch_time);
  }//Case:: All cores --- All threads
  else if( ((p->getThread()).compare("all") == 0) && ((p->getWhere().compare("all")) == 0) ){ //case all - all
    allthreads->CalculateFetchedTime("all",fetch_instr,fetch_time);
  }

  if(*fetch_time|*fetch_instr){
    if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault || p->getFaultType() == p->MemoryInjectedFault  ){
      p->setCPU(reinterpret_cast<BaseCPU *>(find(curCpu.c_str())));// I may manifest during this cycle so se the core.
      return 1;
    }
    else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
        p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
      O3CPUInjectedFault *k = reinterpret_cast<O3CPUInjectedFault*> (p);
      k->setCPU(reinterpret_cast<BaseO3CPU *>(find(curCpu.c_str())));// I may manifest during this cycle so se the core.
      return 2;
    }
  }
  return 0;
}


int 
Fi_System:: get_fi_decode_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *fetch_instr , uint64_t *fetch_time ){

  *fetch_time=0;
  *fetch_instr=0;
  int val = 0;
  // Case :: specific cpu ---- specific thread
  if((p->getWhere().compare(curCpu))==0 && (p->getThread()).compare("all") != 0 && thread.getThreadId() == atoi( (p->getThread()).c_str() ) ){ // case thread_id - cpu_id
    thread.CalculateDecodedTime(curCpu,fetch_instr,fetch_time);
    val=1;
  }//Case :: ALL cores --- specific Thread
  else if((p->getWhere().compare("all") == 0) && (p->getThread()).compare("all") != 0 && thread.getThreadId() == atoi( (p->getThread()).c_str() )){// case thread_id - all
    thread.CalculateDecodedTime("all",fetch_instr,fetch_time);
    val =2;
  }//Case :: Specific Cpu --- All threads
  else if( ( (p->getWhere().compare(curCpu)) == 0  ) && (((p->getThread()).compare("all")) == 0)  ){ //case cpu_id - all
    allthreads->CalculateDecodedTime(curCpu,fetch_instr,fetch_time);
    val=3;
  }//Case:: All cores --- All threads
  else if( ((p->getThread()).compare("all") == 0) && ((p->getWhere().compare("all")) == 0) ){ //case all - all
    allthreads->CalculateDecodedTime("all",fetch_instr,fetch_time);
    val = 4;
  }
  if(*fetch_time | *fetch_instr){
    if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault || p->getFaultType() == p->MemoryInjectedFault){
      p->setCPU(reinterpret_cast<BaseCPU *>(find(curCpu.c_str()))); // I may manifest during this cycle so se the core.
      return val;
    }
    else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
        p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
      O3CPUInjectedFault *k = reinterpret_cast<O3CPUInjectedFault*> (p);
      BaseO3CPU *v = reinterpret_cast<BaseO3CPU *>(find(curCpu.c_str())); // I may manifest during this cycle so se the core.
      k->setCPU(v);
      return val;
    }
  }
  return val;
}


int 
Fi_System:: get_fi_exec_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *exec_instr, uint64_t *exec_time  ){

  *exec_time=0;
  *exec_instr=0;
  if((p->getWhere().compare(curCpu))==0 && (p->getThread()).compare("all") != 0 && thread.getThreadId() == atoi( (p->getThread()).c_str() ) ){ // case thread_id - cpu_id
    thread.CalculateExecutedTime(curCpu,exec_instr,exec_time);
  }
  else if((p->getWhere().compare("all") == 0) && (p->getThread()).compare("all") != 0 && thread.getThreadId() == atoi( (p->getThread()).c_str() )){// case thread_id - all
    thread.CalculateExecutedTime("all",exec_instr,exec_time);
  }
  else if( ( (p->getWhere().compare(curCpu)) == 0  ) && (((p->getThread()).compare("all")) == 0)  ){ //case cpu_id - all
    allthreads->CalculateExecutedTime(curCpu,exec_instr,exec_time);
  }
  else if( ((p->getThread()).compare("all") == 0) && ((p->getWhere().compare("all")) == 0) ){ //case all - all
    allthreads->CalculateExecutedTime("all",exec_instr,exec_time);
  }
  if(*exec_time | *exec_instr){
    if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault || p->getFaultType() == p->MemoryInjectedFault){
      p->setCPU(reinterpret_cast<BaseCPU *>(find(curCpu.c_str()))); // I may manifest during this cycle so se the core.
      return 1;
    }
    else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
        p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
      O3CPUInjectedFault *k = reinterpret_cast<O3CPUInjectedFault*> (p);
      BaseO3CPU *v = reinterpret_cast<BaseO3CPU *>(find(curCpu.c_str())); // I may manifest during this cycle so se the core.
      k->setCPU(v);
      return 2;
    }
  }
  return 0;
}

int 
Fi_System:: get_fi_loadstore_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *exec_instr, uint64_t *exec_time  ){

  *exec_time=0;
  *exec_instr=0;
  if((p->getWhere().compare(curCpu))==0 && (p->getThread()).compare("all") != 0 && thread.getThreadId() == atoi( (p->getThread()).c_str() ) ){ // case thread_id - cpu_id
    thread.CalculateLoadStoreTime(curCpu,exec_instr,exec_time);
  }
  else if((p->getWhere().compare("all") == 0) && (p->getThread()).compare("all") != 0 && thread.getThreadId() == atoi( (p->getThread()).c_str() )){// case thread_id - all
    thread.CalculateLoadStoreTime("all",exec_instr,exec_time);
  }
  else if( ( (p->getWhere().compare(curCpu)) == 0  ) && (((p->getThread()).compare("all")) == 0)  ){ //case cpu_id - all
    allthreads->CalculateLoadStoreTime(curCpu,exec_instr,exec_time);
  }
  else if( ((p->getThread()).compare("all") == 0) && ((p->getWhere().compare("all")) == 0) ){ //case all - all
    allthreads->CalculateLoadStoreTime("all",exec_instr,exec_time);
  }
  if(*exec_time | *exec_instr){
    if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault || p->getFaultType() == p->MemoryInjectedFault){
      p->setCPU(reinterpret_cast<BaseCPU *>(find(curCpu.c_str()))); // I may manifest during this cycle so se the core.
      return 1;
    }
    else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
        p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault ||
        p->getFaultType() == p->LoadStoreInjectedFault){
      O3CPUInjectedFault *k = reinterpret_cast<O3CPUInjectedFault*> (p);
      BaseO3CPU *v = reinterpret_cast<BaseO3CPU *>(find(curCpu.c_str())); // I may manifest during this cycle so se the core.
      k->setCPU(v);
      return 2;
    }
  }
  return 0;
}

int 
Fi_System::increaseTicks(std :: string curCpu , ThreadEnabledFault *curThread , uint64_t ticks){
  curThread->increaseTicks(curCpu, ticks);
  allthreads->increaseTicks(curCpu, ticks);
  return 1;
}



void
Fi_System::scheduleswitch(ThreadContext *tc){
  if(getswitchcpu()){
    Fi_SystemEvent *switching = new Fi_SystemEvent(tc);
    switching->setticks(curTick());
    Tick when = curTick() + 10000 * SimClock::Int::ns;
    mainEventQueue[0]->schedule(switching,when,true);
    DPRINTF(FaultInjection,"EVENT IS scheduled for %i\n",when);
    setswitchcpu(false);
  }
}


void Fi_System::start_fi(ThreadContext *tc,  uint64_t threadid){
  Addr _tmpAddr  = TheISA::getFiThread(tc);
  fi_activation_iter = fi_activation.find(_tmpAddr);
  static int number_of_starts=0;
  DPRINTF(FaultInjection,"Got start (#%d) Reguest: Thread_id:%llx, Task_id:%d\n",++number_of_starts,_tmpAddr, threadid);
  if (fi_activation_iter == fi_activation.end()  ) {
    std::string _name = tc->getCpuPtr()->name();
    if(allthreads == NULL){
      allthreads = new ThreadEnabledFault(-1, _name);
    }
    DPRINTF(FaultInjection,"==Fault Injection Activation Instruction===\n");
    DPRINTF(FaultInjection,"== Addr %llx   =============================\n",_tmpAddr);
    ThreadEnabledFault *thread = new ThreadEnabledFault(threadid,_name);
    fi_activation[_tmpAddr] = thread;
    tc->setEnabledFIThread( thread );
    thread->setMode(START);
    tc->setEnabledFI(true);
    fi_enable++;
    fi_execute = fi_enable & (!iewStageInjectedFaultQueue.empty());
    fi_decode= fi_enable & (!decodeStageInjectedFaultQueue.empty());
    fi_fetch= fi_enable & (!fetchStageInjectedFaultQueue.empty());
    fi_loadstore= fi_enable & (!LoadStoreInjectedFaultQueue.empty());
    fi_main= fi_enable & (!mainInjectedFaultQueue.empty() );
    DPRINTF(FaultInjection,"~==Fault Injection Activation Instruction===\n");
  }
  else{
    if	(fi_activation_iter->second->getMode() == PAUSE ){
      fi_enable++;
      fi_activation_iter->second->setMode(START);
      fi_activation_iter->second->setfaulty(0);
      tc->setEnabledFIThread(fi_activation_iter->second);
      tc->setEnabledFI(true);
      fi_activation_iter->second->setThreadId(threadid);
    }
    else{
      DPRINTF(FaultInjection,"I have already enabled fault injection I am going to ignore this request\n");
    }
  }
}


void Fi_System::pause_fi(ThreadContext *tc,uint64_t threadid)
{
  static int number_of_pauses=0;
  Addr _tmpAddr  = TheISA::getFiThread(tc);
  fi_activation_iter = fi_activation.find(_tmpAddr);
  DPRINTF(FaultInjection,"Got Pause (#%d) Reguest: Thread_id:%llx, Task_id:%d\n",++number_of_pauses,_tmpAddr, threadid);
  tc->getEnabledFIThread()->print_time();
  tc->getEnabledFIThread()->reset_counters();
  if (fi_activation_iter == fi_activation.end()) {
    DPRINTF(FaultInjection,"I have not enabled fault injection going to ignore stop request\n");
  }
  else{ 
    fi_activation_iter->second->setMode(PAUSE);
    tc->setEnabledFI(false);
    fi_enable--;
    tc->setEnabledFIThread(NULL);
  }
  //  if( (number_of_pauses++)%100 == 99){
  //    DPRINTF(FaultInjection,"Paused one more time %d\n",number_of_pauses);
  // }
}

void Fi_System:: stop_fi(ThreadContext *tc, uint64_t req){
  if(DTRACE(FaultInjection))
  {
    std::cout<<"==Fault Injection Deactivation Instruction===\n";
  }

  Addr _tmpAddr  = TheISA::getFiThread(tc);
  DPRINTF(FaultInjection, "\t Process Control Block(PCB) Addressx: %llx ####%d#####\n",_tmpAddr,threadid);  
  fi_activation_iter = fi_activation.find(_tmpAddr);

  if (fi_activation_iter != fi_activation.end()) {
    fi_activation[_tmpAddr]->print_time();
    tc->setEnabledFI(false);
    tc->setEnabledFIThread(NULL);
    fi_activation.erase(fi_activation_iter);
    DPRINTF(FaultInjection,"~===Fault Injection Deactivation Instruction===\n");
    fi_enable--;
  }
}


void Fi_System::dump_fi(ThreadContext *tc){
  DPRINTF(FaultInjection, "Dumping number of instructions\n");
  for( fi_activation_iter = fi_activation.begin(); fi_activation_iter!=fi_activation.end() ; ++fi_activation_iter){
    DPRINTF(FaultInjection, " Thread ID : %llx\n",fi_activation_iter->first);
    fi_activation_iter->second->print_time();
  }
  fi_activation.clear();
  DPRINTF(FaultInjection,"DUMPING NUMBER OF INSTRUCTIONS OF ALL THREADS\n");
  allthreads->print_time();
  DPRINTF(FaultInjection, "Total Num Of Faults %d\n",totalNumFaults);
  tc->setEnabledFIThread(NULL);
  tc->setEnabledFI(false);
  fi_enable = 0;

  if(getswitchcpu())
    scheduleswitch(tc);
}


void Fi_System::rename_ckpt(const char* new_name){
  int num_checkpoints,num_restored;
  dmtcp_get_local_status(&num_checkpoints,&num_restored);
  const char *path = dmtcp_get_ckpt_filename();
  if (path == NULL)
    DPRINTF(FaultInjection,"PATH IS NULL\n");
  std::string new_path(path);
  int count = new_path.find_last_of("/");

  if(!(rename(new_path.substr(count+1).c_str(),new_name)))
    DPRINTF(FaultInjection," Checkpoint created (%s)\n",new_name);
  else
    cout<<"Error \n"; 
}


int Fi_System::checkpointOnFault(){
  if ( getCheckBeforeFI () )
    return dmtcp_checkpoint();
  return 0;

}
