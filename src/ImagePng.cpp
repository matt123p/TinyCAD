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

#include "stdafx.h"
#include "ImagePNG.h"

int CImagePNG::SortRGBAxis;

#define RedV(vv) static_cast<int>((vv>>16) & 0xff)
#define GreenV(vv) static_cast<int>((vv>>8) & 0xff)
#define BlueV(vv) static_cast<int>((vv) & 0xff)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImagePNG::CImagePNG()
{
	m_ColorMapSize = 0;
	m_output_bits = NULL;
}

CImagePNG::~CImagePNG()
{
	delete m_output_bits;
}

// Get the type of this object
CString CImagePNG::GetType()
{
	return "PNG";
}

//////////////////////////////////////////////////////////////////////
//
// Convert a BMP file into a PNG file in memory...
//
//////////////////////////////////////////////////////////////////////


/******************************************************************************
 * Save the PNG resulting memory functions.						      *
 ******************************************************************************/

struct memory_block
{
	unsigned char* m_pData;
	unsigned int m_total_size;
	unsigned int m_size;

	memory_block()
	{
		m_pData = NULL;
		m_total_size = 0;
		m_size = 0;
	}
};

static void __cdecl user_write_mem(struct png_struct_def *p, unsigned char *c, unsigned int size)
{
	// Do we need more memory?
	memory_block *m = (memory_block *) p->io_ptr;

	if (m->m_pData == NULL || m->m_size + size >= m->m_total_size)
	{
		// Yep, so re-alloc
		int new_total_size = m->m_total_size + size * 4;
		unsigned char *n = new unsigned char[new_total_size];
		if (m->m_pData)
		{
			memcpy(n, m->m_pData, m->m_size + 1);
		}

		m->m_pData = n;
		m->m_total_size = new_total_size;
	}

	memcpy(m->m_pData + m->m_size, c, size);
	m->m_size += size;
}

static void __cdecl user_flush_mem(struct png_struct_def *p)
{
	// Do nothing...
}

bool CImagePNG::ImportBMP(const TCHAR *filename)
{
	CFile file;

	LPBITMAPINFOHEADER lpBMIH = NULL;
	LPVOID lpvColorTable = NULL;
	DWORD dwSizeImage = 0;
	int linewidth = 0;
	int nColorTableEntries = 0;
	HPALETTE hPalette = NULL;
	LPBYTE lpImage = NULL;

	// 1. read file header to get size of info hdr + color table
	// 2. read info hdr (to get image size) and color table
	// 3. read image
	// can't use bfSize in file header
	int nCount, nSize;
	BITMAPFILEHEADER bmfh;
	try
	{
		CFileException fe;
		if (!file.Open(filename, CFile::modeRead, &fe))
		{
			AfxMessageBox(_T("Unable to open image file"));
			return false;
		}

		nCount = file.Read((LPVOID) &bmfh, sizeof(BITMAPFILEHEADER));
		if (nCount != sizeof(BITMAPFILEHEADER))
		{
			throw new CUserException;
		}
		if (bmfh.bfType != 0x4d42)
		{
			throw new CUserException;
		}
		nSize = bmfh.bfOffBits - sizeof(BITMAPFILEHEADER);
		lpBMIH = (LPBITMAPINFOHEADER) new char[nSize];
		nCount = file.Read(lpBMIH, nSize); // info hdr & color table

		if (lpBMIH->biSize != sizeof(BITMAPINFOHEADER))
		{
			TRACE("Not a valid Windows bitmap -- probably an OS/2 bitmap\n");
			throw new CUserException;
		}
		dwSizeImage = lpBMIH->biSizeImage;
		if (dwSizeImage == 0)
		{
			DWORD dwBytes = ((DWORD) lpBMIH->biWidth * lpBMIH->biBitCount) / 32;
			if ( ((DWORD) lpBMIH->biWidth * lpBMIH->biBitCount) % 32)
			{
				dwBytes++;
			}
			dwBytes *= 4;
			linewidth = dwBytes;
			dwSizeImage = dwBytes * lpBMIH->biHeight; // no compression
		}
		lpvColorTable = (LPBYTE) lpBMIH + sizeof(BITMAPINFOHEADER);

		if ( (lpBMIH == NULL) || (lpBMIH->biClrUsed == 0))
		{
			switch (lpBMIH->biBitCount)
			{
				case 1:
					nColorTableEntries = 2;
					break;
				case 4:
					nColorTableEntries = 16;
					break;
				case 8:
					nColorTableEntries = 256;
					break;
				case 16:
				case 24:
				case 32:
					nColorTableEntries = 0;
					break;
				default:
					ASSERT(FALSE);
			}
		}
		else
		{
			nColorTableEntries = lpBMIH->biClrUsed;
		}

		// makes a logical palette (hPalette) from the DIB's color table
		// this palette will be selected and realized prior to drawing the DIB
		if (nColorTableEntries != 0)
		{

			if (hPalette != NULL)
			{
				::DeleteObject(hPalette);
			}

			TRACE("CDib::MakePalette -- nColorTableEntries = %d\n", nColorTableEntries);
			LPLOGPALETTE pLogPal = (LPLOGPALETTE) new char[2 * sizeof(WORD) + nColorTableEntries * sizeof(PALETTEENTRY)];
			pLogPal->palVersion = 0x300;
			pLogPal->palNumEntries = (WORD) nColorTableEntries;
			LPRGBQUAD pDibQuad = (LPRGBQUAD) lpvColorTable;
			for (int i = 0; i < nColorTableEntries; i++)
			{
				pLogPal->palPalEntry[i].peRed = pDibQuad->rgbRed;
				pLogPal->palPalEntry[i].peGreen = pDibQuad->rgbGreen;
				pLogPal->palPalEntry[i].peBlue = pDibQuad->rgbBlue;
				pLogPal->palPalEntry[i].peFlags = 0;
				pDibQuad++;
			}
			hPalette = ::CreatePalette(pLogPal);
			delete pLogPal;
		}

		lpImage = (LPBYTE) new char[dwSizeImage];
		nCount = file.Read(lpImage, dwSizeImage); // image only
	} catch (CException* pe)
	{
		AfxMessageBox(_T("Read error"));
		pe->Delete();
		return FALSE;
	}

	// Now we have the bitmap in memory convert into an acceptable format
	CClientDC ref_dc(AfxGetMainWnd());
	CBitmap *bitmap = CreateImageBitmap(lpBMIH->biWidth, lpBMIH->biHeight, ref_dc, 32);
	CDC bitmap_dc;
	bitmap_dc.CreateCompatibleDC(&ref_dc);

	CBitmap *old_bitmap = bitmap_dc.SelectObject(bitmap);
	bitmap_dc.SetStretchBltMode(COLORONCOLOR);
	::StretchDIBits(bitmap_dc.GetSafeHdc(), 0, 0, lpBMIH->biWidth, lpBMIH->biHeight, 0, 0, lpBMIH->biWidth, lpBMIH->biHeight, lpImage, (LPBITMAPINFO) lpBMIH, DIB_RGB_COLORS, SRCCOPY);
	bitmap_dc.SelectObject(old_bitmap);

	// Now destroy anything we created, except the new bitmap
	delete[] lpBMIH;
	if (hPalette != NULL)
	{
		::DeleteObject(hPalette);
	}

	// .. and create the png
	memory_block b;
	Save(ref_dc, &b, user_write_mem, user_flush_mem);

	SetCompressedData(b.m_pData, b.m_size);

	return TRUE;
}

bool CImagePNG::ImportBitmap(CBitmap &bitmap_in)
{
	BITMAP info;
	bitmap_in.GetBitmap(&info);
	CSize sz(info.bmWidth, info.bmHeight);

	// Now we have the bitmap in memory convert into an acceptable format
	CClientDC ref_dc(AfxGetMainWnd());
	CBitmap *bitmap = CreateImageBitmap(sz.cx, sz.cy, ref_dc, 32);

	CDC bitmap_dc_out, bitmap_dc_in;
	bitmap_dc_out.CreateCompatibleDC(&ref_dc);
	bitmap_dc_in.CreateCompatibleDC(&ref_dc);

	CBitmap *old_bitmap_out = bitmap_dc_out.SelectObject(bitmap);
	CBitmap *old_bitmap_in = bitmap_dc_in.SelectObject(&bitmap_in);

	bitmap_dc_out.BitBlt(0, 0, sz.cx, sz.cy, &bitmap_dc_in, 0, 0, SRCCOPY);

	bitmap_dc_out.SelectObject(old_bitmap_out);
	bitmap_dc_in.SelectObject(old_bitmap_in);

	// .. and create the png
	memory_block b;
	Save(ref_dc, &b, user_write_mem, user_flush_mem);

	SetCompressedData(b.m_pData, b.m_size);

	return true;
}

//////////////////////////////////////////////////////////////////////
//
// The error functions
//
//////////////////////////////////////////////////////////////////////
static void __cdecl user_error_fn(struct png_struct_def *p, const char *c)
{

}

static void __cdecl user_warning_fn(struct png_struct_def *p, const char *c)
{
}

//////////////////////////////////////////////////////////////////////
//
// The read functions
//
//////////////////////////////////////////////////////////////////////

void __cdecl CImagePNG::user_read_function(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_buffer *reader = (png_buffer*) (png_ptr->io_ptr);

	int bytes = length;
	if (bytes > reader->m_size_left)
	{
		bytes = reader->m_size_left;
	}

	memcpy(data, reader->m_pData, bytes);
	reader->m_pData += bytes;
	reader->m_size_left -= bytes;
}

// Read from a memory block
bool CImagePNG::Read(CDC &ref_dc, const unsigned char *buf, int size)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also supply the
	 * the compiler header file version, so that we know if the application
	 * was compiled with a compatible version of the library.  REQUIRED
	 */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, user_error_fn, user_warning_fn);
	if (png_ptr == NULL)
	{
		return FALSE;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem reading the file */
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return FALSE;
	}

	/* Set up the input control if you are using standard C streams */
	png_buffer reader;
	reader.m_pData = buf;
	reader.m_size_left = size;
	png_set_read_fn(png_ptr, &reader, user_read_function);

	/* If we have already read some of the signature */
	png_set_sig_bytes(png_ptr, sig_read);

	/* The call to png_read_info() gives us all of the information from the
	 * PNG file before the first IDAT (image data chunk).  REQUIRED
	 */
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);

	/* Set up the data transformations you want.  Note that these are all
	 * optional.  Only call them if you want/need them.  Many of the
	 * transformations only work on specific types of images, and many
	 * are mutually exclusive.
	 */

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	png_set_strip_16(png_ptr);

	/* Strip alpha bytes from the input data without combining with the
	 * background (not recommended).
	 */
	png_set_strip_alpha(png_ptr);

	/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	 * byte into separate bytes (useful for paletted and grayscale images).
	 */
	png_set_packing(png_ptr);

	/* Change the order of packed pixels to least significant bit first
	 * (not useful if you are using png_set_packing). */
	// png_set_packswap(png_ptr);

	/* Expand paletted colors into true RGB triplets */
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);

	/* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);

	/* Expand paletted or RGB images with transparency to full alpha channels
	 * so the data will be available as RGBA quartets.
	 */
	//if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	//   png_set_tRNS_to_alpha(png_ptr);

	/* Set the background color to draw transparent and alpha images over.
	 * It is possible to set the red, green, and blue components directly
	 * for paletted images instead of supplying a palette index.  Note that
	 * even if the PNG file supplies a background, you are not required to
	 * use it - you should use the (solid) application background if it has one.
	 */
	/*
	 png_color_16 my_background, *image_background;

	 if (png_get_bKGD(png_ptr, info_ptr, &image_background))
	 png_set_background(png_ptr, image_background,
	 PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	 else
	 png_set_background(png_ptr, &my_background,
	 PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
	 */

	/* Some suggestions as to how to get a screen gamma value */

	double screen_gamma = 2.2; /* A good guess for a PC monitors in a dimly
	 lit room */

	/* Tell libpng to handle the gamma conversion for you.  The final call
	 * is a good guess for PC generated images, but it should be configurable
	 * by the user at run time by the user.  It is strongly suggested that
	 * your application support gamma correction.
	 */

	int intent;

	if (png_get_sRGB(png_ptr, info_ptr, &intent)) png_set_gamma(png_ptr, screen_gamma, 0.45455);
	else
	{
		double image_gamma;
		if (png_get_gAMA(png_ptr, info_ptr, &image_gamma)) png_set_gamma(png_ptr, screen_gamma, image_gamma);
		else png_set_gamma(png_ptr, screen_gamma, 0.45455);
	}

	/* flip the RGB pixels to BGR (or RGBA to BGRA) */
	if (color_type & PNG_COLOR_MASK_COLOR) png_set_bgr(png_ptr);

	/* swap the RGBA or GA data to ARGB or AG (or BGRA to ABGR) */
	//   png_set_swap_alpha(png_ptr);

	/* swap bytes of 16 bit files to least significant byte first */
	//   png_set_swap(png_ptr);

	/* Add filler (or alpha) byte (before/after each RGB triplet) */
	//   png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	/* Turn on interlace handling.  REQUIRED if you are not using
	 * png_read_image().  To see how to handle interlacing passes,
	 * see the png_read_row() method below:
	 */
	int number_passes = png_set_interlace_handling(png_ptr);

	/* Optional call to gamma correct and add the background to the palette
	 * and update info structure.  REQUIRED if you are expecting libpng to
	 * update the palette for you (ie you selected such a transform above).
	 */
	//   png_read_update_info(png_ptr, info_ptr);

	/* Allocate the memory to hold the image using the fields of info_ptr. */

	/* The easiest way to read the image: */
	png_bytep* row_pointers = new png_bytep[height];
	CreateImageBitmap(width, height, ref_dc, 24);

	for (unsigned row = 0; row < height; row++)
	{
		row_pointers[height - row - 1] = m_input_bits + (m_RowStep * row);
	}

	/* Now it's time to read the image.  One of these methods is REQUIRED */
	png_read_image(png_ptr, row_pointers);

	/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* At this point you have read the entire image */
	delete[] row_pointers;

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

	/* that's it */
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
//
// The save functions
//
//////////////////////////////////////////////////////////////////////


#define ABS(x)	((x) > 0 ? (x) : (-(x)))

/* The colors are stripped to 5 bits per primary color */
#define COLOR_ARRAY_SIZE 32768
#define BITS_PER_PRIM_COLOR 5
#define MAX_PRIM_COLOR      0x1f

/******************************************************************************
 * Save the PNG resulting file functions.						      *
 ******************************************************************************/

static void __cdecl user_write_function(struct png_struct_def *p, unsigned char *c, unsigned int size)
{
	fwrite(c, size, 1, (FILE*) (p->io_ptr));
}

static void __cdecl user_flush_function(struct png_struct_def *p)
{
	fflush((FILE*) (p->io_ptr));
}

/******************************************************************************
 * Save the PNG resulting image.						      *
 ******************************************************************************/
void CImagePNG::Save(CDC &ref_dc, const TCHAR *file_name)
{
	FILE *fp;

	/* open the file */
	errno_t err;
	err = _tfopen_s(&fp, file_name, _T("wb"));
	if ( (fp == NULL) || (err != 0))
	{ //unable to open the file
		return;
	}

	Save(ref_dc, fp, user_write_function, user_flush_function);

	fclose(fp);
}

void CImagePNG::Save(CDC &ref_dc, void *io_ptr, void *write_data_fn, void *output_flush_fn)
{
	png_structp png_ptr;
	png_infop info_ptr;

	/*
	 Convert the 8bpp
	 */
	CBitmap output;
	QuantizeBuffer(output, ref_dc);

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also check that
	 * the library version is compatible with the one used at compile time,
	 * in case we are using dynamically linked libraries.  REQUIRED.
	 */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, user_error_fn, user_warning_fn);

	if (png_ptr == NULL)
	{
		return;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_write_struct(&png_ptr, png_infopp_NULL);
		return;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem reading the file */
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return;
	}

	/* set up the output control if you are using standard C streams */
	// png_init_io(png_ptr, fp);

	/* If you are using replacement read functions, instead of calling
	 * png_init_io() here you would call */
	png_set_write_fn(png_ptr, io_ptr, (png_rw_ptr) write_data_fn, (png_flush_ptr) output_flush_fn);

	/* This is the hard way */

	/* Set the image information here.  Width and height are up to 2^31,
	 * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	 * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	 * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	 * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	 * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	 * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
	 */
	png_set_IHDR(png_ptr, info_ptr, m_Width, m_Height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* ... set palette colors ... */
	png_set_PLTE(png_ptr, info_ptr, m_ColorMap, PNG_MAX_PALETTE_LENGTH);

	/* optional significant bit chunk */
#if 0
	png_color_8p sig_bit;
	/* if we are dealing with a grayscale image then */
	sig_bit.gray = 8;
	/* otherwise, if we are dealing with a color image then */
	sig_bit.red = 8;
	sig_bit.green = 8;
	sig_bit.blue = 8;
	/* if the image has an alpha channel then */
	sig_bit.alpha = 8;
	png_set_sBIT(png_ptr, info_ptr, sig_bit);
#endif

	/* Optional gamma chunk is strongly suggested if you have any guess
	 * as to the correct gamma of the image.
	 */

	/* Optionally write comments into the image */
	png_text text_ptr[3];
	text_ptr[0].key = "Title";
	text_ptr[0].text = "Mona Lisa";
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[1].key = "Author";
	text_ptr[1].text = "Leonardo DaVinci";
	text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[2].key = "Description";
	text_ptr[2].text = "<long text>";
	text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
#ifdef PNG_iTXt_SUPPORTED
	text_ptr[0].lang = NULL;
	text_ptr[1].lang = NULL;
	text_ptr[2].lang = NULL;
#endif
	png_set_text(png_ptr, info_ptr, text_ptr, 3);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	/* The easiest way to write the image (you may have a different memory
	 * layout, however, so choose what fits your needs best).  You need to
	 * use the first method if you aren't handling interlacing yourself.
	 */
	int k;
	png_bytep *row_pointers = new png_bytep[m_Height];
	for (k = 0; k < m_Height; k++)
		row_pointers[m_Height - 1 - k] = m_output_bits + k * m_Width;

	/* One of the following output methods is REQUIRED */
	png_write_image(png_ptr, row_pointers);

	delete[] row_pointers;

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	/* Similarly, if you png_malloced any data that you passed in with
	 png_set_something(), such as a hist or trans array, free it here,
	 when you can be sure that libpng is through with it. */
	// png_free(png_ptr, trans);
	// trans=NULL;

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

// Create an output bitmap
void CImagePNG::CreateOutputBitmap(CBitmap &output, CDC &dc)
{
	// Create a bitmap to draw to...
	struct
	{
		BITMAPINFO bi;
		RGBQUAD bipal[256];
	} q;

	// Word Align
	int rWidth = m_Width;
	while ( (rWidth & 0x3) != 0)
	{
		rWidth++;
	}

	// Create the bitmap
	q.bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	q.bi.bmiHeader.biWidth = rWidth;
	q.bi.bmiHeader.biHeight = -m_Height;
	q.bi.bmiHeader.biPlanes = 1;
	q.bi.bmiHeader.biBitCount = 8;
	q.bi.bmiHeader.biCompression = BI_RGB;
	q.bi.bmiHeader.biSizeImage = 0;
	q.bi.bmiHeader.biXPelsPerMeter = 0;
	q.bi.bmiHeader.biYPelsPerMeter = 0;
	q.bi.bmiHeader.biClrUsed = m_ColorMapSize;
	q.bi.bmiHeader.biClrImportant = 0;

	int i;
	for (i = 0; i < m_ColorMapSize; i++)
	{
		q.bi.bmiColors[i].rgbRed = m_ColorMap[i].red;
		q.bi.bmiColors[i].rgbGreen = m_ColorMap[i].green;
		q.bi.bmiColors[i].rgbBlue = m_ColorMap[i].blue;
		q.bi.bmiColors[i].rgbReserved = 0;
	}

	unsigned char *ScreenBuffer = NULL;
	HBITMAP hb = CreateDIBSection(dc.m_hDC, &q.bi, DIB_RGB_COLORS, (void**) &ScreenBuffer, NULL, 0);
	int e = GetLastError();
	output.Attach(hb);

	unsigned char *Ptr = m_output_bits;
	for (i = 0; i < m_Height; i++)
	{
		memcpy(ScreenBuffer, Ptr, m_Width);
		Ptr += m_Width;
		ScreenBuffer += rWidth;
	}
}

/*******************************************************************************
 * Module to quatize high resolution image into lower one. You may want to    *
 * peek into the following article this code is based on:		     *
 * "Color Image Quantization for frame buffer Display", by Paul Heckbert      *
 * SIGGRAPH 1982 page 297-307.						     *
 * Written by:  Gershon Elber			IBM PC Ver 0.1,	Jun. 1989    *
 ******************************************************************************
 * History:								     *
 * 5 Jan 90 - Version 1.0 by Gershon Elber.				     *
 *****************************************************************************/

/******************************************************************************
 * Quantize high resolution image into lower one. Input image consists of a    *
 * 2D array for each of the RGB colors with size m_Width by m_Height. There is no  *
 * Color map for the input. Output is a quantized image with 2D array of       *
 * indexes into the output color map.					      *
 *   Note input image can be 24 bits at the most (8 for red/green/blue) and    *
 * the output has 256 colors at the most (256 entries in the color map.).      *
 * m_ColorMapSize specifies size of color map up to 256 and will be updated to   *
 * real size before returning.						      *
 *   Also non of the parameter are allocated by this routine.		      *
 *   This function returns GIF_OK if succesfull, GIF_ERROR otherwise.	      *
 ******************************************************************************/
void CImagePNG::QuantizeBuffer(CBitmap &output, CDC &dc)
{
	unsigned int Index, NumOfEntries;
	int i, j, MaxRGBError[3];
	int NewColorMapSize;
	long Red, Green, Blue;
	unsigned int *input_bits = (unsigned int*) m_input_bits;

	m_ColorMapSize = 256;
	NewColorMapType NewColorSubdiv[256];
	QuantizedColorType ColorArrayEntries[COLOR_ARRAY_SIZE];
	QuantizedColorType *QuantizedColor;

	m_output_bits = new unsigned char[m_Width * m_Height];

	for (i = 0; i < COLOR_ARRAY_SIZE; i++)
	{
		ColorArrayEntries[i].RGB[0] = i >> (2 * BITS_PER_PRIM_COLOR);
		ColorArrayEntries[i].RGB[1] = (i >> BITS_PER_PRIM_COLOR) & MAX_PRIM_COLOR;
		ColorArrayEntries[i].RGB[2] = i & MAX_PRIM_COLOR;
		ColorArrayEntries[i].Count = 0;
	}

	/* Sample the colors and their distribution: */
	for (i = 0; i < m_Width * m_Height; i++)
	{
		COLORREF colour = input_bits[i];
		Index = ( (RedV(colour) >> (8 - BITS_PER_PRIM_COLOR)) << (2 * BITS_PER_PRIM_COLOR)) + ( (GreenV(colour) >> (8 - BITS_PER_PRIM_COLOR)) << BITS_PER_PRIM_COLOR) + (BlueV(colour) >> (8 - BITS_PER_PRIM_COLOR));
		ColorArrayEntries[Index].Count++;
	}

	/* Put all the colors in the first entry of the color map, and call the  */
	/* recursive subdivision process.					     */
	for (i = 0; i < 256; i++)
	{
		NewColorSubdiv[i].QuantizedColors = NULL;
		NewColorSubdiv[i].Count = NewColorSubdiv[i].NumEntries = 0;
		for (j = 0; j < 3; j++)
		{
			NewColorSubdiv[i].RGBMin[j] = 0;
			NewColorSubdiv[i].RGBWidth[j] = 255;
		}
	}

	/* Find the non empty entries in the color table and chain them: */
	for (i = 0; i < COLOR_ARRAY_SIZE; i++)
	{
		if (ColorArrayEntries[i].Count > 0)
		{
			break;
		}
	}

	QuantizedColor = NewColorSubdiv[0].QuantizedColors = &ColorArrayEntries[i];
	NumOfEntries = 1;
	while (++i < COLOR_ARRAY_SIZE)
		if (ColorArrayEntries[i].Count > 0)
		{
			QuantizedColor -> Pnext = &ColorArrayEntries[i];
			QuantizedColor = &ColorArrayEntries[i];
			NumOfEntries++;
		}
	QuantizedColor -> Pnext = NULL;

	NewColorSubdiv[0].NumEntries = NumOfEntries;/* Different sampled colors. */
	NewColorSubdiv[0].Count = ((long) m_Width) * m_Height; /* Pixels. */
	NewColorMapSize = 1;
	if (!SubdivColorMap(NewColorSubdiv, m_ColorMapSize, &NewColorMapSize))
	{
		return;
	}

	if (NewColorMapSize < m_ColorMapSize)
	{
		/* And clear rest of color map: */
		for (i = NewColorMapSize; i < m_ColorMapSize; i++)
		{
			m_ColorMap[i].red = m_ColorMap[i].green = m_ColorMap[i].blue = 0;
		}
	}

	/* Average the colors in each entry to be the color to be used in the    */
	/* output color map, and plug it into the output color map itself.       */
	for (i = 0; i < NewColorMapSize; i++)
	{
		if ( (j = NewColorSubdiv[i].NumEntries) > 0)
		{
			QuantizedColor = NewColorSubdiv[i].QuantizedColors;
			Red = Green = Blue = 0;

			while (QuantizedColor)
			{
				QuantizedColor ->NewColorIndex = (unsigned char) i;
				Red += QuantizedColor -> RGB[0];
				Green += QuantizedColor -> RGB[1];
				Blue += QuantizedColor -> RGB[2];
				QuantizedColor = QuantizedColor -> Pnext;
			}

			m_ColorMap[i].red = static_cast<png_byte> ( (Red << (8 - BITS_PER_PRIM_COLOR)) / j);
			m_ColorMap[i].green = static_cast<png_byte> ( (Green << (8 - BITS_PER_PRIM_COLOR)) / j);
			m_ColorMap[i].blue = static_cast<png_byte> ( (Blue << (8 - BITS_PER_PRIM_COLOR)) / j);
		}
		else
		{
			// GIF_MESSAGE("Null entry in quantized color map - thats weird.");
			AfxDebugBreak();
		}

		// Now extend the lower bit so it is 5bpp -> 8bpp
		if ( (m_ColorMap[i].red & 8) != 0)
		{
			m_ColorMap[i].red |= 7;
		}
		if ( (m_ColorMap[i].green & 8) != 0)
		{
			m_ColorMap[i].green |= 7;
		}
		if ( (m_ColorMap[i].blue & 8) != 0)
		{
			m_ColorMap[i].blue |= 7;
		}
	}

	/* Finally scan the input buffer again and put the mapped index in the   */
	/* output buffer.							     */
	MaxRGBError[0] = MaxRGBError[1] = MaxRGBError[2] = 0;

	for (i = 0; i < m_Width * m_Height; i++)
	{
		COLORREF colour = input_bits[i];
		Index = ( (RedV(colour) >> (8 - BITS_PER_PRIM_COLOR)) << (2 * BITS_PER_PRIM_COLOR)) + ( (GreenV(colour) >> (8 - BITS_PER_PRIM_COLOR)) << BITS_PER_PRIM_COLOR) + (BlueV(colour) >> (8 - BITS_PER_PRIM_COLOR));
		Index = ColorArrayEntries[Index].NewColorIndex;

		m_output_bits[i] = (unsigned char) Index;
		if (MaxRGBError[0] < ABS(m_ColorMap[Index].red - RedV(colour))) MaxRGBError[0] = ABS(m_ColorMap[Index].red - RedV(colour));
		if (MaxRGBError[1] < ABS(m_ColorMap[Index].green - GreenV(colour))) MaxRGBError[1] = ABS(m_ColorMap[Index].green - GreenV(colour));
		if (MaxRGBError[2] < ABS(m_ColorMap[Index].blue - BlueV(colour))) MaxRGBError[2] = ABS(m_ColorMap[Index].blue - BlueV(colour));
	}

	CreateOutputBitmap(output, dc);

	m_ColorMapSize = NewColorMapSize;
	return;
}

/******************************************************************************
 * Routine to subdivide the RGB space recursively using median cut in each     *
 * axes alternatingly until m_ColorMapSize different cubes exists.		      *
 * The biggest cube in one dimension is subdivide unless it has only one entry.*
 * Returns GIF_ERROR if failed, otherwise GIF_OK.			      *
 ******************************************************************************/
bool CImagePNG::SubdivColorMap(NewColorMapType *NewColorSubdiv, int m_ColorMapSize, int *NewColorMapSize)
{
	int MaxSize;
	int i, j, Index = 0, NumEntries, MinColor, MaxColor;
	long Sum, Count;
	QuantizedColorType *QuantizedColor, **SortArray;

	while (m_ColorMapSize > *NewColorMapSize)
	{
		/* Find candidate for subdivision: */
		MaxSize = -1;
		for (i = 0; i < *NewColorMapSize; i++)
		{
			for (j = 0; j < 3; j++)
			{
				if ( ((int) NewColorSubdiv[i].RGBWidth[j]) > MaxSize && NewColorSubdiv[i].NumEntries > 1)
				{
					MaxSize = NewColorSubdiv[i].RGBWidth[j];
					Index = i;
					SortRGBAxis = j;
				}
			}
		}

		if (MaxSize == -1) return true;

		/* Split the entry Index into two along the axis SortRGBAxis: */

		/* Sort all elements in that entry along the given axis and split at */
		/* the median.							     */
		if ( (SortArray = new QuantizedColorType*[NewColorSubdiv[Index].NumEntries]) == NULL) return ERROR;
		for (j = 0, QuantizedColor = NewColorSubdiv[Index].QuantizedColors; j < NewColorSubdiv[Index].NumEntries && QuantizedColor != NULL; j++, QuantizedColor = QuantizedColor -> Pnext)
			SortArray[j] = QuantizedColor;
		qsort(SortArray, NewColorSubdiv[Index].NumEntries, sizeof(QuantizedColorType *), SortCmpRtn);

		/* Relink the sorted list into one: */
		for (j = 0; j < NewColorSubdiv[Index].NumEntries - 1; j++)
			SortArray[j] -> Pnext = SortArray[j + 1];
		SortArray[NewColorSubdiv[Index].NumEntries - 1] -> Pnext = NULL;
		NewColorSubdiv[Index].QuantizedColors = QuantizedColor = SortArray[0];
		delete[] SortArray;

		/* Now simply add the Counts until we have half of the Count: */
		Sum = NewColorSubdiv[Index].Count / 2 - QuantizedColor -> Count;
		NumEntries = 1;
		Count = QuantizedColor -> Count;
		while ( (Sum -= QuantizedColor -> Pnext -> Count) >= 0 && QuantizedColor -> Pnext != NULL && QuantizedColor -> Pnext -> Pnext != NULL)
		{
			QuantizedColor = QuantizedColor -> Pnext;
			NumEntries++;
			Count += QuantizedColor -> Count;
		}
		/* Save the values of the last color of the first half, and first    */
		/* of the second half so we can update the Bounding Boxes later.     */
		/* Also as the colors are quantized and the BBoxes are full 0..255,  */
		/* they need to be rescaled.					     */
		MaxColor = QuantizedColor -> RGB[SortRGBAxis];/* Max. of first half. */
		MinColor = QuantizedColor -> Pnext -> RGB[SortRGBAxis];/* of second. */
		MaxColor <<= (8 - BITS_PER_PRIM_COLOR);
		MinColor <<= (8 - BITS_PER_PRIM_COLOR);

		/* Partition right here: */
		NewColorSubdiv[*NewColorMapSize].QuantizedColors = QuantizedColor -> Pnext;
		QuantizedColor -> Pnext = NULL;
		NewColorSubdiv[*NewColorMapSize].Count = Count;
		NewColorSubdiv[Index].Count -= Count;
		NewColorSubdiv[*NewColorMapSize].NumEntries = NewColorSubdiv[Index].NumEntries - NumEntries;
		NewColorSubdiv[Index].NumEntries = NumEntries;
		for (j = 0; j < 3; j++)
		{
			NewColorSubdiv[*NewColorMapSize].RGBMin[j] = NewColorSubdiv[Index].RGBMin[j];
			NewColorSubdiv[*NewColorMapSize].RGBWidth[j] = NewColorSubdiv[Index].RGBWidth[j];
		}
		NewColorSubdiv[*NewColorMapSize].RGBWidth[SortRGBAxis] = NewColorSubdiv[*NewColorMapSize].RGBMin[SortRGBAxis] + NewColorSubdiv[*NewColorMapSize].RGBWidth[SortRGBAxis] - (unsigned char) MinColor;
		NewColorSubdiv[*NewColorMapSize].RGBMin[SortRGBAxis] = (unsigned char) MinColor;

		NewColorSubdiv[Index].RGBWidth[SortRGBAxis] = (unsigned char) MaxColor - NewColorSubdiv[Index].RGBMin[SortRGBAxis];

		(*NewColorMapSize)++;
	}

	return true;
}

/******************************************************************************
 * Routine called by qsort to compare to entries.			      *
 ******************************************************************************/
int __cdecl CImagePNG::SortCmpRtn(const void * Entry1, const void * Entry2)
{
	return (* ((QuantizedColorType **) Entry1)) -> RGB[SortRGBAxis] - (* ((QuantizedColorType **) Entry2)) -> RGB[SortRGBAxis];
}

