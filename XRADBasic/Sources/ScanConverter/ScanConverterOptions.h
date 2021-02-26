#ifndef	XRAD__File_scan_convertor_options_h
#define	XRAD__File_scan_convertor_options_h

#include "ScanAreaGeometry.h"

XRAD_BEGIN

struct	ScanConverterOptions : public PhysicalFrameDimensions
	{
	bool	add_palette;
	bool	draw_grid;
	physical_length	grid_step;
	double	pixels_per_cm;

	void	CopyScanConverterOptions(const ScanConverterOptions &op)
		{
		PhysicalFrameDimensions::operator=(op);
		add_palette = op.add_palette;
		draw_grid = op.draw_grid;
		grid_step = op.grid_step;
		pixels_per_cm = op.pixels_per_cm;
		}
	ScanConverterOptions()
		{
		add_palette = false;
		draw_grid = false;
		grid_step = cm(2); // cm
		pixels_per_cm = 30;
		}

	ScanConverterOptions(const PhysicalFrameDimensions &op) : PhysicalFrameDimensions(op)
		{
		add_palette = false;
		draw_grid = false;
		grid_step = cm(2);
		pixels_per_cm = 30;
		}
	};




XRAD_END


#endif //XRAD__File_scan_convertor_options_h
