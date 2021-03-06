#include <cstdlib>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <functional>

#ifdef ENABLE_MPI
#include <mpi.h>
#endif

#include "Tools/date.h"
#include "Tools/general_utils.h"
#include "Tools/system_functions.h"
#include "Tools/Display/bash_tools.h"
#include "Tools/Exception/exception.hpp"

#include "Factory/Module/Source/Source.hpp"
#include "Factory/Module/CRC/CRC.hpp"
#include "Factory/Module/Encoder/Encoder.hpp"
#include "Factory/Module/Puncturer/Puncturer.hpp"
#include "Factory/Module/Interleaver/Interleaver.hpp"
#include "Factory/Module/Modem/Modem.hpp"
#include "Factory/Module/Channel/Channel.hpp"
#include "Factory/Module/Quantizer/Quantizer.hpp"
#include "Factory/Module/Decoder/Decoder.hpp"
#include "Factory/Module/Monitor/Monitor.hpp"
#include "Factory/Tools/Display/Terminal/Terminal.hpp"

#include "Launcher.hpp"

using namespace aff3ct;
using namespace aff3ct::launcher;

Launcher::Launcher(const int argc, const char **argv, factory::Simulation::parameters &params_common,
                   std::ostream &stream)
: simu(nullptr), ar(argc, argv), params_common(params_common), stream(stream)
{
	cmd_line += std::string(argv[0]) + std::string(" ");
	for (auto i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
			cmd_line += std::string(argv[i]);
		else
			cmd_line += std::string("\"") + std::string(argv[i]) + std::string("\"");

		cmd_line += std::string(" ");
	}
}

Launcher::~Launcher()
{
}

void Launcher::get_description_args()
{
}

void Launcher::store_args()
{
}

int Launcher::read_arguments()
{
	this->get_description_args();

	std::vector<std::string> cmd_error;

	bool miss_arg = !ar.parse_arguments(req_args, opt_args, cmd_warn);
	bool error    = !ar.check_arguments(cmd_error);

	try
	{
		this->store_args();
	}
	catch(std::exception&)
	{
		params_common.display_help = true;
	}

	if (params_common.display_help)
	{
		auto grps = factory::Factory::create_groups({&params_common});

		ar.print_usage(grps);
		error = true; // in order to exit at the end of this function
	}

	// print the errors
	for (unsigned e = 0; e < cmd_error.size(); e++)
		std::cerr << tools::format_error(cmd_error[e]) << std::endl;

	if (miss_arg)
		std::cerr << tools::format_error("At least one required argument is missing.") << std::endl;

	// print the help tags
	if ((miss_arg || error) && !params_common.display_help)
	{
		std::string message = "For more information please display the help (";
		std::vector<std::string> help_tag = {"help", "h"};
		for (unsigned i = 0; i < help_tag.size(); i++)
			message += tools::Arguments_reader::print_tag(help_tag[i]) + ((i < help_tag.size()-1)?", ":"");

		message += ").";
		std::cerr << tools::format_info(message) << std::endl;
	}

	return ((miss_arg || error) ? EXIT_FAILURE : EXIT_SUCCESS);
}

void Launcher::print_header()
{
	// display configuration and simulation parameters
	stream << "# " << tools::style("-------------------------------------------------", tools::Style::BOLD) << std::endl;
	stream << "# " << tools::style("---- A FAST FORWARD ERROR CORRECTION TOOL >> ----", tools::Style::BOLD) << std::endl;
	stream << "# " << tools::style("-------------------------------------------------", tools::Style::BOLD) << std::endl;
	stream << "# " << tools::style(style("Parameters :", tools::Style::BOLD), tools::Style::UNDERLINED) << std::endl;
	factory::Header::print_parameters({&params_common}, false, this->stream);
	this->stream << "#" << std::endl;
}

void Launcher::launch()
{
	std::srand(this->params_common.global_seed);

	// in case of the user call launch multiple times
	if (simu != nullptr)
	{
		delete simu;
		simu = nullptr;
	}

	if (this->read_arguments() == EXIT_FAILURE)
	{
		// print the warnings
#ifdef ENABLE_MPI
		if (this->params_common.mpi_rank == 0)
#endif
			for (unsigned w = 0; w < cmd_warn.size(); w++)
				std::clog << tools::format_warning(cmd_warn[w]) << std::endl;
		return;
	}

	// write the command and he curve name in the PyBER format
#ifdef ENABLE_MPI
	if (!this->params_common.pyber.empty() && this->params_common.mpi_rank == 0)
#else
	if (!this->params_common.pyber.empty())
#endif
	{
		stream << "Run command:"     << std::endl;
		stream << cmd_line           << std::endl;
		stream << "Curve name:"      << std::endl;
		stream << this->params_common.pyber << std::endl;
	}

#ifdef ENABLE_MPI
	if (this->params_common.mpi_rank == 0)
#endif
		this->print_header();

	// print the warnings
#ifdef ENABLE_MPI
	if (this->params_common.mpi_rank == 0)
#endif
		for (unsigned w = 0; w < cmd_warn.size(); w++)
			std::clog << tools::format_warning(cmd_warn[w]) << std::endl;

	try
	{
		simu = this->build_simu();
	}
	catch (std::exception const& e)
	{
		std::cerr << tools::apply_on_each_line(tools::addr2line(e.what()), &tools::format_error) << std::endl;
	}

	if (simu != nullptr)
	{
		// launch the simulation
#ifdef ENABLE_MPI
	if (this->params_common.mpi_rank == 0)
#endif
			stream << "# " << "The simulation is running..." << std::endl;

		try
		{
			simu->launch();
		}
		catch (std::exception const& e)
		{
			std::cerr << tools::apply_on_each_line(tools::addr2line(e.what()), &tools::format_error) << std::endl;
		}
	}

#ifdef ENABLE_MPI
	if (this->params_common.mpi_rank == 0)
#endif
		stream << "# End of the simulation." << std::endl;

	if (simu != nullptr)
	{
		delete simu;
		simu = nullptr;
	}
}
