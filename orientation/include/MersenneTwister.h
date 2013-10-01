/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#ifndef __MersenneTwister_h__
#define __MersenneTwister_h__

/// <summary>
/// Simple implementation of Mersenne Twister RNG
/// </summary>
/// <remarks>
/// This is a relatively simple implementation of the commonly-used Mersenne Twister for generating random numbers.
/// Original paper by M. Matsumoto and T. Nishimura, and this implementation was based on one by J. Bedaux.
///
/// Maintainers: Raffi
/// </remarks>

class MersenneTwister
{

public:

  MersenneTwister();
  MersenneTwister(unsigned long seed);
  void Seed(unsigned long seed);

  inline unsigned long GetInt() {
    // returns a random 32-bit integer
    if (m_Position == N) {
      GenerateState();
    }
    unsigned long x = m_State[m_Position++];
    x ^= (x >> 11);
    x ^= (x << 7) & 0x9D2C5680UL;
    x ^= (x << 15) & 0xEFC60000UL;
    return x ^ (x >> 18);
  }

  inline double GetDouble() {
    // returns a random floating-point value on the interval [0,1)
    return static_cast<double>(GetInt() * (1.0 / 4294967296.0));
  }

private:

  inline unsigned long Twiddle(unsigned long u, unsigned long v) {
    return (((u & 0x80000000UL) | (v & 0x7FFFFFFFUL)) >> 1) ^ ((v & 1UL) ? 0x9908B0DFUL : 0x0UL);
  }

  void GenerateState();

  static const int      N = 624;
  static const int      M = 397;
  unsigned long         m_State[N];
  int                   m_Position;

};

#endif
