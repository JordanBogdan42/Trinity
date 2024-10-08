/* =============================================================================
 * $Id: BlobHeader.cpp 716 2012-04-10 15:17:58Z psizun $
 * @file BlobHeader.cpp
 * @author sizun
 * @date 15 déc. 2011
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
 * =============================================================================
 */

#include "mfm/BlobHeader.h"
#include "mfm/Exception.h"

namespace mfm {
//______________________________________________________________________
/**
 * Default constructor.
 * Constructs a default header for a Blob frame, with specified byte order.
 * @param _byteOrder Byte order.
 */
BlobHeader::BlobHeader(utl::Endianness _byteOrder)
	: Header(_byteOrder)
{
    ;
}
//______________________________________________________________________
/**
 * Constructs a header for a Blob frame, by copying a PrimaryHeader.
 * @param primaryHeader Primary header to copy.
 */
BlobHeader::BlobHeader(PrimaryHeader const & primaryHeader) : Header(primaryHeader)
{
    ;
}
//______________________________________________________________________
/**
 * Copy constructor.
 */
BlobHeader::BlobHeader(BlobHeader const & header)
	: Header(header)
{
	;
}
//______________________________________________________________________
/**
 * Clones this BlobHeader.
 */
BlobHeader* BlobHeader::clone() const
{
	return new BlobHeader(*this);
}
//______________________________________________________________________
uint16_t BlobHeader::headerSize_blk() const
{
    return PrimaryHeader::SPEC_SIZE_B/blkSize_B();
}
//______________________________________________________________________
uint32_t BlobHeader::headerSize_B() const
{
    return PrimaryHeader::SPEC_SIZE_B;
}
//______________________________________________________________________
uint32_t BlobHeader::headerReserveSize_B() const
{
	return headerSize_B() - PrimaryHeader::SPEC_SIZE_B;
}
//______________________________________________________________________
uint32_t BlobHeader::dataReserveSize_B() const
{
	return 0;
}
//______________________________________________________________________
uint16_t BlobHeader::itemSize_B() const
{
    throw Exception("Operation itemSize_B() not supported by Blob frame header!");
}
//______________________________________________________________________
uint32_t BlobHeader::itemCount()  const
{
    return 1;
}
//______________________________________________________________________
void BlobHeader::set_headerSize_blk(uint16_t)
{
    throw Exception("Operation set_headerSize_blk(uint16_t) not supported by Blob frame header!");
}
//______________________________________________________________________
void BlobHeader::set_headerSize_B(uint32_t)
{
    throw Exception("Operation set_headerSize_B(uint32_t) not supported by Blob frame header!");
}
//______________________________________________________________________
void BlobHeader::set_itemSize_B(uint16_t)
{
    throw Exception("Operation set_itemSize_B(uint16_t) not supported by Blob frame header!");
}
//______________________________________________________________________
void BlobHeader::set_itemCount(uint32_t)
{
    throw Exception("Operation set_itemCount(uint32_t) not supported by Blob frame header!");
}
//______________________________________________________________________
void BlobHeader::set_dataSize_blk(uint32_t /* _dataSize_blk */)
{
    throw Exception("Operation set_dataSize_blk(uint32_t) not supported by Blob frame header!");
}
//______________________________________________________________________
void BlobHeader::set_dataSize_B(uint64_t /* _dataSize_B */)
{
    throw Exception("Operation set_dataSize_B(uint64_t) not supported by Blob frame header!");
}
//______________________________________________________________________
} /* namespace mfm */
