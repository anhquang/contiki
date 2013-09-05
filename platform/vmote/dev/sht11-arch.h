/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *	Architecture-specific definitions for the SHT11 sensor on Tmote Sky.
 * \author
 * 	Niclas Finne <nfi@sics.se>
 */

#ifndef SHT11_ARCH_H
#define SHT11_ARCH_H

#define SHT11_ARCH_SDA	1	/* P1.5 */
#define SHT11_ARCH_SCL	0	/* P1.6 */
#define SHT11_ARCH_PWR	2	/* P1.7 */

#define	SHT11_PxDIR	DDRD
#define SHT11_PxIN	PIND
#define SHT11_PxOUT	PORTD
//#define SHT11_PxSEL	PDSEL
//#define SHT11_INIT() do{TWEN=0; SHT11_PxOUT=|BV(SHT11_ARCH_PWR); SHT11_PxOUT &=~(BV(SHT11_ARCH_SDA) | BV(SHT11_ARCH_SCL)); SHT11_PxDIR |= BV(SHT11_ARCH_PWR) |BV(SHT11_ARCH_SCL);}while(0)
#endif
