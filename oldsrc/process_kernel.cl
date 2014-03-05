__kernel void process(__global int levels, __global unsigned w, __global unsigned h, unsigned /*bits*/, std::vector<uint32_t> &pixels)
{
	std::vector<uint32_t> buffer(w*h);
	
	// Depending on whether levels is positive or negative,
	// we flip the order round.
	auto fwd=levels < 0 ? erode : dilate;
	auto rev=levels < 0 ? dilate : erode;
	
	for(int i=0;i<std::abs(levels);i++){
		fwd(w, h, pixels, buffer);
		std::swap(pixels, buffer);
	}
	for(int i=0;i<std::abs(levels);i++){
		rev(w,h,pixels, buffer);
		std::swap(pixels, buffer);
	}
}


#pragma OPENCL EXTENSION cl_khr_fp64: enable

__kernel void extrude_coords(__global const double * coords, __global double * res, const double layer_height){
 	
 	uint i=get_global_id(0);
	uint j=get_global_id(1);
	uint layers=get_global_size(1);

	res[3*(i*layers + j)] = coords[2*i];
    res[3*(i*layers + j) + 1] = coords[2*i + 1];
    res[3*(i*layers + j) + 2] = layer_height * j;
    
}