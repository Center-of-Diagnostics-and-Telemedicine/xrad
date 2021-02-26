#ifndef	XRAD__File_graph_scale_h
#define	XRAD__File_graph_scale_h

#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

XRAD_BEGIN

#if 1
typedef range2_F64 GraphScale;
#else

class	GraphScale
	{
	public:
		double	x1;
		double	x2;
		double	y1;
		double	y2;

		GraphScale()
			{
			x1 = 0;
			x2 = 1;
			y1= 0;
			y2= 1;
			};
		GraphScale(double i_xmin, double i_xmax, double i_ymin, double i_ymax)
			{
			x1 = i_xmin;
			x2 = i_xmax;
			y1 = i_ymin;
			y2 = i_ymax;
			};

	};
#endif

enum	GridScheme
	{
	grid_auto,
	grid_auto_rough,
	grid_auto_fine,
	grid_4,
	grid_8,
	grid_10,
	grid_16
	};



double	guess_auto_grid(double range, GridScheme gs);


XRAD_END


#endif	//XRAD__File_graph_scale_h