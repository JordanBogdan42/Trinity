/**
 * @file CoBoCircularBufferRegisters.h
 * @date Aug 9, 2013
 * @author sizun
 * 
 * @note SVN tag: $Id: CoBoCircularBufferRegisters.h 843 2013-08-29 08:46:46Z psizun $
 * @note Contributor(s): Patrick Sizun
 * @note 
 * @note This file is part of the GetBench software project.
 *
 * @copyright © Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 *
 * @par FREE SOFTWARE LICENCING
 * This software is governed by the CeCILL license under French law and abiding  * by the rules of distribution of free
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated by
 * CEA, CNRS and INRIA at the following URL: "http://www.cecill.info". As a counterpart to the access to the source code
 * and rights to copy, modify and redistribute granted by the license, users are provided only with a limited warranty
 * and the software's author, the holder of the economic rights, and the successive licensors have only limited
 * liability. In this respect, the user's attention is drawn to the risks associated with loading, using, modifying
 * and/or developing or reproducing the software by the user in light of its specific status of free software, that may
 * mean that it is complicated to manipulate, and that also therefore means that it is reserved for developers and
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or data
 * to be ensured and, more generally, to use and operate it in the same conditions as regards security. The fact that
 * you are presently reading this means that you have had knowledge of the CeCILL license and that you accept its terms.
 *
 * @par COMMERCIAL SOFTWARE LICENCING
 * You can obtain this software from CEA under other licencing terms for commercial purposes. For this you will need to
 * negotiate a specific contract with a legal representative of CEA.
 *
 */

#ifndef get_CoBoCircularBufferRegisters_h_INCLUDED
#define get_CoBoCircularBufferRegisters_h_INCLUDED

#include <boost/cstdint.hpp>

namespace get
{
//__________________________________________________________________________________________________
struct CoBoCircularBufferRegisters
{
	struct BufferRegisters
	{
		uint32_t memStartPtrAddr;
		uint32_t memEndPtrAddr;
		uint32_t readPtrAddr;
		uint32_t writePtrAddr;
	};

	CoBoCircularBufferRegisters()
		: interruptRate(*((uint32_t*) interruptRateAddr))
	{
		asad[0].memStartPtrAddr = UINT32_C(0x20000010);
		asad[0].memEndPtrAddr 	= UINT32_C(0x20000014);
		asad[0].readPtrAddr 	= UINT32_C(0x20000020);
		asad[0].writePtrAddr 	= UINT32_C(0x20000024);

		asad[1].memStartPtrAddr = UINT32_C(0x20000028);
		asad[1].memEndPtrAddr 	= UINT32_C(0x2000002C);
		asad[1].readPtrAddr 	= UINT32_C(0x20000048);
		asad[1].writePtrAddr 	= UINT32_C(0x2000004C);

		asad[2].memStartPtrAddr = UINT32_C(0x20000030);
		asad[2].memEndPtrAddr 	= UINT32_C(0x20000034);
		asad[2].readPtrAddr 	= UINT32_C(0x20000050);
		asad[2].writePtrAddr 	= UINT32_C(0x20000054);

		asad[3].memStartPtrAddr = UINT32_C(0x20000038);
		asad[3].memEndPtrAddr 	= UINT32_C(0x2000003C);
		asad[3].readPtrAddr 	= UINT32_C(0x20000058);
		asad[3].writePtrAddr 	= UINT32_C(0x2000005C);
	}

	BufferRegisters asad[4]; ///< Base addresses of the 4 registers of each of the 4 buffers
	uint32_t & interruptRate; ///< Number of events per CPU interrupt.
	static const uint32_t interruptRateAddr = UINT32_C(0x2000001C);
};
//__________________________________________________________________________________________________
}


#endif /* get_CoBoCircularBufferRegisters_h_INCLUDED */
