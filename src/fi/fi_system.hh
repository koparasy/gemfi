/*
 * The usage of this class is mainly  to Communicate with the configuration 
 * script via SimObject interface while wrapping all the functionality of out tool .
 * Written : Konstantinos Parasyris
 */


#ifndef _FI_SYSTEM__
#define _FI_SYSTEM__
#include <map>
#include <utility> 
#include <iostream>
#include <fstream>


#include "config/the_isa.hh"
#include "base/types.hh"
#include "arch/types.hh"
#include "base/trace.hh"
#include "debug/FaultInjection.hh"
#include "fi/faultq.hh"
#include "fi/cpu_threadInfo.hh"
#include "fi/iew_injfault.hh"
#include "fi/cpu_injfault.hh"
#include "mem/mem_object.hh"
#include "params/Fi_System.hh"
#include "fi/genfetch_injfault.hh"
#include "fi/regdec_injfault.hh"
#include "fi/loadstore_injfault.hh"
#include "sim/core.hh"
#include "arch/stacktrace.hh"
#include "sim/pseudo_inst.hh"
#include "cpu/o3/cpu.hh"
#include "cpu/base.hh"
#include "cpu/base_dyn_inst.hh"
#include <dmtcp.h>

// #include <dmtcpaware.h>

#include "arch/decoder.hh"
#define START 1
#define STOP 2
#define PAUSE 3
#define DUMP 4




using namespace std;
using namespace TheISA;

class Fi_System;
class InjectedFaultQueue;

extern Fi_System *fi_system;


extern unsigned fi_IntRegs;
extern unsigned fi_FloatRegs;
extern unsigned fi_CCRegs;
/*
 * This file encapsulates all the functionality of our framework
 * 
 */

class Fi_System : public MemObject
{
  private :
    std::ifstream input;
    std::string in_name;
    std::string meta_file;
    int text_start;	
    Addr StartingPCAddr, StopPCAddr;
    ifstream meta_input;


    class Fi_SystemEvent : public Event
  {
    private:
      ThreadContext *tc;
      uint64_t ticks;
    public:
      void setticks(uint64_t tick){ticks=tick;}
      Fi_SystemEvent(ThreadContext *tc){
        DPRINTF(FaultInjection,"scheduling switchcpu\n");
        tc = tc;
      };
      void process(){
        DPRINTF(FaultInjection,"Switching CPU cause error was injected THEN %lld NOW %lld\n",ticks,curTick());
        PseudoInst::switchcpu(tc);

      };
  };


  public :

    // Path to the file which includes all the the injection faults

    InjectedFaultQueue mainInjectedFaultQueue;		//("Main Fault Queue");
    InjectedFaultQueue fetchStageInjectedFaultQueue;	//("Fetch Stage Fault Queue");
    InjectedFaultQueue decodeStageInjectedFaultQueue;	//("Decode Stage Fault Queue");	
    InjectedFaultQueue iewStageInjectedFaultQueue;	//("IEW Stage Fault Queue");
    InjectedFaultQueue LoadStoreInjectedFaultQueue;	//("LoadStore Fault Queue")
    /*
     * The map correlate a thread/application with the pcb address
     * the colleration is done by keeping a hash table of
     * the pcb address collerated with the index of the vector
     * where the information is going to be stored
     * 
     */

    static const int RegisterInt	= 1;
    static const int RegisterFloat 	= 2;
    static const int RegisterMisc  	= 3;


    std::map<Addr, ThreadEnabledFault*> fi_activation; //A hash table key : PCB address --- vector position
    std::map<Addr, ThreadEnabledFault*>::iterator fi_activation_iter;


    ThreadEnabledFault *allthreads;


    std::map <int,bool> intregs;
    std::map <int,bool> floatregs;
    std::map <int,bool> miscregs;
    float starttime, endtime;
    unsigned int fi_enable;
    bool fi_execute;
    bool fi_decode;
    bool fi_fetch;
    bool fi_loadstore;
    bool fi_main;

  private:

    vector<uint64_t> stackTrack;

    bool check_before_init;
    bool switchcpu;
    bool checkPointBeforeFault;
    bool maincheckpoint;

    int get_core_fetched_time(std::string Cpu,uint64_t* time,uint64_t *instr);
    int get_core_decoded_time(std::string Cpu,uint64_t* time,uint64_t *instr);
    int get_core_executed_time(std::string Cpu,uint64_t* time,uint64_t *instr);
    int get_core_loadstore_time(std::string Cpu,uint64_t* time,uint64_t *instr);

    void setcheck(bool v){check_before_init=v;};
    void setswitchcpu(bool v) {switchcpu = v;}

    void setCheckBeforeFI(bool v){checkPointBeforeFault =v;};
    bool getCheckBeforeFI() {return checkPointBeforeFault;}
    int checkpointOnFault();


    void delete_faults();

  public: 
    typedef Fi_SystemParams Params;

    const Params *params() const
    {
      return reinterpret_cast<const Params *>(_params); 
    }


    Fi_System(Params *p);
    ~Fi_System();

    /*  
        void add_altered_int_reg(int reg);
        void add_altered_float_reg(int reg);
        void add_altered_misc_reg(int reg);

        bool altered_int_reg(int reg);
        bool altered_float_reg(int reg);
        bool altered_misc_reg(int reg);
        */

    int increaseTicks(std :: string curCpu , ThreadEnabledFault *curThread , uint64_t ticks);

    int get_fi_fetch_counters(InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *exec_time , uint64_t *exec_instr );
    int get_fi_decode_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *fetch_instr , uint64_t *fetch_time );
    int get_fi_exec_counters(InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *exec_time , uint64_t *exec_instr );
    int get_fi_loadstore_counters(InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *exec_time , uint64_t *exec_instr );

    void getFromFile(std::ifstream &os);
    bool getCheck(){return check_before_init;}
    bool getswitchcpu(){return switchcpu;}
    bool getMainCheckpoint(){return maincheckpoint;}
    void setmaincheckpoint(bool v){maincheckpoint = v;}
    void reset();
    virtual Port* getPort(const std::string &if_name, int idx = 0);
    virtual void init();
    virtual void startup();


    void stop_fi(ThreadContext *tc, uint64_t id);
    void start_fi(ThreadContext *tc, uint64_t id);
    void pause_fi(ThreadContext *tc, uint64_t id);
    void dump_fi(ThreadContext *tc);

    void rename_ckpt(const char* new_name);

    /* 
       void monitor_propagation(const int type, ThreadContext *tc,StaticInst *si, int idx,const Addr addr);
       void stop_monitoring_propagation(const int type, ThreadContext *tc,StaticInst *si, int idx);
       void stackHandler(ThreadContext *tc,uint64_t NPC);
       void printStack();
       */

    void scheduleswitch(ThreadContext *tc);
    void dump();

    /*
     *  All the following function get the hardware running thread
     * and check if a fault is going to be injected during this cycle/instruction
     * Different function are created depending on the pipeline
     * stage that the fault is going to manifest.
     * Furthermore all the executed instructions are increased from this functions
     */


    template <class MYVAL, class U>
      inline MYVAL lds_fault(U ptr,ThreadContext *tc,MYVAL value,unsigned dataSize){

        if(!( tc->getEnabledFI()))
          return value;
        ThreadEnabledFault *thread = tc->getEnabledFIThread();
        LoadStoreInjectedFault *loadStoreFault = NULL;
        if( thread->getMode() == START && FullSystem  && (TheISA::inUserMode(tc))  ){
          Addr pcAddr = ptr->instAddr(); //PC address for this instruction
          int sigInstr = getSignificance(pcAddr);
          std::string _name = tc->getCpuPtr()->name();
          thread->setInstMode(sigInstr);
          allthreads->setInstMode(sigInstr);
          thread->increaseLoadStoreInstr(_name);
          allthreads->increaseLoadStoreInstr(_name);

          while ((loadStoreFault  = reinterpret_cast<LoadStoreInjectedFault *>(LoadStoreInjectedFaultQueue.scan(_name, *thread, pcAddr))) != NULL){
            if(loadStoreFault->getValueType() == InjectedFault::FlipBit && loadStoreFault->getValue() > dataSize*8){ 
              loadStoreFault->setValue(loadStoreFault->getValue()%(dataSize*8) +1);
            }
            int succeed = checkpointOnFault();
            if ( succeed == 1){
              rename_ckpt("lds_ckpt.dmtcp");
              value = loadStoreFault->process(value);
              thread->setfaulty(1);
              DPRINTF(FaultInjection,"LDS: PCAddr:%llx Fault Inserted in thread %d at instruction %s\n",pcAddr,thread->getThreadId(),ptr->getcurInstr()->getName());
              scheduleswitch(tc);
            }
            else{
              reset();
            }

          }
        }
        return value;
      }

    template <class MYVAL, class U>
      inline void iew_fault(U ptr,ThreadContext *tc,MYVAL *value){

        if(!( tc->getEnabledFI()) )
          return ;
        ThreadEnabledFault *thread = tc->getEnabledFIThread();
        IEWStageInjectedFault *iewFault = NULL;
        if( thread->getMode() == START && FullSystem && (TheISA::inUserMode(tc))   ){
          Addr pcAddr = ptr->instAddr();

          int sigInstr = getSignificance(pcAddr);

          thread->setInstMode(sigInstr);
          allthreads->setInstMode(sigInstr);

          std::string _name = tc->getCpuPtr()->name();

          thread->increaseExecutedInstr(_name);
          allthreads->increaseExecutedInstr(_name);

          while ((iewFault = reinterpret_cast<IEWStageInjectedFault *>(iewStageInjectedFaultQueue.scan(_name, *thread, pcAddr))) != NULL){
            int succeed = checkpointOnFault();
            if ( succeed == 1){
              rename_ckpt("iew_ckpt.dmtcp");
              *value = iewFault->process(*value);
              DPRINTF(FaultInjection,"IEW: PCAddr:%llx Fault Inserted in thread %d at instruction %s\n",pcAddr,thread->getThreadId(),ptr->getcurInstr()->getName());
              scheduleswitch(tc);
            }
            else
              reset();
          }
        }

        return;
      }

    inline void main_fault(ThreadContext *tc,ThreadEnabledFault *thread, Addr pcAddr){
      CPUInjectedFault *mainfault = NULL;
      std::string _name = tc->getCpuPtr()->name();

      if(thread->getMode() != START)
        return;

      int sigInstr = getSignificance(pcAddr);

      thread->setInstMode(sigInstr);
      while ((mainfault = reinterpret_cast<CPUInjectedFault *>(mainInjectedFaultQueue.scan(_name, *thread , pcAddr))) != NULL){
        int succeed = checkpointOnFault();
        if ( succeed == 1){
          rename_ckpt("fetch_ckpt.dmtcp");
          mainfault->process();
          DPRINTF(FaultInjection,"MAIN: PCAddr:%llx Fault Inserted in thread %d at instruction \n",thread->getThreadId(),pcAddr);
          if(string(mainfault->description()).compare("RegisterInjectedFault") != 0)
            scheduleswitch(tc);
        }
        else
          reset();
      }

    }	


    inline TheISA::MachInst fetch_fault(ThreadContext *tc,ThreadEnabledFault *thread, TheISA::MachInst cur_instr,Addr pcAddr){
      GeneralFetchInjectedFault *fetchfault = NULL;
      std::string _name = tc->getCpuPtr()->name();
      if(thread->getMode() != START)
        return cur_instr;
      if(FullSystem && TheISA::inUserMode(tc)){	
        thread->increaseFetchedInstr(_name);
      }
      else
        return cur_instr;	


      allthreads->increaseFetchedInstr(_name);
      int sigInstr = getSignificance(pcAddr);
      thread->setInstMode(sigInstr);
      allthreads->setInstMode(sigInstr);

      while ((fetchfault = reinterpret_cast<GeneralFetchInjectedFault *>(fetchStageInjectedFaultQueue.scan(_name, *thread, pcAddr))) != NULL){
        int succeed = checkpointOnFault();
        if ( succeed == 1){
          sigInstr = getSignificance(pcAddr);
          thread->setInstMode(sigInstr);
          allthreads->setInstMode(sigInstr);
          rename_ckpt("fetch_ckpt.dmtcp");
          cur_instr = fetchfault->process(cur_instr);
          DPRINTF(FaultInjection,"Fetch: PCAddr:%llx In thread %d Fault Inserted \n",pcAddr,thread->getThreadId());
          thread->setfaulty(1);
          scheduleswitch(tc);
        }
        else
          fi_system->reset();
      }
      return cur_instr;
    }

    inline RegisterDecodingInjectedFault* decode_fault(ThreadContext *tc,ThreadEnabledFault *thread, StaticInstPtr instr,Addr pcAddr){
      std::string _name = tc->getCpuPtr()->name();
      RegisterDecodingInjectedFault *decodefault = NULL;
      if(thread->getMode() != START)
        return NULL;

      int sigInstr = getSignificance(pcAddr);
      thread->setInstMode(sigInstr);
      allthreads->setInstMode(sigInstr);


      if(FullSystem && TheISA::inUserMode(tc)){	
        thread->increaseDecodedInstr(_name);
      }
      else
        return NULL;

      allthreads->increaseDecodedInstr(_name);
      while ((decodefault = reinterpret_cast<RegisterDecodingInjectedFault *>(decodeStageInjectedFaultQueue.scan(_name, *thread, pcAddr))) != NULL){
        DPRINTF(FaultInjection,"Decode:PCAddr:%llx Fault Inserted in thread %d at instruction %s \n",pcAddr,thread->getThreadId(),instr->getName());
        int succeed = checkpointOnFault();
        if ( succeed == 1){
          bool val = decodefault->process(instr);
          if ( val ){
            return decodefault;
          }
        }
        else{
          fi_system->reset();
        }
      }	  


      return NULL;
    }

    void setStartingPCAddr(Addr vAddr){StartingPCAddr = vAddr;}
    void setStopPCAddr(Addr vAddr) {StopPCAddr = vAddr;}
    Addr getStartingPCAddr(){return StartingPCAddr;}
    Addr getStopPCAddr(){return StopPCAddr;}
    void open_meta_file(){
      if (meta_file.empty())
        return;
      meta_input.open(meta_file.c_str(),ifstream::binary);
      if (!meta_input.is_open() ){
        DPRINTF(FaultInjection,"Cannot Open meta data file\n");
        assert(NULL);
      }
    }

    int getSignificance(Addr PC_addr){
      return 0; 
    }


};



#endif //_FI_SYSTEM
