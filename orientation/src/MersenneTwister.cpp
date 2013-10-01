/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "MersenneTwister.h"

MersenneTwister::MersenneTwister() {
  Seed(5489UL);
}

MersenneTwister::MersenneTwister(unsigned long seed) {
  Seed(seed);
}

void MersenneTwister::Seed(unsigned long seed) {
  m_State[0] = seed & 0xFFFFFFFFUL;
  for (int i=1; i<N; i++) {
    m_State[i] = 1812433253UL * (m_State[i-1] ^ (m_State[i-1] >> 30)) + i;
    m_State[i] &= 0xFFFFFFFFUL;
  }
  m_Position = N;
}

void MersenneTwister::GenerateState() {
  for (int i=0; i<(N-M); i++) {
    m_State[i] = m_State[i+M] ^ Twiddle(m_State[i], m_State[i+1]);
  }
  for (int i=N-M; i<(N-1); i++) {
    m_State[i] = m_State[i+M-N] ^ Twiddle(m_State[i], m_State[i+1]);
  }
  m_State[N-1] = m_State[M-1] ^ Twiddle(m_State[N-1], m_State[0]);
  m_Position = 0;
}
