/*  x360ce - XBOX360 Controller Emulator
 *  Copyright (C) 2002-2010 Racer_S
 *  Copyright (C) 2010-2013 Robert Krawczyk
 *
 *  x360ce is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  x360ce is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with x360ce.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DINPUT8_H_
#define _DINPUT8_H_

#include <CGuid.h>
#include <vector>
#include <MinHook.h>
#include "Logger.h"

template<typename N>
void IH_CreateHookF(LPVOID pTarget, LPVOID pDetour, N* ppOriginal, const char* pTargetName)
{
	if (*ppOriginal) return;
	MH_STATUS status = MH_CreateHook(pTarget, pDetour, reinterpret_cast<void**>(ppOriginal));
	PrintLog("devreorder: CreateHook %s status %s", pTargetName, MH_StatusToString(status));
}

inline void IH_EnableHookF(LPVOID pTarget, const char* pTargetName)
{
	MH_STATUS status = MH_EnableHook(pTarget);
	PrintLog("devreorder: EnableHook %s status %s", pTargetName, MH_StatusToString(status));
}

#define IH_CreateHook(pTarget, pDetour, ppOrgiginal) IH_CreateHookF(pTarget, pDetour, ppOrgiginal, #pTarget)
#define IH_EnableHook(pTarget) IH_EnableHookF(pTarget, #pTarget)


#endif