/*
 * $Id: BlobFrameTest.cpp 740 2012-06-05 14:59:08Z psizun $
 * @file BlobFrameTest.cpp
 * @created 22 mars 2012
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
#include "mfm/Frame.h"
#include "mfm/FrameFormat.h"
#include "mfm/FrameDictionary.h"
#include "mfm/Exception.h"
using namespace mfm;
#include "tut.h"
#include <string>
#include <fstream>
using namespace std;

#ifndef VXWORKS
#define TEMP_DIR "/tmp/"
#else
#define TEMP_DIR "tmp/"
#endif

namespace tut
{
///////////////////////////////////////////////////////////////////////////////
// test fixture definition
///////////////////////////////////////////////////////////////////////////////
struct blob_frame_test_data
{
};

typedef test_group<blob_frame_test_data> testgroup;
typedef testgroup::object testobject;
testgroup blob_frame_testgroup("BlobFrame");

///////////////////////////////////////////////////////////////////////////////
// test methods body
///////////////////////////////////////////////////////////////////////////////

/**
 * Checks creation of a blob frame from its CompoundConfig format.
 */
template<> template<>
void testobject::test<1>()
{
    set_test_name("Checks creation of a blob frame from its CompoundConfig format");

    FrameFormat format("mfm/sample_formats.xml", "BlobSample", 0);
    ensure_equals("Name of format", format.formatName(), "BlobSample");
    ensure_equals("Kind of format", format.frameKind(), BLOB);
	try
	{
		std::auto_ptr<Frame> blob = format.createFrame();

		ensure_equals("Kind of frame", blob->header().isBlobFrame(), true);
	    ensure_equals("Count of item", blob->itemCount(), 1u);
	    ensure_equals("Size of item", blob->itemSize_B(), 56u);

	    // Set value of field 'field3'
	    blob->item().field(54, 2).setValue(-130);

		// Encode blob
	    string path(TEMP_DIR "blob_frame.bin");
	    ofstream ofs(path.c_str(), ios::out | ios::binary);
	    blob->write(ofs);
	    ofs.close();

	    // Check size of binary frame
	    ifstream ifs(path.c_str(), ios::binary);
	    size_t begin = ifs.tellg();
	    ifs.seekg (0, ios::end);
	    size_t end = ifs.tellg();
	    size_t size = end - begin;
	    ensure_equals("Size of blob sample frame", size, 64u);
	}
	catch (mfm::Exception & e)
	{
		fail("Failed to create a blob frame");
	}
}

/**
 * Checks de-serialization of blob frame.
 */
template<> template<>
void testobject::test<2>()
{
    set_test_name("Checks de-serialization of blob frame");

	try
	{
		// Decode blob
	    string path(TEMP_DIR "blob_frame.bin");
	    ifstream ifs(path.c_str(), ios::binary);
	    std::auto_ptr<Frame> blob = Frame::read(ifs);
	    ifs.close();

	    // Check contents
		ensure_equals("Kind of frame", blob->header().isBlobFrame(), true);
	    ensure_equals("Count of item", blob->itemCount(), 1u);
	    ensure_equals("Size of item", blob->itemSize_B(), 56u);

	    FrameDictionary::instance().addFormats("mfm/sample_formats.xml");
	    ensure_equals("Value of field 'field3'", blob->item().field("field3").value<int16_t>(), -130);
	}
	catch (mfm::Exception & e)
	{
		fail(std::string("Failed to de-serialize blob frame: ").append(e.what()).c_str());
	}
}

} // namespace tut


