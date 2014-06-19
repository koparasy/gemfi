#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

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



Fi_System *fi_system;

Fi_System::Fi_System(Params *p)
  :MemObject(p)
{

  std:: stringstream s1;
  in_name = p->input_fi;
  setcheck(p->check_before_init);
  setswitchcpu(p->fi_switch);
  fi_enable = 0;
  fi_system = this;
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

  allthreads = NULL;
  
  if(in_name.size() > 1){
    input.open (in_name.c_str(), ifstream::in);
    getFromFile(input);
    input.close();
  }

 

}
Fi_System::~Fi_System(){
  
}

void
Fi_System::init()
{

  if (DTRACE(FaultInjection)) {
    std::cout<< "StartTime: %lf\n"<<starttime;
    std::cout << "Fi_System:init()\n";
  }
}


void 
Fi_System:: dump(){
  InjectedFault *p;
  
  if (DTRACE(FaultInjection)) {
    std::cout <<"===Fi_System::dump()===\n";
    std::cout << "Input: " << in_name << "\n";
    
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
  dump();
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
Fi_System:: getFromFile(std::ifstream &os){
	string check;
	
	while(os.good()){
		os>>check;		
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
		else{
		  if (DTRACE(FaultInjection)) {
		    std::cout << "No such Object: "<<check<<"\n";
		  }
		}
	}
}




//delete all info and restart from the begining

void 
Fi_System::delete_faults(){
  
//   while(!mainInjectedFaultQueue.empty())
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

  //remove faults from Queue
  delete_faults();
  
  
  
  allthreads = NULL;
  
  if(in_name.size() > 1){
    if (DTRACE(FaultInjection)) {
       std::cout << "Fi_System::Reading New Faults \n";
    }
    input.open (in_name.c_str(), ifstream::in);
    getFromFile(input);
    input.close();
    
    if (DTRACE(FaultInjection)) {
      std::cout << "~Fi_System::Reading New Faults \n";
    }
  }

  dump();
  
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


/*



void
Fi_System:: add_altered_int_reg(int reg){
      std::map<int,bool>::iterator inserted;
      inserted = intregs.find(reg);
      if(inserted==intregs.end()){
	DPRINTF(FaultInjection,"Begin to Monitor register %d\n",reg);
	intregs[reg] = true;
	return;
      }
      else
	inserted->second=true;
      return;
}

void
Fi_System:: add_altered_float_reg(int reg){
      std::map<int,bool>::iterator inserted;
      inserted = floatregs.find(reg);
      if(inserted==floatregs.end()){
	floatregs[reg] = true;
	return;
      }
      else
	inserted->second=true;
      return;
}

void
Fi_System:: add_altered_misc_reg(int reg){
      std::map<int,bool>::iterator inserted;
      inserted = miscregs.find(reg);
      if(inserted==miscregs.end()){
	miscregs[reg] = true;
	return;
      }
      else
	inserted->second=true;
      return;
}


bool
Fi_System:: altered_int_reg(int reg){
  std::map<int,bool>::iterator inserted;
  inserted = intregs.find(reg);
  if(inserted==intregs.end())
    return false;
  else
    return true;
}


bool
Fi_System:: altered_float_reg(int reg){
  std::map<int,bool>::iterator inserted;
  inserted = floatregs.find(reg);
  if(inserted==floatregs.end())
    return false;
  else
    return true;
}


bool
Fi_System:: altered_misc_reg(int reg){
  std::map<int,bool>::iterator inserted;
  inserted = miscregs.find(reg);
  if(inserted==miscregs.end())
    return false;
  else
    return true;
}

void
Fi_System::monitor_propagation(const int type, ThreadContext *tc, StaticInst *si, int idx,const Addr addr){
  bool fault = false;
  Addr _tmpAddr = tc->readMiscReg(AlphaISA::IPR_PALtemp23);
  fi_activation_iter = fi_activation.find(_tmpAddr);
  if (fi_activation_iter != fi_activation.end()){
    int reg = si->srcRegIdx(idx);
    if(type == RegisterInt )
      fault = altered_int_reg(reg);
    else if(type == RegisterFloat){
      fault = altered_float_reg(reg-FP_Base_DepTag);
    }
    else if(type == RegisterMisc)
      fault = altered_misc_reg(reg);
    
    if(fault){
//       printStack();
      DPRINTF(FaultInjection,"Register Fault Propagated to instruction : %s PCAddr:%llx PHYSICAL %llx \n",si->getName(),addr,vtophys(tc,addr));
      if(type == RegisterInt )
	intregs.erase(reg);
      else if(type == RegisterFloat)
	floatregs.erase(reg-FP_Base_DepTag);
      else if(type == RegisterMisc)
	miscregs.erase(reg);
      
	scheduleswitch(tc);
    }
  }
  return;
}

void
Fi_System::stop_monitoring_propagation(const int type, ThreadContext *tc,StaticInst *si, int idx){
  Addr _tmpAddr = tc->readMiscReg(AlphaISA::IPR_PALtemp23);
  fi_activation_iter = fi_activation.find(_tmpAddr);
  if (fi_activation_iter != fi_activation.end()){
    int reg = si->destRegIdx(idx);
    if(type == RegisterInt && altered_int_reg(reg)){
      intregs.erase(reg);
      scheduleswitch(tc);
    }
    else if(type == RegisterFloat && altered_float_reg(reg-FP_Base_DepTag)){
      floatregs.erase(reg-FP_Base_DepTag);
      scheduleswitch(tc);
    }
    else if(type == RegisterMisc && altered_misc_reg(reg)){
      miscregs.erase(reg);
      scheduleswitch(tc);
    }
  }
  
  return;
}
*/
void
Fi_System::scheduleswitch(ThreadContext *tc){
  if(getswitchcpu()){
    Fi_SystemEvent *switching = new Fi_SystemEvent(tc);
    switching->setticks(curTick());
    mainEventQueue[0]->schedule(switching,curTick()+(tc->getCpuPtr()->clockEdge(Cycles(1)))*	1000,false);
    setswitchcpu(false);
  }
}


void Fi_System::start_fi(ThreadContext *tc,  uint64_t threadid){
  Addr _tmpAddr  = TheISA::getFiThread(tc);
  fi_activation_iter = fi_activation.find(_tmpAddr);
  if (fi_activation_iter == fi_activation.end()  ) {
    std::string _name = tc->getCpuPtr()->name();
        if(allthreads == NULL){
	allthreads = new ThreadEnabledFault(-1, _name);
    }
    DPRINTF(FaultInjection,"==Fault Injection Activation Instruction===\n");
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
	tc->setEnabledFIThread(fi_activation_iter->second);
	tc->setEnabledFI(true);
      }
      else{
	DPRINTF(FaultInjection,"I have already enabled fault injection I am going to ignore this request\n");
      }
  }
}


void Fi_System::pause_fi(ThreadContext *tc,uint64_t threadid)
{
  Addr _tmpAddr  = TheISA::getFiThread(tc);
  fi_activation_iter = fi_activation.find(_tmpAddr);
  if (fi_activation_iter == fi_activation.end()) {
 	DPRINTF(FaultInjection,"I have not enabled fault injection going to ignore stop request\n");
  }
  else{ 
    fi_activation_iter->second->setMode(PAUSE);
    tc->setEnabledFI(false);
    fi_enable--;
    tc->setEnabledFIThread(NULL);
  }
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
   // ThreadEnabledFault *temp = fi_activation[_tmpAddr];
    fi_activation[_tmpAddr]->print_time();
    tc->setEnabledFI(false);
    tc->setEnabledFIThread(NULL);
    fi_activation.erase(fi_activation_iter);
    DPRINTF(FaultInjection,"~===Fault Injection Deactivation Instruction===\n");
    //delete temp;
    
    if(getswitchcpu())
      scheduleswitch(tc);
    fi_enable--;
  }

}


void Fi_System::dump_fi(ThreadContext *tc){
	for( fi_activation_iter = fi_activation.begin(); fi_activation_iter!=fi_activation.end() ; ++fi_activation_iter){
		DPRINTF(FaultInjection, " Thread ID : %llx\n",fi_activation_iter->first);
		fi_activation_iter->second->print_time();
	}
	fi_activation.clear();
	tc->setEnabledFIThread(NULL);
	tc->setEnabledFI(false);
	fi_enable = 0;

}
