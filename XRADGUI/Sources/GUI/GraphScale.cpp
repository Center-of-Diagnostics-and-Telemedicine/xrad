#include "pre_GUI.h"
#include <GraphScale.h>

XRAD_BEGIN


double	guess_auto_grid(double range, GridScheme gs)
	{
	double	grid_step;
	int	i;
	switch(gs)
		{
		//if( range<=1e-5 || GridScheme==0)
		case	grid_8:
			grid_step = range/8;
			break;
		case	grid_10:
			grid_step = range/10;
			break;
		case	grid_16:
			grid_step = range/16;
			break;
		case	grid_4:
			grid_step = range/4;
			break;

		default:
		case grid_auto:
		case grid_auto_rough:
		case grid_auto_fine:
			for (i = 0; range < 1. && i < 20; range*=10., ++i){}
			for(;range>10. && i > -20; range/=10., --i){}
			// range in [1,10]
			if( range<=2)
				grid_step = .1;
			else if( range<=4.)
				grid_step = .25;
			else if( range<=7.)
				grid_step = .5;
			else
				grid_step = 1.;
			for(; i<0; grid_step*=10, ++i){}
			for(; i>0; grid_step/=10, --i){}
			if(gs == grid_auto_rough) grid_step *= 2;
			if(gs == grid_auto_fine) grid_step /= 2;
			break;
		};

	return grid_step;
	}


XRAD_END
