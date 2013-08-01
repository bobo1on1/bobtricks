/*
 * bobdsp
 * Copyright (C) Bob 2013
 * 
 * bobdsp is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bobdsp is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ATOMIC_H
#define ATOMIC_H

#include "inclstdint.h"
#include "config.h"

typedef volatile uint32_t atom;

inline bool MsgCAS(atom* addr, atom oldval, atom newval)
{
#ifdef HAVE_SYNC_BOOL_COMPARE_AND_SWAP
  return __sync_bool_compare_and_swap(addr, oldval, newval);
#else
#warning compare and swap disabled
  return true; //no compare and swap, so disable
#endif
}

#endif //ATOMIC_H
