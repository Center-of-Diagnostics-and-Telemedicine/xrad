#ifndef TomogramAcquisition_h__
#define TomogramAcquisition_h__
/*!
	\file
	\date 2016-09-29 11:43
	\author kns
*/
//--------------------------------------------------------------

#include "ProcessAcquisition.h"
#include <XRADBasic/LinearVectorTypes.h>
#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADBasic/Sources/Containers/VectorFunction.h>
#include <set>

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	Тип и его наследники реализованы только для содержания данных, с которыми в данный момент идёт работа. Этот тип не умеет (и не должен) работать с файлами (забирать оттуда данные или отдавать их туда). Все эти работы происходят через типы Generic.
	В себе эти типы содержат изображения и все остальные нужные для обработки данные в виде массивов, а также набор общих данных, необходимых для успешного формирования dicom-файлов.
*/
class TomogramAcquisition : public ProcessAcquisition
{
public:
	typedef	TomogramAcquisition self;

	//constructors
	TomogramAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_loader_p);

	//operators
	virtual TomogramAcquisition &operator=(const TomogramAcquisition &original);

	virtual std::string classname() const override { return "TomogramAcquisition"; }
	//gets
	virtual size_t n_elements() const override { return m_acquisition_loader->size(); }

	//methods
	virtual void put_elements_to_instance(Dicom::instance &instance, size_t num_frame) const;
	point3_F64	scales() const;

	RealFunctionF32 prepare_RealFunctionF32(Dicom::tag_e elem_tag) const;
	RealFunctionF64 prepare_RealFunctionF64(Dicom::tag_e elem_tag) const;

	RealFunctionF64	thickness() const;
	VectorFunction2_F64	scales_xy() const;
	VectorFunction3_F64	image_positions_patient() const;
	index_vector	sizes() const;
	size_t	sizes(size_t dim) const;
	RealFunctionMD_F32	slices() const;
	//! \brief Загрузить данные, упорядоченные в соответствии с determine_slice_order()
	RealFunctionMD_F32	load_ordered_slices() const;
	RealFunctionMD_F32	load_ordered_slices(const vector<size_t> &slice_order) const;
	vector<size_t> determine_slice_order() const;

	//! \brief Определяет ось, по которой следует производить сортировку срезов томограммы.
	//! Выбирает ту ось, по которой происходит самая большая разница
	bool sort_axis(size_t &sort_axis_p) const;
};

//--------------------------------------------------------------

XRAD_END

#endif // TomogramAcquisition_h__
