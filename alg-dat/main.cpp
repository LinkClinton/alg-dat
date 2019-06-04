#include "datastructure/bounding_volume_hierarchies.hpp"
#include "dependent/bound2d.hpp"

using namespace alg_dat;


int main() {
	bvh_accelerator<bound2, int> a({bound2()},{nullptr});
}