/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002-2005 Matt Pyne.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// ImageJpeg.cpp: implementation of the CImageJpeg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageJpeg.h"
#include <setjmp.h>

// JPEG includes
#undef FAR
#define boolean booleanX
#define INT32 jpegINT32
extern "C" 
{
	#include "jpeg/jinclude.h"
	#include "jpeg/jerror.h"
	#include "jpeg/jpeglib.h"
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImageJpeg::CImageJpeg()
{

}

CImageJpeg::~CImageJpeg()
{

}




METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  static JOCTET dummy[] = { (JOCTET) 0xFF, (JOCTET) JPEG_EOI };

  // WARNMS(cinfo, JWRN_JPEG_EOF);

  /* Insert a fake EOI marker */

  cinfo->src->next_input_byte = dummy;
  cinfo->src->bytes_in_buffer = 2;

  return TRUE;
}

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes > 0) {
    while (num_bytes > (long) cinfo->src->bytes_in_buffer) {
      num_bytes -= (long) cinfo->src->bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
    }
    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
  }
}


METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}



void Read_JPEG_Mem( struct jpeg_source_mgr &jsrc, const unsigned char * buffer, unsigned long length )
{
  // specify the data source
  jsrc.init_source = init_source;
  jsrc.fill_input_buffer = fill_input_buffer;
  jsrc.skip_input_data = skip_input_data;
  jsrc.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  jsrc.term_source = term_source;
  jsrc.bytes_in_buffer = length;
  jsrc.next_input_byte = (JOCTET *) buffer;
}



///////////////////////////////////////////////////////////////////////////
//
//
// Read in a JPEG file


struct my_error_mgr 
{
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  // (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}



bool CImageJpeg::Read( CDC &ref_dc, const unsigned char *buf, int size )
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    return false;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  struct jpeg_source_mgr jsrc;
  cinfo.src = &jsrc;
  Read_JPEG_Mem( jsrc, buf, size );

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */

  CreateImageBitmap( cinfo.output_width, cinfo.output_height, ref_dc, 24 );


  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */

	while (cinfo.output_scanline < cinfo.output_height) 
	{
		int y = cinfo.output_height - cinfo.output_scanline -1;
		unsigned char *buffer = m_input_bits + (m_RowStep * y);
		jpeg_read_scanlines(&cinfo, &buffer, 1);

		/* We must now re-order the colours as they come out in the wrong order
		for windows ... */
		for (int x = 0; x < cinfo.output_width; x++)
		{
		   unsigned char *b = buffer;
		   unsigned char *r = buffer+2;
		   unsigned char temp = *b; 
		   *b = *r; 
		   *r = temp;
		   buffer += cinfo.output_components;
		}
	}


  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);


  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return 1;
}


// Get the type of this object
CString CImageJpeg::GetType()
{
	return "JPEG";
}

