enum opcodes {
    /* R-Type */
  OP_RType=0b000000,
  /* I-Type */
  OP_addi=0b001000, 
  OP_lwcl=0b110001,
  OP_swcl=0b111001,
  OP_j=0b000010,
  OP_beq=0b000100,
  /* F-Type */
  OP_FType=0b010001,
  NUM_OPCODES
};

/* MIPS only has 6 bits for the opcode, which isn't enough to specify
   all of the implemented instructions.  To work around this, several
   instructions share the same "OP_SPECIAL" opcode.  To differentiate
   these instructions, we use the 11 bit function field.  The func values
   for these instructions are defined here.
   Max function range: 0 - 2047
*/

enum functions {
  FUNC_jr=0b001000,
  FUNC_mul_s=0b000010,
  FUNC_add_s=0b000000,

/* !!! Do *NOT* remove this entry !!!
   This is used to delimit the end of the function array, so we can check
   to see if you try to define more that 2048 functions!
*/
  NUM_FUNCS
};

