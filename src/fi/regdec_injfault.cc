#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/regdec_injfault.hh"
#include "fi/fi_system.hh"
#include "arch/registers.hh"
#include <time.h>
using namespace std;

    RegisterDecodingInjectedFault::RegisterDecodingInjectedFault( std::ifstream &os)
:O3CPUInjectedFault(os)
{
    string s;
    os>>s;
    parseRegDec(s);
    fi_system->decodeStageInjectedFaultQueue.insert(this);
    setFaultType(InjectedFault::RegisterDecodingInjectedFault);
}



RegisterDecodingInjectedFault::~RegisterDecodingInjectedFault()
{
}


const char *
RegisterDecodingInjectedFault::description() const
{
    return "RegisterDecodingInjectedFault";
}

void 
RegisterDecodingInjectedFault::dump() const
{
    if (DTRACE(FaultInjection)) {
        std::cout << "===RegisterDecodingInjectedFault::dump()===\n";
        O3CPUInjectedFault::dump();
        std::cout << "\tSrc Or Dst: " << getSrcOrDst() << "\n";
        std::cout << "\tRegister to Change: " << getRegToChange() << "\n";
        std::cout << "\tChange to Reg: " << getChangeToReg() << "\n";
        std::cout << "~==RegisterDecodingInjectedFault::dump()===\n";
    }
}

bool RegisterDecodingInjectedFault::injectInputFault(int newSrc, int numOperand, StaticInstPtr si){
    si->_srcRegIdx[numOperand] = newSrc;
    return true;
}

bool RegisterDecodingInjectedFault::injectOutputFault(int newSrc, int numOperand, StaticInstPtr si){
    si->_destRegIdx[numOperand] = newSrc;
    return true;
}


bool RegisterDecodingInjectedFault::process(StaticInstPtr staticInst)
{
    int tried=0;
    bool faultInjected=false;
    srand(time(NULL));
    DPRINTF(FaultInjection, "Setting Faulty Values to Instruction\n");
    int places = staticInst->numSrcRegs() + staticInst->numDestRegs();
    if ( places != 0 ){
        int faulty_operand = rand()%places;
        if ( faulty_operand < staticInst->numSrcRegs() ){
            setSrcOrDst(SrcRegisterInjectedFault);
            operand = faulty_operand;
        }
        else{
            operand =  faulty_operand -  staticInst->numSrcRegs() ;
        }
        getQueue()->remove(this);
        return true;
    }

    //I will not inject fault since I dont have 
    //any operands to alter, I will try to 
    //inject it on the next instruction.

    int insts = getTiming() + 1;
    Addr addr = getTiming() + (getCPU()->getContext(getTContext())->pcState().nextInstAddr() - getCPU()->getContext(getTContext())->pcState().instAddr());
    increaseTiming(0,insts,addr);
    setManifested(false);
    return false;
}

/*
   StaticInstPtr
   RegisterDecodingInjectedFault::process(StaticInstPtr staticInst)
   {
   int tried=0;
   bool faultInjected=false;
   srand(time(NULL));
   while(tried < 2 && ! faultInjected){
   if (getSrcOrDst() == RegisterDecodingInjectedFault::SrcRegisterInjectedFault&&tried < 2){
   if ( staticInst->numSrcRegs() == 0){
   tried++;
   setSrcOrDst(2);
   }
   while ( !faultInjected ){
   int regToChange=rand()%staticInst->numSrcRegs();
   if (staticInst->_srcRegIdx[regToChange] < TheISA::FP_Reg_Base ){
   int NewReg  = rand()%(TheISA::FP_Reg_Base);
   faultInjected=injectInputFault(NewReg,regToChange,staticInst);
   DPRINTF(FaultInjection , "Inject Fault To : #Src: Is Integer\n");
   }
   else if ( staticInst->_srcRegIdx[regToChange] < TheISA::CC_Reg_Base ){
   int NewReg  = rand()%(TheISA::CC_Reg_Base - TheISA::FP_Reg_Base) + FP_Reg_Base;
   faultInjected=injectInputFault(NewReg,regToChange,staticInst);
   DPRINTF(FaultInjection,"Inject Fault To : #Src: Is FLoating Point\n");
   }
   else if (  staticInst->_srcRegIdx[regToChange] > TheISA:: Segment_base_reg &&  staticInst->_srcRegIdx[regToChange] < TheISA::Segment_end_reg  ){
   int NewReg  = rand()%(TheISA::Segment_end_reg - TheISA::Segment_base_reg) + TheISA::Segment_base_reg;
   faultInjected=injectInputFault(NewReg,regToChange,staticInst);
   DPRINTF(FaultInjection, "Inject Fault To : #Src: Is MiscReg \n");
   }
   }
   }
   if (getSrcOrDst() == RegisterDecodingInjectedFault::DstRegisterInjectedFault ){
   if ( staticInst->numSrcRegs() == 0){
   tried++;
   setSrcOrDst(1);
   }

   while ( !faultInjected ){
   int regToChange=rand()%staticInst->numDestRegs();
   if (staticInst->_destRegIdx[regToChange] < TheISA::FP_Reg_Base ){
   int NewReg  = rand()%(TheISA::FP_Reg_Base);
   faultInjected=injectInputFault(NewReg,regToChange,staticInst);
   DPRINTF(FaultInjection , "Inject Fault To : #DestDest : Is Integer\n");
   }
   else if ( staticInst->_destRegIdx[regToChange] < TheISA::CC_Reg_Base ){
   int NewReg  = rand()%(TheISA::CC_Reg_Base - TheISA::FP_Reg_Base) + FP_Reg_Base;
   faultInjected=injectInputFault(NewReg,regToChange,staticInst);
   DPRINTF(FaultInjection,"Inject Fault To : #Dest:  Is FLoating Point\n" );
   }
   else if (  staticInst->_destRegIdx[regToChange] > TheISA:: Segment_base_reg &&  staticInst->_srcRegIdx[regToChange] < TheISA::Segment_end_reg  ){
   int NewReg  = rand()%(TheISA::Segment_end_reg - TheISA::Segment_base_reg) + TheISA::Segment_base_reg;
   faultInjected=injectOutputFault(NewReg,regToChange,staticInst);
   DPRINTF(FaultInjection, "Inject Fault To : #Dest Is MiscReg \n");
   }
   }
   }
   }
   if ( tried == 2)
   DPRINTF(FaultInjection,"No Operands to be injected\n");
   return staticInst;
   }
 */


    int
RegisterDecodingInjectedFault::parseRegDec(std::string s)
{
    if (DTRACE(FaultInjection)) {
        std::cout << "RegisterDecodingInjecteFault::parseRegDec()\n";
    }
    size_t pos;

    if (s.compare(0,3,"Src",0,3) == 0) {
        setSrcOrDst(RegisterDecodingInjectedFault::SrcRegisterInjectedFault);

        std::string s2 = s.substr(4);
        pos = s2.find_first_of(":");

        setRegToChange(s2.substr(0,pos));
        setChangeToReg(s2.substr(pos+1));
    }
    else if (s.compare(0,3,"Dst",0,3) == 0) {
        setSrcOrDst(RegisterDecodingInjectedFault::DstRegisterInjectedFault);

        std::string s2 = s.substr(4);
        pos = s2.find_first_of(":");

        setRegToChange(s2.substr(0,pos));
        setChangeToReg(s2.substr(pos+1));

    }
    else {
        std::cout << "RegisterDecodingInjecteFault::parseRegDec(): " << s << "\n";
        assert(0);
        return 1;
    }

    return 0;
}
