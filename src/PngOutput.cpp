

#include "stdafx.h"
#include "PngOutput.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

int CPngOutput::SortRGBAxis;

#define RedV(vv) static_cast<int>((vv>>16) & 0xff)
#define GreenV(vv) static_cast<int>((vv>>8) & 0xff)
#define BlueV(vv) static_cast<int>((vv) & 0xff)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPngOutput::CPngOutput()
{
	m_Width = 0;
	m_Height = 0;
	m_ColorMapSize = 0;
	m_output_bits = NULL;
	m_input_bits = NULL;
}

CPngOutput::~CPngOutput()
{
	delete m_output_bits;
}






#define ABS(x)	((x) > 0 ? (x) : (-(x)))

/* The colors are stripped to 5 bits per primary color */
#define COLOR_ARRAY_SIZE 32768
#define BITS_PER_PRIM_COLOR 5
#define MAX_PRIM_COLOR      0x1f

CBitmap *CPngOutput::CreateInputBitmap(int Width, int Height, CDC &dc)
{
	m_Width = Width;
	m_Height = Height;

	// Create the bitmap
	CBitmap bitmap;
	struct
	{
		BITMAPINFO bi;
		RGBQUAD bipal[3];
	} q;

	q.bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
	q.bi.bmiHeader.biWidth = Width;
	q.bi.bmiHeader.biHeight = Height; 
	q.bi.bmiHeader.biPlanes = 1; 
	q.bi.bmiHeader.biBitCount = 32;
	q.bi.bmiHeader.biCompression = BI_RGB;
	q.bi.bmiHeader.biSizeImage = 0; 
	q.bi.bmiHeader.biXPelsPerMeter = 0; 
	q.bi.bmiHeader.biYPelsPerMeter = 0; 
	q.bi.bmiHeader.biClrUsed = 3; 
	q.bi.bmiHeader.biClrImportant = 0; 
	q.bi.bmiColors[0].rgbRed = 0;
	q.bi.bmiColors[0].rgbGreen = 0;
	q.bi.bmiColors[0].rgbBlue = 0;
	q.bi.bmiColors[0].rgbReserved = 0;

	m_input_bits = NULL;
	HBITMAP hb = CreateDIBSection(dc.m_hDC, &q.bi, DIB_RGB_COLORS, (void**)&m_input_bits, NULL, 0 ); 
	m_input.Attach( hb );

	return &m_input;
}

static void __cdecl user_error_fn( struct png_struct_def *p, const char *c)
{
	AfxMessageBox( "PNG Output error" );
}

static void __cdecl user_warning_fn(struct png_struct_def *p, const char *c)
{
}

static void __cdecl user_write_function( struct png_struct_def *p, unsigned char *c, unsigned int size)
{
	fwrite( c, size,1, (FILE*)(p->io_ptr) );
}

static void __cdecl user_flush_function( struct png_struct_def *p)
{
	fflush( (FILE*)(p->io_ptr) );
}


/******************************************************************************
* Save the PNG resulting image.						      *
******************************************************************************/
void CPngOutput::Save( const char *file_name )
{
   FILE *fp;
   png_structp png_ptr;
   png_infop info_ptr;

   /* open the file */
   fp = fopen(file_name, "wb");
   if (fp == NULL)
   {
      return;
   }

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      NULL, user_error_fn, user_warning_fn);

   if (png_ptr == NULL)
   {
      fclose(fp);
      return;
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
      return;
   }

   /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* If we get here, we had a problem reading the file */
      fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return;
   }

   /* set up the output control if you are using standard C streams */
   // png_init_io(png_ptr, fp);

   /* If you are using replacement read functions, instead of calling
    * png_init_io() here you would call */
   png_set_write_fn(png_ptr, (void *)fp, user_write_function,
      user_flush_function);


   /* This is the hard way */

   /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */
   png_set_IHDR(png_ptr, info_ptr, m_Width, m_Height, 8, PNG_COLOR_TYPE_PALETTE,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

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
     row_pointers[m_Height -1 - k] = m_output_bits + k*m_Width;

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

   /* close the file */
   fclose(fp);
}


// Create an output bitmap
void CPngOutput::CreateOutputBitmap(CBitmap &output, CDC &dc)
{
	// Create a bitmap to draw to...
	struct
	{
		BITMAPINFO bi;
		RGBQUAD bipal[256];
	} q;

	// Word Align
	int rWidth = m_Width;
	while ((rWidth & 0x3) != 0)
	{
		rWidth ++;
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

	for (int i =0; i<m_ColorMapSize; i++)
	{
		q.bi.bmiColors[i].rgbRed = m_ColorMap[i].red;
		q.bi.bmiColors[i].rgbGreen = m_ColorMap[i].green;
		q.bi.bmiColors[i].rgbBlue = m_ColorMap[i].blue;
		q.bi.bmiColors[i].rgbReserved = 0;
	}


	unsigned char *ScreenBuffer = NULL;
	HBITMAP hb = CreateDIBSection(dc.m_hDC, &q.bi, DIB_RGB_COLORS, (void**)&ScreenBuffer, NULL, 0 ); 
	int e = GetLastError();
	output.Attach( hb );

    unsigned char *Ptr = m_output_bits;
    for (i = 0; i < m_Height; i++) 
	{
		memcpy(ScreenBuffer, Ptr, m_Width );	
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
void CPngOutput::QuantizeBuffer( CBitmap &output, CDC &dc )
{
    unsigned int Index, NumOfEntries;
    int i, j, MaxRGBError[3];
    int NewColorMapSize;
    long Red, Green, Blue;

	m_ColorMapSize = 256;
    NewColorMapType NewColorSubdiv[256];
    QuantizedColorType ColorArrayEntries[COLOR_ARRAY_SIZE];
	QuantizedColorType *QuantizedColor;

	m_output_bits = new unsigned char[ m_Width * m_Height ];


    for (i = 0; i < COLOR_ARRAY_SIZE; i++) 
	{
		ColorArrayEntries[i].RGB[0]= i >> (2 * BITS_PER_PRIM_COLOR);
		ColorArrayEntries[i].RGB[1] = (i >> BITS_PER_PRIM_COLOR) &
	    							MAX_PRIM_COLOR;
		ColorArrayEntries[i].RGB[2] = i & MAX_PRIM_COLOR;
		ColorArrayEntries[i].Count = 0;
    }

    /* Sample the colors and their distribution: */
	for (i=0; i < m_Width*m_Height; i++)
	{
		COLORREF colour = m_input_bits[i];
		Index = ((RedV(colour) >> (8 - BITS_PER_PRIM_COLOR))
			<< (2 * BITS_PER_PRIM_COLOR)) +
		((GreenV(colour) >> (8 - BITS_PER_PRIM_COLOR))
			<< BITS_PER_PRIM_COLOR) +
		(BlueV(colour) >> (8 - BITS_PER_PRIM_COLOR));
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
    NewColorSubdiv[0].Count = ((long) m_Width) * m_Height;            /* Pixels. */
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
			m_ColorMap[i].red =
			m_ColorMap[i].green =
			m_ColorMap[i].blue = 0;
		}
    }

    /* Average the colors in each entry to be the color to be used in the    */
    /* output color map, and plug it into the output color map itself.       */
    for (i = 0; i < NewColorMapSize; i++) 
	{
		if ((j = NewColorSubdiv[i].NumEntries) > 0) 
		{
			QuantizedColor = NewColorSubdiv[i].QuantizedColors;
			Red = Green = Blue = 0;
			
			while (QuantizedColor) 
			{
				QuantizedColor ->NewColorIndex = i;
				Red += QuantizedColor -> RGB[0];
				Green += QuantizedColor -> RGB[1];
				Blue += QuantizedColor -> RGB[2];
				QuantizedColor = QuantizedColor -> Pnext;
			}

			m_ColorMap[i].red = (Red << (8 - BITS_PER_PRIM_COLOR)) / j;
			m_ColorMap[i].green = (Green << (8 - BITS_PER_PRIM_COLOR)) / j;
			m_ColorMap[i].blue= (Blue << (8 - BITS_PER_PRIM_COLOR)) / j;
		}
		else
		{
			// GIF_MESSAGE("Null entry in quantized color map - thats weird.");
			AfxDebugBreak();
		}
	}

    /* Finally scan the input buffer again and put the mapped index in the   */
    /* output buffer.							     */
    MaxRGBError[0] = MaxRGBError[1] = MaxRGBError[2] = 0;

	for (i=0; i < m_Width*m_Height; i++)
	{
		COLORREF colour = m_input_bits[i];
		Index = ((RedV(colour) >> (8 - BITS_PER_PRIM_COLOR))
				<< (2 * BITS_PER_PRIM_COLOR)) +
			((GreenV(colour) >> (8 - BITS_PER_PRIM_COLOR))
				<< BITS_PER_PRIM_COLOR) +
			(BlueV(colour) >> (8 - BITS_PER_PRIM_COLOR));
		Index = ColorArrayEntries[Index].NewColorIndex;

		m_output_bits[i] = Index;
		if (MaxRGBError[0] < ABS(m_ColorMap[Index].red - RedV(colour)))
			MaxRGBError[0] = ABS(m_ColorMap[Index].red - RedV(colour));
		if (MaxRGBError[1] < ABS(m_ColorMap[Index].green - GreenV(colour)))
			MaxRGBError[1] = ABS(m_ColorMap[Index].green - GreenV(colour));
		if (MaxRGBError[2] < ABS(m_ColorMap[Index].blue - BlueV(colour)))
			MaxRGBError[2] = ABS(m_ColorMap[Index].blue - BlueV(colour));
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
bool CPngOutput::SubdivColorMap(NewColorMapType *NewColorSubdiv,
			  int m_ColorMapSize,
			  int *NewColorMapSize)
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
				if (((int) NewColorSubdiv[i].RGBWidth[j]) > MaxSize &&
					NewColorSubdiv[i].NumEntries > 1) 
				{
					MaxSize = NewColorSubdiv[i].RGBWidth[j];
					Index = i;
					SortRGBAxis = j;
				}
			}
		}

		if (MaxSize == -1)
			return true;

		/* Split the entry Index into two along the axis SortRGBAxis: */

		/* Sort all elements in that entry along the given axis and split at */
		/* the median.							     */
		if ((SortArray = 
			new QuantizedColorType*[ NewColorSubdiv[Index].NumEntries] ) == NULL)
    			return ERROR;
		for (j = 0, QuantizedColor = NewColorSubdiv[Index].QuantizedColors;
			 j < NewColorSubdiv[Index].NumEntries && QuantizedColor != NULL;
			 j++, QuantizedColor = QuantizedColor -> Pnext)
			SortArray[j] = QuantizedColor;
		qsort(SortArray, NewColorSubdiv[Index].NumEntries,
			  sizeof(QuantizedColorType *), SortCmpRtn);

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
		while ((Sum -= QuantizedColor -> Pnext -> Count) >= 0 &&
			   QuantizedColor -> Pnext != NULL &&
			   QuantizedColor -> Pnext -> Pnext != NULL) {
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
		NewColorSubdiv[*NewColorMapSize].QuantizedColors =
			QuantizedColor -> Pnext;
		QuantizedColor -> Pnext = NULL;
		NewColorSubdiv[*NewColorMapSize].Count = Count;
		NewColorSubdiv[Index].Count -= Count;
		NewColorSubdiv[*NewColorMapSize].NumEntries =
			NewColorSubdiv[Index].NumEntries - NumEntries;
		NewColorSubdiv[Index].NumEntries = NumEntries;
		for (j = 0; j < 3; j++) 
		{
			NewColorSubdiv[*NewColorMapSize].RGBMin[j] =
			NewColorSubdiv[Index].RGBMin[j];
			NewColorSubdiv[*NewColorMapSize].RGBWidth[j] =
			NewColorSubdiv[Index].RGBWidth[j];
		}
		NewColorSubdiv[*NewColorMapSize].RGBWidth[SortRGBAxis] =
			NewColorSubdiv[*NewColorMapSize].RGBMin[SortRGBAxis] +
			NewColorSubdiv[*NewColorMapSize].RGBWidth[SortRGBAxis] -
			MinColor;
		NewColorSubdiv[*NewColorMapSize].RGBMin[SortRGBAxis] = MinColor;

		NewColorSubdiv[Index].RGBWidth[SortRGBAxis] =
			MaxColor - NewColorSubdiv[Index].RGBMin[SortRGBAxis];

		(*NewColorMapSize)++;
    }

    return true;
}

/******************************************************************************
* Routine called by qsort to compare to entries.			      *
******************************************************************************/
int __cdecl CPngOutput::SortCmpRtn(const void * Entry1, const void * Entry2)
{
    return (* ((QuantizedColorType **) Entry1)) -> RGB[SortRGBAxis] -
	   (* ((QuantizedColorType **) Entry2)) -> RGB[SortRGBAxis];
}
