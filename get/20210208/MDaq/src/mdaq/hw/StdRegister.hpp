/* =============================================================================
 * StdRegister.hpp
 * -----------------------------------------------------------------------------
 * template class mdaq::hw::Std::Register
 * Pre-defined class mdaq::hw::RegisterUINT8
 * Pre-defined class mdaq::hw::RegisterUINT16
 * Pre-defined class mdaq::hw::RegisterUINT32
 * Pre-defined class mdaq::hw::RegisterUINT64
 * Created on: 23 September 2009 at Irfu/Sedi/Lilas, CEA Saclay, F-91191, France
 * -----------------------------------------------------------------------------
 * © Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
 * -----------------------------------------------------------------------------
 * Contributors: Frederic Chateau (frederic.chateau@cea.fr)
 *               Shebli Anvar (shebli.anvar@cea.fr)
 * -----------------------------------------------------------------------------
 * This software is part of the hardware access classes for embedded systems of
 * the Mordicus Real-Time Software Development Framework.
 * -----------------------------------------------------------------------------
 * FREE SOFTWARE LICENCING
 * This software is governed by the CeCILL license under French law and abiding
 * by the rules of distribution of free software. You can use, modify and/or
 * redistribute the software under the terms of the CeCILL license as circulated
 * by CEA, CNRS and INRIA at the following URL: "http://www.cecill.info".
 * As a counterpart to the access to the source code and rights to copy, modify
 * and redistribute granted by the license, users are provided only with a
 * limited warranty and the software's author, the holder of the economic
 * rights, and the successive licensors have only limited liability. In this
 * respect, the user's attention is drawn to the risks associated with loading,
 * using, modifying and/or developing or reproducing the software by the user in
 * light of its specific status of free software, that may mean that it is
 * complicated to manipulate, and that also therefore means that it is reserved
 * for developers and experienced professionals having in-depth computer
 * knowledge. Users are therefore encouraged to load and test the software's
 * suitability as regards their requirements in conditions enabling the security
 * of their systems and/or data to be ensured and, more generally, to use and
 * operate it in the same conditions as regards security.
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 * -----------------------------------------------------------------------------
 * COMMERCIAL SOFTWARE LICENCING
 * You can obtain this software from CEA under other licencing terms for
 * commercial purposes. For this you will need to negotiate a specific contract
 * with a legal representative of CEA.
 * =============================================================================
 */

#ifndef mdaq_hw_StdRegister_hpp_INCLUDED
#define mdaq_hw_StdRegister_hpp_INCLUDED

#include "SmartRegister.hpp"
#include "MemStoragePolicy.hpp"
#include <boost/cstdint.hpp>

namespace mdaq
{
namespace hw
{
/**
 * This template class represents a plain hardware register accessed through
 * standard memory read and write.
 *
 * @note Since C++ does not support template typedefs, this class has been
 * implemented with a nested typedef (Std::Register): The "template typedef"
 * equivalent would be:
   @verbatim
template <typename T> typedef SmartRegister< T, StdAccess<T> > StdRegister;
   @endverbatim
 *
 * @tparam T The word type of the register.
 *
 * @author Shebli Anvar (Shebli.Anvar@cea.fr)
 */
template <typename T>
struct Std
{
	/// "Standard" register based on default memory access.
	typedef SmartRegister<T, MemStoragePolicy> Register;
};

//@{ @name Standard register definitions for common word types
typedef Std<uint8_t >::Register RegisterUINT8;
typedef Std<uint16_t>::Register RegisterUINT16;
typedef Std<uint32_t>::Register RegisterUINT32;
typedef Std<uint64_t>::Register RegisterUINT64;
//@}

} // namespace hw
} // namespace mdaq

#endif /* mdaq_hw_Register_hpp_INCLUDED */
