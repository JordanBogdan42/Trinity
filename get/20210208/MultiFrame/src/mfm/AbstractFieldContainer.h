/*
 * $Id: AbstractFieldContainer.h 741 2012-06-12 08:10:23Z psizun $
 * @file AbstractFieldContainer.h
 * @created 20 mars 2012
 * @author sizun
 * -----------------------------------------------------------------------------
 * © Commissariat a l'Energie Atomique et aux Energies Alternatives (CEA)
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
 * -----------------------------------------------------------------------------
 */

#ifndef mfm_AbstractFieldContainer_h_INCLUDED
#define mfm_AbstractFieldContainer_h_INCLUDED

#include <string>

namespace mfm {
//______________________________________________________________________
class Header;
class Frame;
//______________________________________________________________________
/** This class is an interface implemented by all structures containing fields.
 */
class AbstractFieldContainer
{
public:
	/** Default constructor. */
	AbstractFieldContainer() {};
	/** Accessor to the Header of the Frame the field belongs to.
	 * @return Returns a const reference to the header. */
	virtual Header const & header() const = 0;
	/** Accessor to the Frame the field belongs to.
	 * @return Returns a const reference to the frame. */
	virtual Frame const & frame() const = 0;
	/** Returns the name of the field with given offset and size.
	 * @param offset_B Absolute offset of the field within the total frame buffer, in Bytes.
	 * @param size_B Size of the field, in Bytes.
	 * @return Returns the name of the field found.
	 */
	virtual std::string fieldName(size_t const & offset_B, size_t const & size_B) const = 0;
	/** Finds a named bit field within a named field and return its position and width, in bits.
	 * @param fieldName Name of the field.
	 * @param bitFieldName Name of the bit field.
	 * @param pos_b Position of the bit field, in bits, within the field.
	 * @param width_b Width of the bit field, in bits.
	 */
	virtual void findBitField(std::string const & fieldName, std::string const & bitFieldName,
			size_t & pos_b, size_t & width_b) const = 0;
	/** Default destructor. */
	virtual ~AbstractFieldContainer() {};
	/**
	 * Returns the offset of the container w.r.t. to the beginning of the frame.
	 */
	virtual uint64_t offset_B() const = 0;
};
//______________________________________________________________________
} /* namespace mfm */
#endif /* mfm_AbstractFieldContainer_h_INCLUDED */
