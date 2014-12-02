#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/regdec_injfault.hh"
#include "fi/fi_system.hh"
#include "arch/registers.hh"
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

  StaticInstPtr
RegisterDecodingInjectedFault::process(StaticInstPtr staticInst)
{
  int i;
  for( i = 0; i < staticInst->numSrcRegs() ; i++){
    DPRINTF(FaultInjection, "Src Operand %d, is %d ",i,(int) staticInst->_srcRegIdx[i]);
    if (staticInst->_srcRegIdx[i] < TheISA::FP_Reg_Base )
      DPRINTF(FaultInjection , " Is Integer\n");
    else if (staticInst->_srcRegIdx[i] < TheISA::CC_Reg_Base)
      DPRINTF(FaultInjection," Is FLoating Point\n" );
    else 
      DPRINTF(FaultInjection, "i Is MiscReg \n");
  }
  for( i = 0; i < staticInst->numDestRegs() ; i++){
    DPRINTF(FaultInjection, "Dest Operand %d, is %d\n",i,(int) staticInst->_destRegIdx[i]);
    if (staticInst->_destRegIdx[i] < TheISA::FP_Reg_Base )
      DPRINTF(FaultInjection , " Is Integer\n");
    else if (staticInst->_destRegIdx[i] < TheISA::CC_Reg_Base)
      DPRINTF(FaultInjection," Is FLoating Point\n" );
    else 
      DPRINTF(FaultInjection, " Is MiscReg \n");
  }
  int num_operands = staticInst->numDestRegs() + staticInst->numSrcRegs();
  if (!num_operands)
    return staticInst;

  int place = rand()%num_operands;

  if ( place < staticInst->numSrcRegs() )
    DPRINTF(FaultInjection,"I will Alter Src\n");
  else if ( place -staticInst->numSrcRegs() < staticInst->numDestRegs() )
    DPRINTF(FaultInjection,"I will Alter Dst\n");




  /* 
     if (getSrcOrDst() == RegisterDecodingInjectedFault::SrcRegisterInjectedFault) {
     DPRINTF(FaultInjection, "NUM SRC REGS ARE %d\n",staticInst->numSrcRegs());
     for( i = 0; i < staticInst->numSrcRegs() ; i++)
     DPRINTF(FaultInjection, "Operand %d, is %d\n",i,staticInst->_srcRegIdx[i]);
     }
     else {
     }
     if (getSrcOrDst() == RegisterDecodingInjectedFault::SrcRegisterInjectedFault) {
     int rTc = getRegToChange() % staticInst->numSrcRegs(); //Make sure the rTC field of the instruction exists
     if(staticInst->_srcRegIdx[rTc] >= TheISA::FP_Reg_Base)
     setChangeToReg(getChangeToReg()+TheISA::FP_Reg_Base);
     staticInst->_srcRegIdx[rTc] = getChangeToReg();
     }
     else {
     int rTc = getRegToChange() % staticInst->numDestRegs();
     if(staticInst->_destRegIdx[rTc] >= TheISA::FP_Reg_Base)
     setChangeToReg(getChangeToReg()+TheISA::FP_Reg_Base);
     staticInst->_destRegIdx[rTc] = getChangeToReg();
     }  

     if (DTRACE(FaultInjection)) {
     std::cout << "===RegisterDecodingInjectedFault::process()===\n";
     std::cout <<"Instruction Type : "<<staticInst->getName()<<"\n";;
     dump();
     }


     check4reschedule();

     if (DTRACE(FaultInjection)) {
     std::cout << "~==RegisterDecodingInjectedFault::process()===\n";
     }
     */
  return staticInst;
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
