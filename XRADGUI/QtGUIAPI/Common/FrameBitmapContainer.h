#ifndef FrameBitmapContainer_h__
#define FrameBitmapContainer_h__
/********************************************************************
	created:	2016/01/27
	created:	27:1:2016   11:17
	author:		kns

	purpose:

	объект содержит windows bmp данные в виде rgb или indexed.
	это часть конструкции по отображению разнородных данных
	в ImageWindow.h, возможно не совсем удачной.
*********************************************************************/

#include <XRADBasic/Sources/Utils/BitmapContainer.h>
#include <XRADGUI/Sources/GUI/DisplaySampleType.h>

namespace XRAD_GUI
{



struct	FrameBitmapContainer
{
	private:
		BitmapContainerRGB	rgb;
		BitmapContainerIndexed	indexed;
		display_sample_type	current_sample_type;

	public:
		FrameBitmapContainer(size_t vs, size_t hs)
		{
			SetFrameSizes(vs,hs);
			current_sample_type = gray_sample_ui8;
		}
		void	SetFrameSizes(size_t vs, size_t hs)
		{
			rgb.SetSizes(vs,hs);
			indexed.SetSizes(vs,hs);
		}

		size_t	vsize() const{return rgb.vsize();}
		size_t	hsize() const{return rgb.hsize();}

		const void* GetBitmapFile() const;

		size_t	GetBitmapFileSize() const;

		template<class T, class FUNCTOR>
			void	SetFrameData(const xrad::DataArray2D<xrad::DataArray<T> >& frame, bool transpose, const FUNCTOR &f);

		template<class T, class FUNCTOR>
			void SetFrameData( const xrad::DataArray2D<xrad::DataArray<RGBColorSample<T> > >& frame, bool transpose, const FUNCTOR &f);
};



}//namespace

#include "FrameBitmapContainer.hh"

#endif // FrameBitmapContainer_h__
