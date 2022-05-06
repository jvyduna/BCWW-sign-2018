
/* SignMessages.h - symbols for messages the signs can display
  
   Symbols for messages are in form of HBC[W|N]W
   Bit vectors are HBCWW(N?)
*/

const byte _BCWW = 0B011110; const byte BAO_CHICKA_WOW_WOW = _BCWW;
const byte HB___ = 0B110000; const byte HOT_BAO = HB___;
const byte H__N_ = 0B100101; const byte HOT_NOW = H__N_;
const byte _B_N_ = 0B010101; const byte BAO_NOW = _B_N_;
const byte HB_N_ = 0B110101; const byte HOT_BAO_NOW = HB_N_;
const byte HB_W_ = 0B110100; const byte HOT_BAO_WOW = HB_W_;
const byte HB_NW = 0B110111; const byte HOT_BAO_NOW_WOW = HB_NW;
const byte H__W_ = 0B100100; const byte HOT_WOW = H__W_;

const byte H____ = 0B100000; const byte HOT = H____;
const byte _B___ = 0B010000; const byte BAO = _B___;
const byte __C__ = 0B001000; const byte CHICKA = __C__;
const byte ___W_ = 0B000100; const byte WOW1 = ___W_;
const byte ___N_ = 0B000101; const byte NOW = ___N_;
const byte ____W = 0B000010; const byte WOW2 = ____W;

const byte _BC__ = 0B011000; const byte BAO_CHICKA = _BC__;
const byte _B_W_ = 0B010100; const byte BAO_WOW = _B_W_;
const byte _BCW_ = 0B011100; const byte BAO_CHICKA_WOW = _BCW_;
const byte ___WW = 0B000110; const byte WOW_WOW = ___WW;
const byte _B_WW = 0B010110; const byte BAO_WOW_WOW = _B_WW;
const byte _B_NW = 0B010111; const byte BAO_NOW_WOW = _B_NW;
