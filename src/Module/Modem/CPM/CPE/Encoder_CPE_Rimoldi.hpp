#ifndef ENCODER_CPE_RIMOLDI_HPP_
#define ENCODER_CPE_RIMOLDI_HPP_

#include <mipp.h>

#include "Encoder_CPE.hpp"

namespace aff3ct
{
namespace module
{
template <typename SIN = int, typename SOUT = int>
class Encoder_CPE_Rimoldi : public Encoder_CPE<SIN, SOUT>
{
public:
	Encoder_CPE_Rimoldi(const int N, const CPM_parameters<SIN,SOUT>& cpm);
	virtual ~Encoder_CPE_Rimoldi() {}

	SOUT inner_encode(const SIN new_symbol, int &state);
	SIN  tail_symb   (const int &state                );

	void generate_mapper(mipp::vector<SIN>& transition_to_binary,
	                     mipp::vector<SIN>& binary_to_transition,
	                     const std::string& mapping);

	void generate_allowed_states    (mipp::vector<int>& allowed_states     );
	void generate_allowed_wave_forms(mipp::vector<SOUT>& allowed_wave_forms);
};
}
}

#endif /* ENCODER_CPE_RIMOLDI_HPP_ */
