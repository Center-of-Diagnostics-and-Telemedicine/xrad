#ifndef	XRAD__File_scan_area_geometry_h
#define	XRAD__File_scan_area_geometry_h

#include <XRADBasic/Sources/Utils/PhysicalUnits.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

XRAD_BEGIN

//--------------------------------------------------------------
//
//	минимальный набор величин, определяющих геометрические
//	параметры области сканирования при линейном и секторном обзоре
//

// rename to PhysicalFrameOptions
struct	PhysicalFrameDimensions
	{
	private:
		range_1<physical_angle, double> m_angle_range;
//		physical_angle	_start_angle, _end_angle;
			// для линейного сканирования оба параметра равны между собой,
			// обычно (но необязательно) они нулевые
	//public:
		physical_angle	m_steering_angle;
			// угол стиринга, пока в формулах не учитывается!
		physical_length	m_scanning_trajectory_length;
			// длина траектории сканирования на минимальной имеющейся глубине.
			// для линейного датчика совпадает с длиной линейки (или активной ее части);
			// для конвекса равен (R_c+R_min)*(end_angle-start_angle),
			// где Rc -- радиус кривизны датчика, R_min -- глубина от поверхности
			// датчика до первого принятого отсчета;
			// для секторного фазированного датчика равен R_min*(end_angle-start_angle),

		physical_length	m_depth_range;
			// диапазон глубин

		//	construction

	public:

		PhysicalFrameDimensions() :
				m_angle_range(radians(0), radians(0)),
				m_steering_angle(radians(0)),
				m_depth_range(cm(0)),
				m_scanning_trajectory_length(cm(0))
			{
			}

		PhysicalFrameDimensions &operator=(const PhysicalFrameDimensions &original)
			{
			m_angle_range = original.m_angle_range;
			m_steering_angle = original.m_steering_angle;
			m_depth_range = original.m_depth_range;
			m_scanning_trajectory_length = original.m_scanning_trajectory_length;
			return *this;
			}

		void	SetFrameRectangle(const physical_length &width, const physical_length &heigth)
			{
			m_depth_range = heigth;
			m_scanning_trajectory_length = width;
			m_angle_range.x1() = m_angle_range.x2() = m_steering_angle = radians(0);
			}

		void	SetFrameSector(const physical_length &in_scanning_trajectory_length, const physical_length &in_depth_range,
							const physical_angle &in_start_angle, const physical_angle &in_end_angle)
			{
			m_depth_range = in_depth_range;
			m_scanning_trajectory_length = in_scanning_trajectory_length;
			m_angle_range.x1() = in_start_angle;
			m_angle_range.x2() = in_end_angle;
			m_steering_angle = radians(0);
			}

		//	access

		physical_length	r_min() const
			{
			if(angle_range().radians()) return m_scanning_trajectory_length/angle_range().radians();
			else return cm(0);
			}

		physical_length	r_max() const{return r_min() + m_depth_range;}
		physical_length	depth_range() const {return m_depth_range;}
		physical_length	scanning_trajectory_length() const {return m_scanning_trajectory_length;}

		physical_angle	start_angle() const {return m_angle_range.x1();}
		physical_angle	end_angle() const {return m_angle_range.x2();}

		physical_angle angle_range() const {return m_angle_range.delta();}
	};


inline PhysicalFrameDimensions	ScanFrameRectangle(const physical_length &width, const physical_length &heigth)
	{
	PhysicalFrameDimensions	result;
	result.SetFrameRectangle(width, heigth);
//	result.depth_range = heigth;
//	result.scanning_trajectory_length = width;
	return result;
	}

inline PhysicalFrameDimensions	ScanFrameSector(const physical_length &scanning_trajectory_length, const physical_length &depth_range,
					const physical_angle &start_angle, const physical_angle &end_angle)
	{
	PhysicalFrameDimensions	result;
	result.SetFrameSector(scanning_trajectory_length, depth_range, start_angle, end_angle);
	/*
	result.depth_range = depth_range;
	result.scanning_trajectory_length = scanning_trajectory_length;
	result.start_angle = start_angle;
	result.end_angle = end_angle;
	*/
	return result;
	}



//--------------------------------------------------------------



struct	SampledFrameROI
	{
	size_t n_samples_skipped; // число отсчетов, пропущенных системой до начала записи
	size_t n_samples_to_skip_at_start; // число отсчетов, после начала записи, которые следует игнорировать
	size_t n_samples_to_skip_at_end; // число отсчетов перед концом записи, которые следует игнорировать

	physical_frequency carrier_offset;

	SampledFrameROI() : n_samples_skipped(0), n_samples_to_skip_at_start(0), n_samples_to_skip_at_end(0), carrier_offset(MHz(0)){}
	};


struct	SampledFrameDimensions
	{
	size_t n_rays;
	size_t n_samples; // число полезных отсчетов в одной строке

//	physical_frequency carrier_frequency; // не здесь этому место. bandwidth еще добавить
	physical_frequency sample_rate;

	SampledFrameDimensions() : n_rays(0), n_samples(0), sample_rate(MHz(0)){}
	};



XRAD_END

#endif //XRAD__File_scan_area_geometry_h
