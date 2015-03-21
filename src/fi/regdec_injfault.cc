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

int
RegisterDecodingInjectedFault::inject(RegType type, int op, ThreadContext *tc){
  int baseFloatIndex = fi_IntRegs;
  int baseCCIndex = fi_IntRegs + fi_FloatRegs;
  int totalRegs = fi_IntRegs + fi_FloatRegs + fi_CCRegs;
//  int checkpoint = dmtcp_checkpoint(); 
//  if ( checkpoint == 1){
//   fi_system->rename_ckpt("decode_ckpt.dmtcp");
    tc->getEnabledFIThread()->setfaulty(1);
    int random =rand();
    DPRINTF(FaultInjection,"Random Value is %d\n",random);
    fi_system->scheduleswitch(tc);
    switch (type){
      case INT:
        return (random % (baseFloatIndex) );
        break;
      case FLOAT:
        return ( random % ( baseCCIndex - baseFloatIndex ) + baseFloatIndex);
        break;
      case MISC:
        return ( random  % (totalRegs - baseCCIndex) +baseCCIndex);
        break;
      case CC:
        return ( random  % (totalRegs - baseCCIndex) +baseCCIndex);
        break;
    }
  return op;
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
        DPRINTF(FaultInjection,"Tagged Instruction As faulty Operand: %d , Num Src %d , Num Dest %d\n",faulty_operand, staticInst->numSrcRegs(),staticInst->numDestRegs());
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
    DPRINTF(FaultInjection, "Resceduling instruction for next instruction\n");
    int insts = getTiming() + 1;
    Addr addr = getTiming() + (getCPU()->getContext(getTContext())->pcState().nextInstAddr() - getCPU()->getContext(getTContext())->pcState().instAddr());
    increaseTiming(0,insts,addr);
    setManifested(false);
    return false;
}



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
