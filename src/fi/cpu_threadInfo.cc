#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "fi/cpu_threadInfo.hh"
#include "cpu/o3/cpu.hh"
#include "fi/faultq.hh"
#include "fi/fi_system.hh"

std::string allcores = string("all");

//Set all the counters to the correct value
cpuExecutedTicks:: cpuExecutedTicks(std:: string name)
{
  setName(name);
  setInstrFetched(0);
  setInstrExecuted(0);
  setInstrLoadStore(0);
  setTicks(0);
}

cpuExecutedTicks::~cpuExecutedTicks()
{
  
}

void cpuExecutedTicks:: dump(){
    if (DTRACE(FaultInjection)) {
    std::cout<<"================\t"<<"CpuExecutedTicks :"<<getName()<<" \t==========================\n"; 
    std::cout << "CpuFetchedInstr: " <<getInstrFetched() <<"\n";
    std::cout << "CpuExecutedInstr: "<<getInstrExecuted() <<  "\n";
    std::cout << "CpuLoadStoreInstr "<<getInstrLoadStore()<<"\n";
    std::cout << "Ticks: "<<getTicks() << "\n";
    std::cout<<"================\t~CpuExecutedTicks~\t==========================\n";
  }
}

//a new thread has enabled fault injection get the id and set all the information
ThreadEnabledFault::ThreadEnabledFault(int threadId, std::string name)
{
  all = new cpuExecutedTicks(allcores);
  cores.insert(pair<string,cpuExecutedTicks*> (allcores,all));

  cores.insert(pair<string,cpuExecutedTicks*>(name, new cpuExecutedTicks(name)));
  
  setThreaId(threadId);
  setMyid();
  setMagicInstVirtualAddr(-1);
}

ThreadEnabledFault::~ThreadEnabledFault()
{
  delete all;
  for(itcores = cores.begin(); itcores!=cores.end() ; ++itcores)
      delete itcores->second;
  
}



void ThreadEnabledFault::dump(){

  if (DTRACE(FaultInjection)) {
    std::cout<<"================\t"<<"ThreadEnabledFault "<<getMyId()<<" \t==========================\n"; 
    std::cout << "ThreadEnabledInfo  MagicInstVirtualAddr : "<<getMagicInstVirtualAddr()<<" ThreadId :"<<getThreaId() <<"\n";
    std::cout<<"================\t~ThreadEnabledFault~\t==========================\n";
  }
}

//This thread has executed one more cycle increase it

int ThreadEnabledFault:: increaseTicks(std:: string curCpu, uint64_t ticks)
{
 
  assert(all != NULL);
  all->increaseTicks(ticks);
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
	cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
	cores[curCpu]->increaseTicks(ticks);

  return 1;
  
}


//This thread has fethed one more instruction.


int ThreadEnabledFault:: increaseFetchedInstr(std:: string curCpu)
{
  assert( all != NULL);
  all->increaseFetchInstr();
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
	cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
	cores[curCpu]->increaseFetchInstr();
  return 1;
 
}



//This thread has executed one more instruction.

int ThreadEnabledFault:: increaseExecutedInstr(std:: string curCpu)
{
  assert(all != NULL);
  all->increaseExecInstr();
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
	cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
	cores[curCpu]->increaseExecInstr();
  return 1;
}


int ThreadEnabledFault:: increaseLoadStoreInstr(std:: string curCpu)
{
  assert(all != NULL);
  all->increaseLoadStoreInstr();
  itcores = cores.find(curCpu);
  if ( itcores == cores.end())
	cores[curCpu] = new cpuExecutedTicks(curCpu);
  else
	cores[curCpu]->increaseLoadStoreInstr();
  return 1;
}


//how many instruction has this thread fetched  untill now on this core or on all cores?

void ThreadEnabledFault:: CalculateFetchedTime(std::string curCpu , uint64_t *fetched_instr , uint64_t *fetched_time )
{
  assert( all != NULL);
  if(curCpu.compare("all")==0){
      *fetched_time  =  all->getTicks();
      *fetched_instr =  all->getInstrFetched();
  }
  else{
    itcores = cores.find(curCpu);
    if(itcores != cores.end()){
      *fetched_time = itcores->second->getTicks();
      *fetched_instr = itcores->second->getInstrFetched();
    }
      return;
  }
}



void ThreadEnabledFault:: CalculateExecutedTime(std::string curCpu  , uint64_t *exec_instr , uint64_t *exec_time)
{
  assert( all != NULL);
  if(curCpu.compare("all")==0){
      *exec_time  =  all->getTicks();
      *exec_instr =  all->getInstrExecuted();
  }
  else{
    itcores = cores.find(curCpu);
    if(itcores != cores.end()){
      *exec_time = itcores->second->getTicks();
      *exec_instr = itcores->second->getInstrExecuted();
    }
  }
  return;
}



void ThreadEnabledFault:: CalculateLoadStoreTime(std::string curCpu  , uint64_t *exec_instr , uint64_t *exec_time)
{
  assert( all != NULL);
  if(curCpu.compare("all")==0){
    *exec_time  =  all->getTicks();
    *exec_instr =  all->getInstrLoadStore();
  }
  else{
    itcores = cores.find(curCpu);
    if(itcores != cores.end()){
      *exec_time = itcores->second->getTicks();
      *exec_instr = itcores->second->getInstrLoadStore();
    }
  }
  return;
}


//It is good to know how many instructions i have executed on all cores.
//The more the info the better the results.
void ThreadEnabledFault:: print_time(){
  if (DTRACE(FaultInjection)){
    std::cout<<"THREAD ID: "<<getMyId()<<"\n";
    for(itcores = cores.begin(); itcores!=cores.end() ; ++itcores){
      std::cout<<"CORE:"<<itcores->second->getName()<<"\n";
      std::cout<<"Fetched Instr: "<< itcores->second->getInstrFetched() <<"\n";
      std::cout<<"Executed Instr: "<< itcores->second->getInstrExecuted() <<"\n";
      std::cout<<"LoadStore Instr: "<< itcores->second->getInstrLoadStore() <<"\n";
      std::cout<<"Ticks : "<< itcores->second->getTicks() <<"\n";
    }
    
  }
}






