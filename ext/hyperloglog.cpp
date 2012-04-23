#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ewah/ewah.h"
#include "murmur3/murmur3.h"

typedef VALUE (ruby_method)(...);

typedef struct hyperloglog {
  short bits;
  BoolArray<uword32> *registers;
  // EWAHBoolArray<uword64> *compressed;
} HyperLogLog;

// Need to separate the reading from the writing
// Writing needs to work on an uncompressed set reading doesn't

// Register Helpers
extern "C" void hyperloglog_set_register(BoolArray<uword32> *registers, uword32 position, uword32 value) {
  uword32 bucketPos = position / 6;
  uword32 shift = 5 * (position - (bucketPos * 6));
  registers->setWord( position, static_cast<uword32>( (registers->getWord(bucketPos) & ~(0x1f << shift)) | (value << shift) ) );
}

extern "C" uword32 hyperloglog_get_register(BoolArray<uword32> *registers, uword32 position) {
  uword32 bucketPos = position / 6;
  uword32 shift = 5 * (position - (bucketPos * 6));
  return (registers->getWord(bucketPos) & (0x1f << shift)) >> shift;
}

// Hashing and Calculations 
extern "C" uword32 hyperloglog_clz(uword32 x) {
  if(x == 0) { return 0; }
  for(short i = sizeof(uword32); i >= 0; i--) {
    if(((x >> i) & 0x01) == 1) { return sizeof(uword32) - i; }
  }
  throw "Could not find a count of leading zeros.";
}

extern "C" uword32 hyperloglog_hash(VALUE element) {
  uword32 hash;                /* Output for the hash */
  uword32 seed = 23;              /* Seed value for hash */
  MurmurHash3_x86_32(RSTRING(element)->ptr, RSTRING(element)->len, seed, &hash);
  return hash;
}

// Core API
extern "C" VALUE hyperloglog_offer(VALUE self, VALUE item) {
  HyperLogLog *estimator;
  Data_Get_Struct(self, HyperLogLog, estimator);
  
  uword32 x = hyperloglog_hash(item);
  uword32 j = x >> (32 - estimator->bits);
  uword32 r = hyperloglog_clz( (x << estimator->bits) | (1 << (estimator->bits - 1)) + 1 );
  
  if(hyperloglog_get_register(estimator->registers, j) < r) {
    hyperloglog_set_register(estimator->registers, j, r);
    return Qtrue;
  } else {
    return Qfalse;
  }
}

// class function
extern "C" VALUE hyperloglog_estimate(VALUE estimator) {
  // var r_sum = 0
  // for(var j = 0; j < registers.count; j++) {
  //   r_sum += Math.pow(2, (-1 * registers.get(j)))
  // }
  // 
  // // for registerCount >= 128 (bits >= 7)
  // var alpha_m = 0.7213 / (1 + 1.079 / registers.count)
  // var estimate = alpha_m * Math.pow(registers.count, 2) * (1 / r_sum)
  // 
  // if(estimate <= (5.0/2.0) * registers.count) {
  //   // Small Range Estimate
  //   var zeros = 0.0
  //   for(var z = 0; z < registers.count; z++) {
  //     if(registers.get(z) == 0) { zeros++ }
  //   }
  //   return Math.round(registers.count * Math.log(registers.count / zeros))
  // } else if(estimate <= (1.0/30.0) * Math.pow(2, 32)) {
  //   // Intermedia Range Estimate
  //   return Math.round(estimate)
  // } else if(estimate > (1.0/30.0) * Math.pow(2, 32)) {
  //   // Large Range Estimate
  //   return Math.round( (Math.pow(-2, 32) * Math.log(1 - (estimate / Math.pow(2, 32)))) )
  // }
  
  return Qnil;
}

