#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "Filter.h"

Filter::Filter( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	//HLS_FLATTEN_ARRAY(data_buffer);
	//HLS_FLATTEN_ARRAY(box);
	//HLS_FLATTEN_ARRAY(center);
	//HLS_FLATTEN_ARRAY(result);
#endif
	SC_THREAD( do_filter );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);
        
#ifndef NATIVE_SYSTEMC
	i_rgb.clk_rst(i_clk, i_rst);
  	o_result.clk_rst(i_clk, i_rst);
#endif
}

Filter::~Filter() {}
const int mask[MASK_X][MASK_Y] = {{1, 1, 1}, {1, 2, 1}, {1, 1, 1}}; // sum = 10

void Filter::do_filter() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
#endif
		wait();
	}
	while (true) { 
		for (unsigned int i = 0; i<3; ++i) {
			result[i] = 0;
		}
		// Read data 
		for (unsigned int v = 0; v<3; ++v) {
			for (unsigned int u = 0; u<3; ++u) {
				sc_dt::sc_uint<25> rgb;
#ifndef NATIVE_SYSTEMC
				{
					HLS_DEFINE_PROTOCOL("input");
					rgb = i_rgb.get();
					wait();
				}
#else
				rgb = i_rgb.read();
#endif
				unsigned char color[3];
				color[0] = rgb.range(7,0);
				color[1] = rgb.range(15,8);
				color[2] = rgb.range(23, 16);
				if (rgb[24] == 1){

						//HLS_CONSTRAIN_LATENCY(0, 1, "lat01");
					pixel_r[u][v] = color[0];   
					pixel_g[u][v] = color[1]; 
					pixel_b[u][v] = color[2];           
					
				} 
				else{
					for (unsigned int v = 0; v < MASK_Y; ++v){
						for (unsigned int u = 0; u < MASK_X; ++u){
						if(u!=2){
						pixel_r[u][v] = r_buffer[u][v];
						pixel_g[u][v] = g_buffer[u][v];
						pixel_b[u][v] = b_buffer[u][v];
						}
						else{
						pixel_r[u][v] = color[0];
						pixel_g[u][v] = color[1];
						pixel_b[u][v] = color[2];
						
						}
						}
					}
					break;
				}
				
			}
		}

		// Store pixels in data_buffer
		for (unsigned int v = 0; v < MASK_Y; ++v){
			for (unsigned int u = 0; u < MASK_X-1; ++u){ 
			r_buffer[u][v] = pixel_r[u+1][v];
			g_buffer[u][v] = pixel_g[u+1][v]; 
			b_buffer[u][v] = pixel_b[u+1][v];
			}
		}


		// 1. Applying "median filter" to each color channel
		// Flatten the 2D array into a 1D array
		int flattened_r[9],flattened_g[9],flattened_b[9];
		int k = 0;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				flattened_r[k] = pixel_r[i][j];
				flattened_g[k] = pixel_g[i][j];
				flattened_b[k] = pixel_b[i][j];
				k=k+1;
			}
		}
		std::sort(flattened_r, flattened_r + 9);
		std::sort(flattened_g, flattened_g + 9);
		std::sort(flattened_b, flattened_b + 9);

		unsigned char filtered_r = flattened_r[4];
		unsigned char filtered_g = flattened_g[4];
		unsigned char filtered_b = flattened_b[4];


		// 2. Applying "mean filter" to each color channel
		int sum_r = 0, sum_g = 0, sum_b = 0;

		for (unsigned int v = 0; v < MASK_Y; ++v) {
			for (unsigned int u = 0; u < MASK_X; ++u) {
				sum_r += filtered_r * mask[u][v];
				sum_g += filtered_g * mask[u][v];
				sum_b += filtered_b * mask[u][v];
			}
		}

		filtered_r = sum_r / 10;
		filtered_g = sum_g / 10;
		filtered_b = sum_b / 10;

		sc_dt::sc_uint<24> total;
		total.range(7, 0) = filtered_r ;
		total.range(15, 8) = filtered_g ;
		total.range(23, 16) = filtered_b ;

#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_result.put(total);
			wait();
		}
#else
		o_result.write(total);
#endif
	}
}
