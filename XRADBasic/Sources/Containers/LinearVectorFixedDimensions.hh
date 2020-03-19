XRAD_BEGIN

//	задаются свойства, предусмотренные в number_traits.h.
//	делаются необходимые проверки

#define XRAD__linear_vector_fd LinearVectorFixedDimensions<XRAD__linear_vector_template_args>

template<XRAD__linear_vector_template>
CHILD zero_value(const XRAD__linear_vector_fd&){return CHILD(0);}

template<XRAD__linear_vector_template>
void make_zero(XRAD__linear_vector_fd &datum){datum = XRAD__linear_vector_fd(0);}

template<XRAD__linear_vector_template>
number_complexity_e complexity_e(const XRAD__linear_vector_fd &){return number_complexity_e::array;}

template<XRAD__linear_vector_template>
number_complexity::array *complexity_t(const XRAD__linear_vector_fd &){return nullptr;}

template<XRAD__linear_vector_template>
size_t n_components(const XRAD__linear_vector_fd &){return N;}

template<XRAD__linear_vector_template>
const T& component(const XRAD__linear_vector_fd &x, size_t n){return x[n];}

template<XRAD__linear_vector_template>
T& component(XRAD__linear_vector_fd &x, size_t n){return x[n];}

template<XRAD__linear_vector_template>
double quadratic_norma(const XRAD__linear_vector_fd &x)
{
	double	result(0);
	typename XRAD__linear_vector_fd::const_iterator it = x.cbegin(), ie = x.cend();
	for(; it<ie; ++it) result += quadratic_norma(*it);
	return result;
}

template<XRAD__linear_vector_template>
double norma(const XRAD__linear_vector_fd &x){return sqrt(quadratic_norma(x));}

template<XRAD__linear_vector_template>
double fast_norma(const XRAD__linear_vector_fd &x)
{
	double	result(0);
	typename XRAD__linear_vector_fd::const_iterator it = x.cbegin(), ie = x.cend();
	for(; it<ie; ++it) result += fast_norma(*it);
	return result;
}

#undef XRAD__linear_vector_fd

namespace internal_LinearVectorFDDummy
{
struct	dummy_lvf: public LinearVectorFixedDimensions<dummy_lvf, double, double, AlgebraicStructures::FieldTagScalar, 3>
{
	dummy_lvf(double x): LinearVectorFixedDimensions(x){}
	dummy_lvf(){}
};

check_if_number_traits_defined(dummy_lvf);
}

XRAD_END
