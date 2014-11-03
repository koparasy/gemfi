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


// #include <dmtcpaware.h>

#include "arch/decoder.hh"
#define START 1
#define STOP 2
#define PAUSE 3
#define DUMP 4

#define RELIABLE 0
#define NON_RELIABLE 1
#define NOT_DEFINED 2
#define NOT_SET 3


using namespace std;
using namespace TheISA;

class Fi_System;
class InjectedFaultQueue;


extern Fi_System *fi_system;

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
        Addr StartingPCAddr;
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
        bool maincheckpoint;

        int get_core_fetched_time(std::string Cpu,uint64_t* time,uint64_t *instr);
        int get_core_decoded_time(std::string Cpu,uint64_t* time,uint64_t *instr);
        int get_core_executed_time(std::string Cpu,uint64_t* time,uint64_t *instr);
        int get_core_loadstore_time(std::string Cpu,uint64_t* time,uint64_t *instr);

        void setcheck(bool v){check_before_init=v;};
        void setswitchcpu(bool v) {switchcpu = v;}


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
            inline MYVAL lds_fault(U ptr,ThreadContext *tc,MYVAL value){

                if(!( tc->getEnabledFI()))
                    return value;

                ThreadEnabledFault *thread = tc->getEnabledFIThread();
                LoadStoreInjectedFault *loadStoreFault = NULL;
                if( thread->getMode() == START && FullSystem  && (TheISA::inUserMode(tc))  ){
                    Addr pcAddr = ptr->instAddr(); //PC address for this instruction
                    std::string _name = tc->getCpuPtr()->name();
                    thread->write_instr_and_name(pcAddr,ptr->getcurInstr()->getName());
                    while ((loadStoreFault  = reinterpret_cast<LoadStoreInjectedFault *>(LoadStoreInjectedFaultQueue.scan(_name, *thread, pcAddr))) != NULL){
                        value = loadStoreFault->process(value);
                        DPRINTF(FaultInjection,"PCAddr:%llx Fault Inserted in instruction %s\n",pcAddr,ptr->getcurInstr()->getName());
                        scheduleswitch(tc);
                    }
                    thread->increaseLoadStoreInstr(_name);
                    allthreads->increaseLoadStoreInstr(_name);
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

                    bool sigInstr = getSignificance(pcAddr);

                    if (!sigInstr)
                        return;


                    std::string _name = tc->getCpuPtr()->name();
                    //					thread->write_instr_and_name(pcaddr,ptr->getcurInstr()->getName());

                    while ((iewFault = reinterpret_cast<IEWStageInjectedFault *>(iewStageInjectedFaultQueue.scan(_name, *thread, pcAddr))) != NULL){
                        *value = iewFault->process(*value);
                        DPRINTF(FaultInjection,"PCAddr:%llx Fault Inserted in instruction %s\n",pcAddr,ptr->getcurInstr()->getName());
                        scheduleswitch(tc);
                    }
                    //					DPRINTF(FaultInjection,"PCAddr:%llx Fault Inserted in instruction %s\n",pcaddr,ptr->getcurInstr()->getName());

                    thread->increaseExecutedInstr(_name);
                    allthreads->increaseExecutedInstr(_name);
                }
                return;
            }

        inline void main_fault(ThreadContext *tc,ThreadEnabledFault *thread, Addr pcAddr){
            CPUInjectedFault *mainfault = NULL;
            std::string _name = tc->getCpuPtr()->name();

            if(thread->getMode() != START)
                return;
            while ((mainfault = reinterpret_cast<CPUInjectedFault *>(mainInjectedFaultQueue.scan(_name, *thread , pcAddr))) != NULL){
                mainfault->process();
                if(string(mainfault->description()).compare("RegisterInjectedFault") != 0){
                    scheduleswitch(tc);
                }
            }

        }	


        inline TheISA::MachInst fetch_fault(ThreadContext *tc,ThreadEnabledFault *thread, TheISA::MachInst cur_instr,Addr pcAddr){


            GeneralFetchInjectedFault *fetchfault = NULL;
            std::string _name = tc->getCpuPtr()->name();

            if(thread->getMode() != START)
                return cur_instr;

            if(FullSystem && TheISA::inUserMode(tc)){	
                thread->increaseFetchedInstr(_name);
                thread->write_PC_address(pcAddr);
            }
            else
                return cur_instr;	


            bool sigInstr = getSignificance(pcAddr);

            if (!sigInstr)
                return cur_instr;

            allthreads->increaseFetchedInstr(_name);
            while ((fetchfault = reinterpret_cast<GeneralFetchInjectedFault *>(fetchStageInjectedFaultQueue.scan(_name, *thread, pcAddr))) != NULL){
                cur_instr = fetchfault->process(cur_instr);
                DPRINTF(FaultInjection,"PCAddr:%llx Fault Inserted \n",pcAddr);
                scheduleswitch(tc);
            }

            return cur_instr;
        }

        inline StaticInstPtr decode_fault(ThreadContext *tc,ThreadEnabledFault *thread, StaticInstPtr cur_instr,Addr pcAddr){

            std::string _name = tc->getCpuPtr()->name();
            RegisterDecodingInjectedFault *decodefault = NULL;

            if(thread->getMode() != START)
                return cur_instr;

            bool sigInstr = getSignificance(pcAddr);

            if (sigInstr)
                return cur_instr;

            while ((decodefault = reinterpret_cast<RegisterDecodingInjectedFault *>(decodeStageInjectedFaultQueue.scan(_name, *thread, pcAddr))) != NULL){
                cur_instr = decodefault->process(cur_instr);
                DPRINTF(FaultInjection,"PCAddr:%llx Fault Inserted %s \n",pcAddr,cur_instr->getName());
                scheduleswitch(tc);
            }	  


            return cur_instr;
        }


        void setStartingPCAddr(Addr vAddr){StartingPCAddr = vAddr;}
        Addr getStartingPCAddr(){return StartingPCAddr;}
        void open_meta_file(){
            meta_input.open(meta_file.c_str(),ifstream::binary);
            if (!meta_input.is_open() ){
                DPRINTF(FaultInjection,"Cannot Open meta data file\n");
                assert(NULL);
            }
        }

        bool getSignificance(Addr PC_addr){

            if(text_start < 0 )
                return false;

            int offset = PC_addr - StartingPCAddr;
            bool val;
            if (offset < 0 ){
                DPRINTF(FaultInjection, " Something is going very wrong I should never find negative offset\n");
            }
            int pos = text_start + offset;

            char byte;
            meta_input.seekg(pos,std::ios::beg);
            if (!meta_input.is_open())
                return false;
            meta_input.read(&byte,sizeof(char));

            switch (byte){
                case RELIABLE:
                    DPRINTF(FaultInjection, "Reliable Instruction\n");
                    val = true;
                    break;
                case NON_RELIABLE:
                    DPRINTF(FaultInjection, "Non-Reliable Instruction\n");
                    val = false;
                    break;
                case NOT_DEFINED :
                    DPRINTF(FaultInjection, "Compiler did not define this Instruction\n");
                    val = true;
                    break;
                case NOT_SET:
                    DPRINTF(FaultInjection, "Instruction is included from another object file\n");
                    val = true;
                    break;
                default:
                    DPRINTF(FaultInjection, " I should never reach here\n");
                    val = true;
                    break;
            }

            return val;
        }


};



#endif //_FI_SYSTEM
