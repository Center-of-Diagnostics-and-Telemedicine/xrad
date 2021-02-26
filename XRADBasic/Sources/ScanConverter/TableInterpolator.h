#ifndef	XRAD__File_table_interpolator_h
#define	XRAD__File_table_interpolator_h


#include <XRADBasic/Sources/SampleTypes/ColorSample.h>
#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>

XRAD_BEGIN

// версия с bool
template<class image_t, class sample_t, class factor_t>
class	TableInterpolator
	{
	private:
		bool	use_default_sample;
		sample_t	default_sample;

		factor_t	f00, f01, f10, f11;	// множители
		typename image_t::value_type	*s00, *s01, *s10, *s11;	// отсчеты, отвечающие за интерполяцию в конкретной точке

	private:
		// слегка оптимизированные алгоритмы умножения для разных исходных данных
		enum
			{
			one_shift=7
			};

		double	multiply(double ij, double i_j, double ij_, double i_j_) const
			{
			return (ij*f00 + i_j*f01 + ij_*f10 + i_j_*f11);
			}

		float	multiply(float ij, float i_j, float ij_, float i_j_) const
			{
			return (ij*f00 + i_j*f01 + ij_*f10 + i_j_*f11);
			}


		int	multiply(unsigned char ij, unsigned char i_j, unsigned char ij_, unsigned char i_j_) const
			{
			return	(ij*f00 + i_j*f01 + ij_*f10 + i_j_*f11)>>one_shift;
			}

		int	multiply(int ij, int i_j, int ij_, int i_j_) const
			{
			return	(ij*f00 + i_j*f01 + ij_*f10 + i_j_*f11)>>one_shift;
			// если здесь возникает ошибка, нужно доопределить тип
			// struct	FactorTypeSelector<тип_отсчета_изображения>{typedef int factor_type;}; в объявлении класса ScanConverter
			// также доопределить DisplaySampleType<>
			}

		template<class T>
		RGBColorSample<T> multiply(const RGBColorSample<T> &ij, const RGBColorSample<T> &i_j, const RGBColorSample<T> &ij_, const RGBColorSample<T> &i_j_) const
			{
			return RGBColorSample<T>(
				multiply(ij.red(), i_j.red(), ij_.red(), i_j_.red()),
				multiply(ij.green(), i_j.green(), ij_.green(), i_j_.green()),
				multiply(ij.blue(), i_j.blue(), ij_.blue(), i_j_.blue()));
			}

		template<class T, class ST>
		ComplexSample<T,ST> multiply(const ComplexSample<T,ST> &ij, const ComplexSample<T,ST> &i_j, const ComplexSample<T,ST> &ij_, const ComplexSample<T,ST> &i_j_) const
			{
			return ComplexSample<T,ST>(
				multiply(ij.re, i_j.re, ij_.re, i_j_.re),
				multiply(ij.im, i_j.im, ij_.im, i_j_.im));
			}

		factor_t one(double){return 1;}
		factor_t one(float){return 1;}
		factor_t one(int){return (1<<one_shift);}


	public:

		TableInterpolator()
			{
			use_default_sample = true;
			default_sample = 0;
			s00 = s01 = s10 = s11 = NULL;
			}

		TableInterpolator(const sample_t &p)
			{
			use_default_sample = true;
			default_sample = p;
			s00 = s01 = s10 = s11 = NULL;
			}

		TableInterpolator(double v, double h, image_t &im)
			{
			use_default_sample = false;

			int	vi = int(v);
			int	hi = int(h);
			double	a = v-vi;
			double	b = h-hi;

			vi = range(vi, 0, int(im.vsize()-2));
			hi = range(hi, 0, int(im.hsize()-2));

			f00 = factor_t((1.-a)*(1-b)*one((factor_t)0));
			f01 = factor_t(a*(1.-b)*one((factor_t)0));
			f10 = factor_t((1.-a)*b*one((factor_t)0));
			f11 = factor_t(a*b*one((factor_t)0));

			s00 = &im.at(vi,hi);
			s01 = &im.at(vi+1,hi);
			s10 = &im.at(vi,hi+1);
			s11 = &im.at(vi+1,hi+1);
			}

		sample_t interpolate() const
			{
			if(use_default_sample) return default_sample;
			return sample_t(multiply(*s00, *s01, *s10, *s11));
			}

	};


XRAD_END

#endif //XRAD__File_table_interpolator_h
