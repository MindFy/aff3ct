#ifndef FACTORY_TERMINAL_HPP_
#define FACTORY_TERMINAL_HPP_

#include <string>
#include <chrono>

#include "../../../Factory.hpp"

namespace aff3ct
{
namespace factory
{
extern const std::string Terminal_name;
extern const std::string Terminal_prefix;
struct Terminal : Factory
{
	class parameters : public Factory::parameters
	{
	public:
		// ------------------------------------------------------------------------------------------------- PARAMETERS
		// optional parameters
		std::chrono::milliseconds frequency = std::chrono::milliseconds(500);
		bool                      disabled  = false;

		// ---------------------------------------------------------------------------------------------------- METHODS
		virtual ~parameters();
		virtual Terminal::parameters* clone() const;

		// parameters construction
		virtual void get_description(arg_map &req_args, arg_map &opt_args                              ) const;
		virtual void store          (const arg_val_map &vals                                           );
		virtual void get_headers    (std::map<std::string,header_list>& headers, const bool full = true) const;

	protected:
		parameters(const std::string &n = Terminal_name, const std::string &p = Terminal_prefix);
	};
};
}
}

#endif /* FACTORY_TERMINAL_HPP_ */
