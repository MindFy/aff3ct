#ifndef ENCODER_LDPC_DVBS2_HPP_
#define ENCODER_LDPC_DVBS2_HPP_

#include <vector>

#include "../Encoder_LDPC.hpp"

#include "Encoder_LDPC_DVBS2_constants.hpp"

namespace aff3ct
{
namespace module
{

template <typename B = int>
class Encoder_LDPC_DVBS2 : public Encoder_LDPC<B>
{
	const dvbs2_values* dvbs2 = nullptr;

public:
	Encoder_LDPC_DVBS2(const int K, const int N, const int n_frames = 1);
	virtual ~Encoder_LDPC_DVBS2();

	const std::vector<uint32_t>& get_info_bits_pos();

	bool is_sys() const;

protected:
	void _encode(const B *U_K, B *X_N, const int frame_id);

private:
	void build_dvbs2();
};

}
}

#endif /* ENCODER_LDPC_DVBS2_HPP_ */
