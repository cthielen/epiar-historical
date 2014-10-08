#include "includes.h"
#include "system/math.h"

#define ERROR 3

float get_distance_sqrd(float x1, float y1, float x2, float y2) {
	return (((x2-x1)*(x2-x1)) + ((y2 - y1)*(y2 - y1)));
}

float get_distance(float x1, float y1, float x2, float y2) {
	return (sqrt(((x2-x1)*(x2-x1)) + ((y2 - y1)*(y2 - y1))));
}

/* twisted version of ai_turn_towards() in ai.c */
/* perspective is from the first set of coords */
float get_angle_to(float x1, float y1, float x2, float y2) {
	float angle;

	/* invert the y's because we're mathematically incorrect (the in-game coords are relfected over the x axis) */
	angle = (float)atan2((float)((-1 * y2) - (-1 * y1)), (float)(x2 - x1));
	angle = (angle * 180.0) / (3.14);

	if (angle < 0.0) angle += 360.0;

	return(angle);
}

/* simple line intersection - written by chris thielen (chris@luethy.net) jan. 19, 2003 */
int line_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float *x_inter, float *y_inter) 
{
   float m1, m2, b1, b2;

   /* resolve slopes */
	if ((x2 - x1) == 0)
   		m1 = (y2 - y1) / 0.1f;
	else
		m1 = (y2 - y1) / (x2 - x1);
	if ((x4 - x3) == 0)
		m2 = (y4 - y3) / 0.1f;
	else
		m2 = (y4 - y3) / (x4 - x3);

	if (m1 == m2) {
		return (-1); /* no intersection */
	}

	/* resolve y intercepts */
	b1 = y1 - (m1*x1);
	b2 = y3 - (m2*x3);

	/* if m1 is a vertical line and m2 isnt */
	if (((x2 - x1) == 0) && ((x4 - x3) != 0)) {
		// does m1's x coord fit on m2 within the range?
		float new_y = (m2 * x1) + b2;
		if (((new_y > y3) && (new_y < y4)) ||  ((new_y > y4) && (new_y < y3)))
			return (0);
		else
			return (-1);
	}
	/* if m2 is a vertical line and m1 isnt */
	if (((x4 - x3) == 0) && ((x2 - x1) != 0)) {
		// does m2's x coord fit on m1 within the range?
		float new_y = (m1 * x3) + b1;
		if (((new_y > y1) && (new_y < y2)) ||  ((new_y > y2) && (new_y < y1)))
			return (0);
		else
			return (-1);
	}

   /* figure out intercept */
   *x_inter = (b2 - b1) / (m1 - m2);
   *y_inter = (m1 * *x_inter) + b1;

	if ((((*x_inter + ERROR) >= x1) && ((*x_inter - ERROR) <= x2)) || (((*x_inter - ERROR) <= x1) && ((*x_inter + ERROR) >= x2))) 
	{

		if ((((*x_inter + ERROR) >= x3) && ((*x_inter - ERROR) <= x4)) || (((*x_inter - ERROR) <= x3) && ((*x_inter + ERROR) >= x4))) 
		{

	     if ((((*y_inter + ERROR) >= y1) && ((*y_inter - ERROR) <= y2)) || (((*y_inter - ERROR) <= y1) && ((*y_inter + ERROR) >= y2))) 
	       {
	
		  if ((((*y_inter + ERROR) >= y3) && ((*y_inter - ERROR) <= y4)) || (((*y_inter - ERROR) <= y3) && ((*y_inter + ERROR) >= y4))) 
		    {
		       return (0);
		    }
	       }
	  }
     }

   return (-1);
   
}
