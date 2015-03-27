from m5.params import *
from MemObject import MemObject

class Fi_System(MemObject):
  type='Fi_System'
  input_fi=Param.String("","Input File Name")
  check_before_init=Param.Bool(False, "create CheckPoint before initialize of fault injection system")
  fi_switch=Param.Bool(True,"Switch to atomic after fault injection")
  meta_file=Param.String("","Input File Name");
  text_start=Param.Int(-1,"Text section offset in binary file");
  checkBeforeFI=Param.Bool(True," Checkpoint just before injecting a fault");
  
  
