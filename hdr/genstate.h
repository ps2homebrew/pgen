#ifndef _GENSTATE_H
#define _GENSTATE_H

#define state_save_register_UINT8(mod, ins, name, val, size) \
  state_transfer8(mod, name, ins, val, size);
#define state_save_register_UINT16(mod, ins, name, val, size) \
  state_transfer16(mod, name, ins, val, size);
#define state_save_register_UINT32(mod, ins, name, val, size) \
  state_transfer32(mod, name, ins, val, size);

#define state_save_register_INT8(mod, ins, name, val, size) \
  state_transfer8(mod, name, ins, val, size);
#define state_save_register_INT16(mod, ins, name, val, size) \
  state_transfer16(mod, name, ins, val, size);
#define state_save_register_INT32(mod, ins, name, val, size) \
  state_transfer32(mod, name, ins, val, size);

#define state_save_register_int(mod, ins, name, val) \
  state_transfer32(mod, name, ins, val, 1);

#define state_save_register_func_postload(a) a();

#endif
